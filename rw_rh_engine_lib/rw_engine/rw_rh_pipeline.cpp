#include "rw_rh_pipeline.h"
#include "rw_api_injectors.h"
#include "system_funcs/rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/primitive_type.h>
#include <algorithm>
#include <common_headers.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{

struct RxTriangle
{
    uint16_t a, b, c;
};

static bool SortTriangles( const RxTriangle &pA, const RxTriangle &pB ) noexcept
{
    uint32_t sortedIndexA[3] = { pA.a, pA.b, pA.c };
    uint32_t sortedIndexB[3] = { pB.a, pB.b, pB.c };

    if ( sortedIndexA[0] > sortedIndexA[1] )
        std::swap( sortedIndexA[0], sortedIndexA[1] );

    if ( sortedIndexA[1] > sortedIndexA[2] )
    {
        std::swap( sortedIndexA[1], sortedIndexA[2] );

        if ( sortedIndexA[0] > sortedIndexA[1] )
        {
            std::swap( sortedIndexA[0], sortedIndexA[1] );
        }
    }

    if ( sortedIndexB[0] > sortedIndexB[1] )
    {
        std::swap( sortedIndexB[0], sortedIndexB[1] );
    }

    if ( sortedIndexB[1] > sortedIndexB[2] )
    {
        std::swap( sortedIndexB[1], sortedIndexB[2] );

        if ( sortedIndexB[0] > sortedIndexB[1] )
        {
            std::swap( sortedIndexB[0], sortedIndexB[1] );
        }
    }

    if ( sortedIndexA[0] == sortedIndexB[0] )
    {
        if ( sortedIndexA[1] == sortedIndexB[1] )
        {
            return ( sortedIndexA[2] < sortedIndexB[2] );
        }

        return ( sortedIndexA[1] < sortedIndexB[1] );
    }

    return ( sortedIndexA[0] < sortedIndexB[0] );
}

void GenerateNormals( VertexDescPosColorUVNormals *verticles,
                      uint32_t vertexCount, RpTriangle *triangles,
                      unsigned int triangleCount, bool /*isTriStrip*/ )
{
    // generate normal for each triangle and vertex in mesh
    for ( uint32_t i = 0; i < triangleCount; i++ )
    {
        const auto triangle = triangles[i];
        auto       iA = triangle.vertIndex[2], iB = triangle.vertIndex[1],
             iC = triangle.vertIndex[0];

        if ( iA == iB || iB == iC || iA == iC )
            continue;
        const auto vA = verticles[iA], vB = verticles[iB], vC = verticles[iC];
        // tangent vector
        RwV3d tangent = { vB.x - vA.x, vB.y - vA.y, vB.z - vA.z };
        // bitangent vector
        RwV3d bitangent = { vA.x - vC.x, vA.y - vC.y, vA.z - vC.z };
        // fix for triangle strips

        // float normalDirection = isTriStrip?(i % 2 == 0 ? 1.0f : -1.0f):1.0f;
        // normal vector as cross product of (tangent X bitangent)
        RwV3d normal = {
            ( tangent.y * bitangent.z - tangent.z * bitangent.y ),
            ( tangent.z * bitangent.x - tangent.x * bitangent.z ),
            ( tangent.x * bitangent.y - tangent.y * bitangent.x ) };
        // increase normals of each vertex in triangle
        /*verticles[iA].material_idx = triangle.matIndex;
        verticles[iB].material_idx = triangle.matIndex;
        verticles[iC].material_idx = triangle.matIndex;*/
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
    for ( uint32_t i = 0; i < vertexCount; i++ )
    {
        assert( !( isnan( verticles[i].nx ) || isnan( verticles[i].ny ) ||
                   isnan( verticles[i].nx ) ) );
        float length = sqrt( verticles[i].nx * verticles[i].nx +
                             verticles[i].ny * verticles[i].ny +
                             verticles[i].nz * verticles[i].nz );
        assert( !isnan( length ) );
        if ( length > 0.0f )
        {
            verticles[i].nx = verticles[i].nx / length;
            verticles[i].ny = verticles[i].ny / length;
            verticles[i].nz = verticles[i].nz / length;
        }
        else
        {
            // Better than 0 length vector
            verticles[i].nx = 0.5f;
            verticles[i].ny = 0.5f;
            verticles[i].nz = 0.5f;
        }
    }
}

RwResEntry *InstanceAtomicGeometry( RpGeometryInterface *geom_io, void *owner,
                                    RwResEntry **       resEntryPointer,
                                    const RpMeshHeader *meshHeader )
{
    using namespace rh::engine;
    ResEnty *resEntry;

    resEntry = reinterpret_cast<ResEnty *>(
        gRwDeviceGlobals.ResourceFuncs.AllocateResourceEntry(
            owner, resEntryPointer, sizeof( ResEnty ) - sizeof( RwResEntry ),
            []( RwResEntry *resEntry ) noexcept
            {
                auto *entry = reinterpret_cast<ResEnty *>( resEntry );
                if ( entry != nullptr )
                    DestroyBackendMesh( entry->meshData );
            } ) );

    *resEntryPointer = resEntry;
    if ( resEntry == nullptr )
        return nullptr;

    PrimitiveType primType = PrimitiveType::TriangleStrip;

    bool convert_to_list = false;
    if ( ( meshHeader->flags & rpMESHHEADERTRISTRIP ) != 0 )
    {
        primType        = PrimitiveType::TriangleList;
        convert_to_list = true;
    }
    else if ( ( meshHeader->flags & rpMESHHEADERPRIMMASK ) == 0 )
        primType = PrimitiveType::TriangleList;

    const auto *mesh_start = reinterpret_cast<const RpMesh *>( meshHeader + 1 );
    auto *      indexBuffer = new uint16_t[meshHeader->totalIndicesInMesh * 3];
    uint32_t    startIndex  = 0;
    uint32_t    indexCount;

    // Index data
    std::vector<GeometrySplit>    geometry_splits;
    std::vector<GeometryMaterial> geometry_mats;
    geometry_splits.reserve( meshHeader->numMeshes );
    geometry_mats.reserve( meshHeader->numMeshes );
    auto meshes = std::span( mesh_start, meshHeader->numMeshes );
    for ( auto &mesh : meshes )
    {
        auto          mesh_material = mesh.material;
        GeometrySplit meshData{};
        meshData.mIndexOffset = startIndex;

        indexCount = mesh.numIndices;
        MeshGetNumVerticesMinIndex( mesh.indices, indexCount,
                                    meshData.mVertexCount,
                                    meshData.mVertexOffset );
        GeometryMaterial material{};
        material.mDiffuseColor     = mesh_material->color;
        material.mSpecular         = mesh_material->surfaceProps.specular;
        material.mDiffuseRasterIdx = -1;

        geometry_mats.push_back( material );

        if ( convert_to_list )
        {
            size_t j = startIndex;
            for ( size_t i = startIndex; i < startIndex + indexCount - 2; i++ )
            {
                int idxA = 0, idxB = 1, idxC = 2;
                if ( ( i - startIndex ) & 1 )
                {
                    idxB = 2;
                    idxC = 1;
                }
                int16_t indx_a = mesh.indices[i - startIndex + idxA];
                int16_t indx_b = mesh.indices[i - startIndex + idxB];
                int16_t indx_c = mesh.indices[i - startIndex + idxC];
                // Skip degenerate tris
                if ( indx_a == indx_b || indx_b == indx_c || indx_a == indx_c )
                    continue;
                indexBuffer[j++] = indx_a;
                indexBuffer[j++] = indx_b;
                indexBuffer[j++] = indx_c;
            }
            indexCount = j - startIndex;
        }
        else
        {
            for ( size_t i = startIndex; i < startIndex + indexCount; i++ )
                indexBuffer[i] = mesh.indices[i - startIndex];
        }

        if ( primType == PrimitiveType::TriangleList )
        {
            std::sort(
                reinterpret_cast<RxTriangle *>( indexBuffer + startIndex ),
                reinterpret_cast<RxTriangle *>( indexBuffer + startIndex +
                                                indexCount ),
                SortTriangles );
        }
        meshData.mIndexCount = indexCount;
        startIndex += indexCount;

        geometry_splits.push_back( meshData );
    }

    // Vertex data
    auto *vertexData = new VertexDescPosColorUVNormals[static_cast<size_t>(
        geom_io->GetVertexCount() )];

    auto   morph_target = geom_io->GetMorphTarget( 0 );
    RwV3d *vertexPos    = morph_target->verts;
    RwV3d *normalsPtr   = morph_target->normals;

    RwTexCoords *vertexUV       = geom_io->GetTexCoordSetPtr( 0 );
    RwRGBA *     vertexColorPtr = geom_io->GetVertexColorPtr();

    uint32_t v_id = 0;
    for ( ; vertexPos != morph_target->verts + geom_io->GetVertexCount();
          vertexPos++ )
    {
        VertexDescPosColorUVNormals desc{};
        desc.x = vertexPos->x;
        desc.y = vertexPos->y;
        desc.z = vertexPos->z;
        desc.w = 1.0f;
        if ( normalsPtr )
        {
            desc.nx = normalsPtr->x;
            desc.ny = normalsPtr->y;
            desc.nz = normalsPtr->z;
            normalsPtr++;
        }
        else
        {
            desc.nx = desc.ny = desc.nz = 0;
        }
        if ( vertexColorPtr )
        {
            desc.color[0] = vertexColorPtr->red;
            desc.color[1] = vertexColorPtr->green;
            desc.color[2] = vertexColorPtr->blue;
            desc.color[3] = vertexColorPtr->alpha;
            vertexColorPtr++;
        }
        else
        {
            desc.color[0] = 255;
            desc.color[1] = 255;
            desc.color[2] = 255;
            desc.color[3] = 255;
        }
        if ( vertexUV )
        {
            desc.u = vertexUV->u;
            desc.v = vertexUV->v;
            vertexUV++;
        }
        else
        {
            desc.u = 0;
            desc.v = 0;
        }
        vertexData[v_id] = desc;
        v_id++;
    }
    if ( morph_target->normals == nullptr )
        GenerateNormals( vertexData,
                         static_cast<uint32_t>( geom_io->GetVertexCount() ),
                         geom_io->GetTrianglePtr(),
                         static_cast<uint32_t>( geom_io->GetTriangleCount() ),
                         primType == rh::engine::PrimitiveType::TriangleStrip );

    int j = 0;
    for ( const auto &split : geometry_splits )
    {
        for ( int i = split.mIndexOffset;
              i < split.mIndexOffset + split.mIndexCount; i++ )
        {
            auto &m_b = BackendMaterialPlugin::GetData( meshes[j].material );
            vertexData[indexBuffer[i]].material_idx = j;
            vertexData[indexBuffer[i]].emissive     = m_b.Emission;
        }
        j++;
    }

    BackendMeshInitData backendMeshInitData{};
    backendMeshInitData.mIndexCount = startIndex;
    backendMeshInitData.mVertexCount =
        static_cast<size_t>( geom_io->GetVertexCount() );
    backendMeshInitData.mIndexData  = indexBuffer;
    backendMeshInitData.mVertexData = vertexData;
    backendMeshInitData.mSplits     = geometry_splits;
    backendMeshInitData.mMaterials  = geometry_mats;
    resEntry->meshData              = CreateBackendMesh( backendMeshInitData );
    delete[] vertexData;
    delete[] indexBuffer;

    resEntry->batchId = meshHeader->serialNum;

    return reinterpret_cast<RwResEntry *>( resEntry );
}

RenderStatus InstanceAtomic( RpAtomic *atomic, RpGeometryInterface *geom_io )
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

    const uint32_t geomFlags = geom_io->GetFlags();

    RwResEntry *resEntry;

    if ( !( rpGEOMETRYNATIVE & geomFlags ) )
    {
        /* If the geometry has more than one morph target the resEntry in the
         * atomic is used else the resEntry in the geometry */
        if ( geom_io->GetMorphTargetCount() != 1 )
        {
            resEntry = atomic->repEntry;
        }
        else
        {
            resEntry = static_cast<RwResEntry *>( geom_io->GetResEntry() );
        }

        /* If the meshes have changed we should re-instance */
        if ( resEntry )
        {
            auto *rEntry = reinterpret_cast<ResEnty *>( resEntry );

            if ( rEntry->batchId != meshHeader->serialNum )
            {
                /* Destroy resources to force reinstance */
                gRwDeviceGlobals.ResourceFuncs.FreeResourceEntry( rEntry );
                resEntry = nullptr;
            }
        }
        if ( resEntry != nullptr )
            return RenderStatus::Instanced;
        RwResEntry **resEntryPointer = &geom_io->GetResEntryRef();
        void *       owner;
        meshHeader = geom_io->GetMeshHeader();
        if ( geom_io->GetMorphTargetCount() != 1 )
        {
            owner           = atomic;
            resEntryPointer = &atomic->repEntry;
        }
        else
        {
            owner           = atomic->geometry;
            resEntryPointer = &geom_io->GetResEntryRef();
        }
        resEntry = InstanceAtomicGeometry( geom_io, owner, resEntryPointer,
                                           meshHeader );
        if ( resEntry == nullptr )
            return RenderStatus::Failure;
        geom_io->Unlock();
        return RenderStatus::Instanced;
    }
    else
        return RenderStatus::Failure;
}

void MeshGetNumVerticesMinIndex( const uint16_t *indices, uint32_t size,
                                 uint32_t &numVertices, uint32_t &min )
{
    if ( size > 0 )
    {
        uint16_t minVert = 0xFFFF;
        uint16_t maxVert = 0x00000000;

        /* Find min and max vertex index */
        for ( size_t i = 0; i < size; i++ )
        {
            minVert = (std::min)( minVert, indices[i] );
            maxVert = (std::max)( maxVert, indices[i] );
        }

        numVertices = static_cast<uint32_t>( ( maxVert - minVert ) + 1 );
        min         = minVert;
    }
    else
    {
        numVertices = 0;
        min         = 0;
    }
}

void DrawAtomic( RpAtomic *atomic, RpGeometryInterface *geom_io,
                 const std::function<void( ResEnty * )> &render_callback )
{
    geom_io->Init( atomic->geometry );
    ResEnty *entry;
    if ( geom_io->GetMorphTargetCount() != 1 )
        entry = reinterpret_cast<ResEnty *>( atomic->repEntry );
    else
        entry = reinterpret_cast<ResEnty *>( geom_io->GetResEntry() );
    if ( render_callback )
        render_callback( entry );
    // if ( resEntry && resEntry->modelInstance )
    //    pipeline->DrawMesh( context, resEntry->modelInstance );
}

} // namespace rh::rw::engine
