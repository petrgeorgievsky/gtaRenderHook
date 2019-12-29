#include "rw_rh_pipeline.h"
#include "rw_api_injectors.h"
#include <Engine/Common/types/index_buffer_info.h>
#include <Engine/Common/types/vertex_buffer_info.h>
#include <Engine/D3D11Impl/Buffers/D3D11IndexBuffer.h>
#include <Engine/D3D11Impl/Buffers/D3D11VertexBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <Engine/IRenderer.h>
#include <algorithm>
#include <common_headers.h>
namespace rh::rw::engine {

struct rwResEnty_ : RwResEntry
{
    rh::engine::IPrimitiveBatch *modelInstance;
    uint32_t padd;
};

struct RxTriangle
{
    RxVertexIndex a, b, c;
};

static bool SortTriangles( const RxTriangle &pA, const RxTriangle &pB ) noexcept
{
    uint32_t sortedIndexA[3] = {pA.a, pA.b, pA.c};
    uint32_t sortedIndexB[3] = {pB.a, pB.b, pB.c};

    if ( sortedIndexA[0] > sortedIndexA[1] )
        std::swap( sortedIndexA[0], sortedIndexA[1] );

    if ( sortedIndexA[1] > sortedIndexA[2] ) {
        std::swap( sortedIndexA[1], sortedIndexA[2] );

        if ( sortedIndexA[0] > sortedIndexA[1] ) {
            std::swap( sortedIndexA[0], sortedIndexA[1] );
        }
    }

    if ( sortedIndexB[0] > sortedIndexB[1] ) {
        std::swap( sortedIndexB[0], sortedIndexB[1] );
    }

    if ( sortedIndexB[1] > sortedIndexB[2] ) {
        std::swap( sortedIndexB[1], sortedIndexB[2] );

        if ( sortedIndexB[0] > sortedIndexB[1] ) {
            std::swap( sortedIndexB[0], sortedIndexB[1] );
        }
    }

    if ( sortedIndexA[0] == sortedIndexB[0] ) {
        if ( sortedIndexA[1] == sortedIndexB[1] ) {
            return ( sortedIndexA[2] < sortedIndexB[2] );
        }

        return ( sortedIndexA[1] < sortedIndexB[1] );
    }

    return ( sortedIndexA[0] < sortedIndexB[0] );
}

void GenerateNormals( VertexDescPosColorUVNormals *verticles,
                      uint32_t vertexCount,
                      RpTriangle *triangles,
                      unsigned int triangleCount,
                      bool /*isTriStrip*/ )
{
    // generate normal for each triangle and vertex in mesh
    for ( RwUInt32 i = 0; i < triangleCount; i++ ) {
        auto triangle = triangles[i];
        auto iA = triangle.vertIndex[2], iB = triangle.vertIndex[1], iC = triangle.vertIndex[0];

        const auto vA = verticles[iA], vB = verticles[iB], vC = verticles[iC];
        // tangent vector
        RwV3d tangent = {vB.x - vA.x, vB.y - vA.y, vB.z - vA.z};
        // bitangent vector
        RwV3d bitangent = {vA.x - vC.x, vA.y - vC.y, vA.z - vC.z};
        // fix for triangle strips

        // float normalDirection = isTriStrip?(i % 2 == 0 ? 1.0f : -1.0f):1.0f;
        // normal vector as cross product of (tangent X bitangent)
        RwV3d normal = {( tangent.y * bitangent.z - tangent.z * bitangent.y ),
                        ( tangent.z * bitangent.x - tangent.x * bitangent.z ),
                        ( tangent.x * bitangent.y - tangent.y * bitangent.x )};
        // increase normals of each vertex in triangle

        verticles[iA].nx = verticles[iA].nx + normal.x;
        verticles[iA].ny = verticles[iA].ny + normal.y;
        verticles[iA].nz = verticles[iA].nz + normal.z;

        verticles[iB].nx = verticles[iB].nx + normal.x;
        verticles[iB].ny = verticles[iB].ny + normal.y;
        verticles[iB].nz = verticles[iB].nz + normal.z;

        verticles[iC].nx = verticles[iC].nx + normal.x;
        verticles[iC].ny = verticles[iC].ny + normal.y;
        verticles[iC].nz = verticles[iC].nz + normal.z;
    }
    // normalize normals
    for ( RwUInt32 i = 0; i < vertexCount; i++ ) {
        RwReal length = sqrt( verticles[i].nx * verticles[i].nx + verticles[i].ny * verticles[i].ny
                              + verticles[i].nz * verticles[i].nz );
        if ( length > 0.000001f ) {
            verticles[i].nx = verticles[i].nx / length;
            verticles[i].ny = verticles[i].ny / length;
            verticles[i].nz = verticles[i].nz / length;
        }
    }
}

RwResEntry *RHInstanceAtomicGeometry( RpGeometryInterface *geom_io,
                                      void *owner,
                                      RwResEntry *&resEntryPointer,
                                      const RpMeshHeader *meshHeader )
{
    rwResEnty_ *resEntry;

    resEntry = reinterpret_cast<rwResEnty_ *>( g_pGlobal_API.fpResourcesAllocateResEntry(
        owner, &resEntryPointer, sizeof( rwResEnty_ ) - sizeof( RwResEntry ), [
        ]( RwResEntry * resEntry ) noexcept {
            rwResEnty_ *entry = reinterpret_cast<rwResEnty_ *>( resEntry );
            if ( entry != nullptr )
                delete entry->modelInstance;
        } ) );
    resEntryPointer = resEntry;
    if ( resEntry == nullptr )
        return nullptr;

    rh::engine::IGPUResource *d3dIndexBuffer = nullptr;
    rh::engine::IGPUResource *d3dVertexBuffer = nullptr;

    rh::engine::PrimitiveType primType = rh::engine::PrimitiveType::TriangleStrip;
    if ( meshHeader->flags & rpMESHHEADERTRISTRIP )
        primType = rh::engine::PrimitiveType::TriangleStrip;
    else if ( ( meshHeader->flags & rpMESHHEADERPRIMMASK ) == 0 )
        primType = rh::engine::PrimitiveType::TriangleList;

    const RpMesh *mesh_start = reinterpret_cast<const RpMesh *>( meshHeader + 1 );
    RxVertexIndex *indexBuffer = new RxVertexIndex[meshHeader->totalIndicesInMesh * 3];
    uint32_t startIndex = 0;
    uint32_t indexCount = 0;

    std::vector<rh::engine::MeshSplitData> splitData;
    splitData.reserve( meshHeader->numMeshes );
    // Index data
    for ( const RpMesh *mesh = mesh_start; mesh != mesh_start + meshHeader->numMeshes; mesh++ ) {
        indexCount = mesh->numIndices;

        rh::engine::MeshSplitData meshData{};
        meshData.startIndex = startIndex;
        meshData.numIndex = indexCount;
        meshData.material = mesh->material;
        uint32_t minVert;

        MeshGetNumVerticesMinIndex( mesh->indices, indexCount, meshData.numVertices, minVert );

        for ( size_t i = startIndex; i < startIndex + indexCount; i++ )
            indexBuffer[i] = mesh->indices[i - startIndex];

        if ( primType == rh::engine::PrimitiveType::TriangleList ) {
            std::sort( reinterpret_cast<RxTriangle *>( indexBuffer + startIndex ),
                       reinterpret_cast<RxTriangle *>( indexBuffer + startIndex + indexCount ),
                       SortTriangles );
        }
        startIndex += indexCount;

        splitData.push_back( meshData );
    }
    {
        D3D11_SUBRESOURCE_DATA subresData{};
        subresData.pSysMem = indexBuffer;

        rh::engine::IndexBufferInfo bufferInfo;
        bufferInfo.indexCount = meshHeader->totalIndicesInMesh;
        bufferInfo.isDynamic = false;
        bufferInfo.initialData = &subresData;

        rh::engine::g_pRHRenderer->GetGPUAllocator()->AllocateIndexBuffer( bufferInfo,
                                                                           d3dIndexBuffer );
    }
    delete[] indexBuffer;

    // Vertex data
    VertexDescPosColorUVNormals *vertexData
        = new VertexDescPosColorUVNormals[static_cast<size_t>( geom_io->GetVertexCount() )];

    auto morph_target = geom_io->GetMorphTarget( 0 );
    RwV3d *vertexPos = morph_target->verts;
    RwTexCoords *vertexUV = geom_io->GetTexCoordSetPtr( 0 );
    RwRGBA *vertexColorPtr = geom_io->GetVertexColorPtr();
    RwV3d *normalsPtr = morph_target->normals;

    uint32_t v_id = 0;
    for ( ; vertexPos != morph_target->verts + geom_io->GetVertexCount(); vertexPos++ ) {
        VertexDescPosColorUVNormals desc{};
        desc.x = vertexPos->x;
        desc.y = vertexPos->y;
        desc.z = vertexPos->z;
        desc.w = 1.0f;
        if ( normalsPtr ) {
            desc.nx = normalsPtr->x;
            desc.ny = normalsPtr->y;
            desc.nz = normalsPtr->z;
            normalsPtr++;
        } else {
            desc.nx = desc.ny = desc.nz = 0;
        }
        if ( vertexColorPtr ) {
            desc.color[0] = vertexColorPtr->red;
            desc.color[1] = vertexColorPtr->green;
            desc.color[2] = vertexColorPtr->blue;
            desc.color[3] = vertexColorPtr->alpha;
            vertexColorPtr++;
        } else {
            desc.color[0] = 255;
            desc.color[1] = 255;
            desc.color[2] = 255;
            desc.color[3] = 255;
        }
        if ( vertexUV ) {
            desc.u = vertexUV->u;
            desc.v = vertexUV->v;
            vertexUV++;
        } else {
            desc.u = 0;
            desc.v = 0;
        }
        vertexData[v_id] = desc;
        v_id++;
    }

    GenerateNormals( vertexData,
                     static_cast<uint32_t>( geom_io->GetVertexCount() ),
                     geom_io->GetTrianglePtr(),
                     static_cast<uint32_t>( geom_io->GetTriangleCount() ),
                     primType == rh::engine::PrimitiveType::TriangleStrip );

    {
        D3D11_SUBRESOURCE_DATA vertSubresData{};
        vertSubresData.pSysMem = vertexData;

        rh::engine::VertexBufferInfo vertexBufferInfo;
        vertexBufferInfo.vertexCount = static_cast<uint32_t>( geom_io->GetVertexCount() );
        vertexBufferInfo.vertexSize = sizeof( VertexDescPosColorUVNormals );
        vertexBufferInfo.isDynamic = false;
        vertexBufferInfo.initialData = &vertSubresData;
        rh::engine::g_pRHRenderer->GetGPUAllocator()->AllocateVertexBuffer( vertexBufferInfo,
                                                                            d3dVertexBuffer );
    }

    delete[] vertexData;

    resEntry->modelInstance = new rh::engine::D3D11PrimitiveBatch( meshHeader->serialNum,
                                                                   d3dIndexBuffer,
                                                                   d3dVertexBuffer,
                                                                   primType,
                                                                   splitData );

    return reinterpret_cast<RwResEntry *>( resEntry );
}

RenderStatus RwRHInstanceAtomic( RpAtomic *atomic, RpGeometryInterface *geom_io )
{
    geom_io->Init( atomic->geometry );

    // Early return if geometry has no vertices
    if ( geom_io->GetVertexCount() <= 0 )
        return RenderStatus::NotInstanced;

    RpMeshHeader *meshHeader = geom_io->GetMeshHeader();

    if ( meshHeader == nullptr )
        return RenderStatus::Failure;

    if ( meshHeader->numMeshes <= 0 )
        return RenderStatus::NotInstanced;

    const RwUInt32 geomFlags = geom_io->GetFlags();

    RwResEntry *resEntry = nullptr;

    if ( !( rpGEOMETRYNATIVE & geomFlags ) ) {
        /* If the geometry has more than one morph target the resEntry in the
     * atomic is used else the resEntry in the geometry */
        if ( geom_io->GetMorphTargetCount() != 1 ) {
            resEntry = atomic->repEntry;
        } else {
            resEntry = static_cast<RwResEntry *>( geom_io->GetResEntry() );
        }

        /* If the meshes have changed we should re-instance */
        if ( resEntry ) {
            rwResEnty_ *rEntry = reinterpret_cast<rwResEnty_ *>( resEntry );
            rh::engine::IPrimitiveBatch *resEntryHeader;

            resEntryHeader = rEntry->modelInstance;
            if ( resEntryHeader->BatchId() != meshHeader->serialNum ) {
                /* Destroy resources to force reinstance */
                // RwResourcesFreeResEntry( resEntry );
                // RHDebug::DebugLogger::Log( "test" );
                resEntry = nullptr;
            }
        }
        if ( resEntry == nullptr ) {
            RwResEntry *&resEntryPointer = geom_io->GetResEntryRef();
            void *owner;

            meshHeader = geom_io->GetMeshHeader();

            if ( geom_io->GetMorphTargetCount() != 1 ) {
                owner = atomic;
                resEntryPointer = atomic->repEntry;
            } else {
                owner = atomic->geometry;
                resEntryPointer = geom_io->GetResEntryRef();
            }

            /* Create vertex buffers and instance */
            resEntry = RHInstanceAtomicGeometry( geom_io,
                                                 owner,
                                                 geom_io->GetResEntryRef(),
                                                 meshHeader );
            if ( resEntry == nullptr )
                return RenderStatus::Failure;

            /* The geometry is up to date */
            geom_io->Unlock();
            // geometry->lockedSinceLastInst = 0;
        }
        return RenderStatus::Instanced;
    } else
        return RenderStatus::Failure;
}

void MeshGetNumVerticesMinIndex( const uint16_t *indices,
                                 uint32_t size,
                                 uint32_t &numVertices,
                                 uint32_t &min )
{
    if ( size > 0 ) {
        uint32_t minVert = 0xFFFFFFFF;
        uint32_t maxVert = 0x00000000;

        /* Find min and max vertex index */
        for ( size_t i = 0; i < size; i++ ) {
            minVert = min( minVert, indices[i] );
            maxVert = max( maxVert, indices[i] );
        }

        numVertices = ( maxVert - minVert ) + 1;
        min = minVert;
    } else {
        numVertices = 0;
        min = 0;
    }
}

void DrawAtomic( RpAtomic *atomic,
                 RpGeometryInterface *geom_io,
                 rh::engine::IRenderingContext *context,
                 rh::engine::IRenderingPipeline *pipeline )
{
    geom_io->Init( atomic->geometry );
    rwResEnty_ *resEntry = reinterpret_cast<rwResEnty_ *>( geom_io->GetResEntry() );
    if ( resEntry && resEntry->modelInstance )
        pipeline->DrawMesh( context, resEntry->modelInstance );
}

void *RpGeometryRw36::GetResEntry()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->repEntry;
}

RwResEntry *&RpGeometryRw36::GetResEntryRef()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->repEntry;
}

int32_t RpGeometryRw36::GetVertexCount()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->numVertices;
}

int32_t RpGeometryRw36::GetMorphTargetCount()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->numMorphTargets;
}

RpMorphTarget *RpGeometryRw36::GetMorphTarget( uint32_t id )
{
    return &static_cast<RpGeometry *>( m_pGeometryImpl )->morphTarget[id];
}

RwTexCoords *RpGeometryRw36::GetTexCoordSetPtr( uint32_t id )
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->texCoords[id];
}

RwRGBA *RpGeometryRw36::GetVertexColorPtr()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->preLitLum;
}

void RpGeometryRw36::Unlock()
{
    static_cast<RpGeometry *>( m_pGeometryImpl )->lockedSinceLastInst = 0;
}

int32_t RpGeometryRw36::GetTriangleCount()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->numTriangles;
}

RpTriangle *RpGeometryRw36::GetTrianglePtr()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->triangles;
}

uint32_t RpGeometryRw36::GetFlags()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->flags;
}

RpMeshHeader *RpGeometryRw36::GetMeshHeader()
{
    return static_cast<RpGeometry *>( m_pGeometryImpl )->mesh;
}

void RpGeometryInterface::Init( void *geometry )
{
    m_pGeometryImpl = geometry;
}

} // namespace rw_rh_engine
