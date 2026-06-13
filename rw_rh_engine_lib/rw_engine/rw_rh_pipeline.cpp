#include "rw_rh_pipeline.h"
#include "rw_api_injectors.h"
#include "system_funcs/rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/primitive_type.h>
#include <algorithm>
#include <common_headers.h>
#include <queue>
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
                                    RwResEntry        **resEntryPointer,
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
    // Vertex data
    std::vector<VertexDescPosColorUVNormals> vertex_data{
        static_cast<size_t>( geom_io->GetVertexCount() ) };
    size_t orig_vertex_count = vertex_data.size();

    auto   morph_target = geom_io->GetMorphTarget( 0 );
    RwV3d *vertexPos    = morph_target->verts;
    RwV3d *normalsPtr   = morph_target->normals;

    RwTexCoords *vertexUV       = geom_io->GetTexCoordSetPtr( 0 );
    RwRGBA      *vertexColorPtr = geom_io->GetVertexColorPtr();

    auto    *indexBuffer = new uint16_t[meshHeader->totalIndicesInMesh * 3];
    uint32_t startIndex  = 0;
    uint32_t indexCount;

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
            // Problem:
            // In RenderWare it so happens that 3dsMax plugins for exporting DFF
            // files, allowed to set "Export 2 sided" flag, that resulted in
            // duplication of triangles, to simulate multisided polygons, but
            // they used same indices for such triangles in triangle strips,
            // just flipped. We want to generate normals for such geometry,
            // especially if original game doesn't provide them. To do that we
            // need to duplicate such vertices, so that they get separate
            // normals for inside and outside facing geometry. It'd be wiser -
            // to remove backfaces entirely, and set some material property, but
            // order of triangles isn't preserved, that means simple
            // deduplication strategy(removing duplicate triangle in order of
            // traversal) - doesn't work.
            // To fix that - we do the following:
            // 1. Find all duplicate triangle keys.
            // 2. Split double-sided triangles, from single sided ones.
            // 3. Duplicate such triangles vertices, to avoid normal
            // self-destruction.
            struct TriKey
            {
                TriKey( uint16_t a, uint16_t b, uint16_t c )
                {
                    ids[0] = a;
                    ids[1] = b;
                    ids[2] = c;
                    std::sort( ids, ids + 3 );
                }
                uint16_t ids[3];
                bool     operator<( const TriKey &other ) const
                {
                    if ( ids[0] != other.ids[0] )
                        return ids[0] < other.ids[0];
                    if ( ids[1] != other.ids[1] )
                        return ids[1] < other.ids[1];
                    return ids[2] < other.ids[2];
                }
            };
            std::set<TriKey> dup_tris;
            auto             is_degenerate_tri =
                []( uint16_t indx_a, uint16_t indx_b, uint16_t indx_c )
            {
                return indx_a == indx_b || indx_b == indx_c || indx_a == indx_c;
            };

            uint32_t tri_count = 0;
            {
                std::set<TriKey> processed_triangles;
                for ( size_t i = startIndex; i < startIndex + indexCount - 2;
                      i++ )
                {
                    int      idxA = 0, idxB = 1, idxC = 2;
                    uint16_t indx_a = mesh.indices[i - startIndex + idxA];
                    uint16_t indx_b = mesh.indices[i - startIndex + idxB];
                    uint16_t indx_c = mesh.indices[i - startIndex + idxC];

                    if ( is_degenerate_tri( indx_a, indx_b, indx_c ) )
                    {
                        continue;
                    }
                    tri_count++;
                    TriKey key{ indx_a, indx_b, indx_c };
                    if ( processed_triangles.contains( key ) )
                    {
                        dup_tris.insert( key );
                        continue;
                    }

                    processed_triangles.insert( key );
                }
            }
            std::vector<RxTriangle> duplicate_triangles{};
            std::vector<RxTriangle> all_triangles{};
            all_triangles.reserve( tri_count );
            for ( size_t i = startIndex; i < startIndex + indexCount - 2; i++ )
            {
                int      idxA = 0, idxB = 1, idxC = 2;
                uint16_t indx_a = mesh.indices[i - startIndex + idxA];
                uint16_t indx_b = mesh.indices[i - startIndex + idxB];
                uint16_t indx_c = mesh.indices[i - startIndex + idxC];
                // Skip degenerate tris
                if ( is_degenerate_tri( indx_a, indx_b, indx_c ) )
                {
                    continue;
                }
                if ( i % 2 != 0 )
                {
                    // Флипаем, чтобы все смотрели в одну сторону
                    std::swap( indx_b, indx_c );
                }
                TriKey key{ indx_a, indx_b, indx_c };
                if ( dup_tris.contains( key ) )
                {
                    duplicate_triangles.push_back( { indx_a, indx_b, indx_c } );
                }
                else
                {
                    all_triangles.push_back( { indx_a, indx_b, indx_c } );
                }
            }
            auto duplicate_vertex_data = [&]( uint16_t id ) -> uint16_t
            {
                uint16_t new_idx = static_cast<uint16_t>( vertex_data.size() );
                auto     new_vtx_data = vertex_data[id];
                new_vtx_data.x        = vertexPos[id].x;
                new_vtx_data.y        = vertexPos[id].y;
                new_vtx_data.z        = vertexPos[id].z;
                new_vtx_data.w        = 1.f;
                new_vtx_data.nx       = 0.0f;
                new_vtx_data.ny       = 0.0f;
                new_vtx_data.nz       = 0.0f;
                if ( normalsPtr )
                {
                    new_vtx_data.nx = normalsPtr[id].x;
                    new_vtx_data.ny = normalsPtr[id].y;
                    new_vtx_data.nz = normalsPtr[id].z;
                }
                if ( vertexColorPtr )
                {
                    new_vtx_data.color[0] = vertexColorPtr[id].red;
                    new_vtx_data.color[1] = vertexColorPtr[id].green;
                    new_vtx_data.color[2] = vertexColorPtr[id].blue;
                    new_vtx_data.color[3] = vertexColorPtr[id].alpha;
                }
                else
                {
                    new_vtx_data.color[0] = 255;
                    new_vtx_data.color[1] = 255;
                    new_vtx_data.color[2] = 255;
                    new_vtx_data.color[3] = 255;
                }
                if ( vertexUV )
                {
                    new_vtx_data.u = vertexUV[id].u;
                    new_vtx_data.v = vertexUV[id].v;
                }
                else
                {
                    new_vtx_data.u = 0;
                    new_vtx_data.v = 0;
                }
                vertex_data.push_back( new_vtx_data );
                return new_idx;
            };
            auto get_vpos = [&]( uint16_t idx )
            {
                if ( idx >= orig_vertex_count )
                {
                    auto &vdecl = vertex_data[idx];
                    return RwV3d{ vdecl.x, vdecl.y, vdecl.z };
                }
                return vertexPos[idx];
            };
            for ( auto tri : duplicate_triangles )
            {
                all_triangles.emplace_back( duplicate_vertex_data( tri.a ),
                                            duplicate_vertex_data( tri.b ),
                                            duplicate_vertex_data( tri.c ) );
            }
            assert( all_triangles.size() != 0 );
            for ( auto tri : all_triangles )
            {
                indexBuffer[j++] = tri.a;
                indexBuffer[j++] = tri.b;
                indexBuffer[j++] = tri.c;
            }
            indexCount = j - startIndex;

            if ( !normalsPtr )
            {
                for ( size_t i = startIndex; i < startIndex + indexCount;
                      i += 3 )
                {
                    uint16_t indx_a = indexBuffer[i];
                    uint16_t indx_b = indexBuffer[i + 1];
                    uint16_t indx_c = indexBuffer[i + 2];

                    const auto vA = get_vpos( indx_a );
                    const auto vB = get_vpos( indx_b );
                    const auto vC = get_vpos( indx_c );

                    RwV3d tangent   = { vB.x - vA.x, vB.y - vA.y, vB.z - vA.z };
                    RwV3d bitangent = { vC.x - vA.x, vC.y - vA.y, vC.z - vA.z };

                    RwV3d normal = {
                        ( tangent.y * bitangent.z - tangent.z * bitangent.y ),
                        ( tangent.z * bitangent.x - tangent.x * bitangent.z ),
                        ( tangent.x * bitangent.y - tangent.y * bitangent.x ) };
                    float len =
                        sqrt( normal.x * normal.x + normal.y * normal.y +
                              normal.z * normal.z );
                    if ( len > 0.0f )
                    {
                        normal.x /= len;
                        normal.y /= len;
                        normal.z /= len;
                    }
                    vertex_data[indx_a].nx += normal.x;
                    vertex_data[indx_a].ny += normal.y;
                    vertex_data[indx_a].nz += normal.z;
                    vertex_data[indx_b].nx += normal.x;
                    vertex_data[indx_b].ny += normal.y;
                    vertex_data[indx_b].nz += normal.z;
                    vertex_data[indx_c].nx += normal.x;
                    vertex_data[indx_c].ny += normal.y;
                    vertex_data[indx_c].nz += normal.z;
                }
            }
            else
            {
                // Correct winding order, if normals are known
                // This is some weird bug I see in GTA VC, specifically on
                // vehicles, not sure what's the reason.
                // If someone finds out another solution to such problem -
                // commits are welcome.
                for ( size_t i = startIndex; i < startIndex + indexCount;
                      i += 3 )
                {
                    uint16_t indx_a = indexBuffer[i];
                    uint16_t indx_b = indexBuffer[i + 1];
                    uint16_t indx_c = indexBuffer[i + 2];

                    const auto vA   = get_vpos( indx_a );
                    const auto vB   = get_vpos( indx_b );
                    const auto vC   = get_vpos( indx_c );
                    RwV3d tangent   = { vB.x - vA.x, vB.y - vA.y, vB.z - vA.z };
                    RwV3d bitangent = { vC.x - vA.x, vC.y - vA.y, vC.z - vA.z };

                    RwV3d geomNormal = {
                        ( tangent.y * bitangent.z - tangent.z * bitangent.y ),
                        ( tangent.z * bitangent.x - tangent.x * bitangent.z ),
                        ( tangent.x * bitangent.y - tangent.y * bitangent.x ) };
                    float len = sqrt( geomNormal.x * geomNormal.x +
                                      geomNormal.y * geomNormal.y +
                                      geomNormal.z * geomNormal.z );
                    if ( len > 0.0f )
                    {
                        geomNormal.x /= len;
                        geomNormal.y /= len;
                        geomNormal.z /= len;
                    }
                    const auto nA         = normalsPtr[indx_a];
                    const auto nB         = normalsPtr[indx_b];
                    const auto nC         = normalsPtr[indx_c];
                    RwV3d      meshNormal = { ( nA.x + nB.x + nC.x ),
                                              ( nA.y + nB.y + nC.y ),
                                              ( nA.z + nB.z + nC.z ) };
                    auto       norm_dir   = meshNormal.x * geomNormal.x +
                                    meshNormal.y * geomNormal.y +
                                    meshNormal.z * geomNormal.z;
                    if ( norm_dir < 0.f )
                    {
                        std::swap( indexBuffer[i], indexBuffer[i + 1] );
                    }
                }
            }
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

    uint32_t v_id = 0;
    for ( ; vertexPos != morph_target->verts + geom_io->GetVertexCount();
          vertexPos++ )
    {
        VertexDescPosColorUVNormals desc{ vertex_data[v_id] };
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
            if ( !convert_to_list )
            {
                desc.nx = desc.ny = desc.nz = 0;
            }
            else
            {
                auto l = sqrt( desc.nx * desc.nx + desc.ny * desc.ny +
                               desc.nz * desc.nz );
                if ( l > 0.f )
                {
                    desc.nx /= l;
                    desc.ny /= l;
                    desc.nz /= l;
                }
            }
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
        vertex_data[v_id] = desc;
        v_id++;
    }
    if ( morph_target->normals == nullptr &&
         !convert_to_list ) // < tri strip has a normal generation beforehand
        GenerateNormals( vertex_data.data(), vertex_data.size(),
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
            vertex_data[indexBuffer[i]].material_idx = j;
            vertex_data[indexBuffer[i]].emissive     = m_b.Emission;
        }
        j++;
    }

    BackendMeshInitData backendMeshInitData{};
    backendMeshInitData.mIndexCount  = startIndex;
    backendMeshInitData.mVertexCount = vertex_data.size();
    backendMeshInitData.mIndexData   = indexBuffer;
    backendMeshInitData.mVertexData  = vertex_data.data();
    backendMeshInitData.mSplits      = geometry_splits;
    backendMeshInitData.mMaterials   = geometry_mats;
    resEntry->meshData               = CreateBackendMesh( backendMeshInitData );
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
        void        *owner;
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
