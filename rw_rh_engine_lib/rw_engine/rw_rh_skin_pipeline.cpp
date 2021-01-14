//
// Created by peter on 12.05.2020.
//

#include "rw_rh_skin_pipeline.h"
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <Engine/Common/types/primitive_type.h>
#include <common.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rh_backend/skinned_mesh_backend.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
#include <scene_graph.h>

namespace rh::rw::engine
{

void GenerateSkinNormals( VertexDescPosColorUVNormals *verticles,
                          uint32_t vertexCount, RpTriangle *triangles,
                          unsigned int triangleCount, bool /*isTriStrip*/ )
{
    // generate normal for each triangle and vertex in mesh
    for ( uint32_t i = 0; i < triangleCount; i++ )
    {
        auto triangle = triangles[i];
        auto iA = triangle.vertIndex[2], iB = triangle.vertIndex[1],
             iC = triangle.vertIndex[0];

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
        float length = sqrt( verticles[i].nx * verticles[i].nx +
                             verticles[i].ny * verticles[i].ny +
                             verticles[i].nz * verticles[i].nz );
        if ( length > 0.000001f )
        {
            verticles[i].nx = verticles[i].nx / length;
            verticles[i].ny = verticles[i].ny / length;
            verticles[i].nz = verticles[i].nz / length;
        }
    }
}

RwResEntry *RHInstanceSkinAtomicGeometry( RpGeometryInterface *geom_io,
                                          void *               owner,
                                          RwResEntry *&        resEntryPointer,
                                          const RpMeshHeader * meshHeader,
                                          RpHAnimHierarchy *   pHierarchy,
                                          RwFrame *            pFrame )
{
    ResEnty *resEntry;

    resEntry = reinterpret_cast<ResEnty *>(
        DeviceGlobals::ResourceFuncs.AllocateResourceEntry(
            owner, &resEntryPointer, sizeof( ResEnty ) - sizeof( RwResEntry ),
            []( RwResEntry *resEntry ) noexcept {
                auto *entry = reinterpret_cast<ResEnty *>( resEntry );
                if ( entry != nullptr )
                    DestroySkinMesh( entry->meshData );
            } ) );

    resEntryPointer = resEntry;
    if ( resEntry == nullptr )
        return nullptr;

    rh::engine::PrimitiveType primType =
        rh::engine::PrimitiveType::TriangleStrip;

    bool convert_to_list = false;
    if ( meshHeader->flags & rpMESHHEADERTRISTRIP )
    {
        primType        = rh::engine::PrimitiveType::TriangleList;
        convert_to_list = true;
    }
    else if ( ( meshHeader->flags & rpMESHHEADERPRIMMASK ) == 0 )
        primType = rh::engine::PrimitiveType::TriangleList;

    const auto *mesh_start = reinterpret_cast<const RpMesh *>( meshHeader + 1 );
    auto *      indexBuffer = new uint16_t[meshHeader->totalIndicesInMesh * 3];
    uint32_t    startIndex  = 0;
    uint32_t    indexCount  = 0;

    // Index data
    std::vector<GeometrySplit>    geometry_splits;
    std::vector<GeometryMaterial> geometry_mats;
    for ( const RpMesh *mesh = mesh_start;
          mesh != mesh_start + meshHeader->numMeshes; mesh++ )
    {
        GeometrySplit meshData{};
        meshData.mIndexOffset = startIndex;
        auto mat_ext          = GetBackendMaterialExt( mesh->material );
        if ( mat_ext->mMaterialId == 0xBADF00D )
            mat_ext->mMaterialId = CreateMaterialData( mesh->material );
        meshData.mMaterialIdx = mat_ext->mMaterialId; // mesh->material;

        indexCount = mesh->numIndices;
        MeshGetNumVerticesMinIndex( mesh->indices, indexCount,
                                    meshData.mVertexCount,
                                    meshData.mVertexOffset );
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
                int16_t indx_a = mesh->indices[i - startIndex + idxA];
                int16_t indx_b = mesh->indices[i - startIndex + idxB];
                int16_t indx_c = mesh->indices[i - startIndex + idxC];
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
                indexBuffer[i] = mesh->indices[i - startIndex];
        }

        if ( primType == rh::engine::PrimitiveType::TriangleList )
        {
            /*std::sort(
                reinterpret_cast<RxTriangle *>( indexBuffer + startIndex ),
                reinterpret_cast<RxTriangle *>( indexBuffer + startIndex +
                                                indexCount ),
                SortTriangles );*/
        }
        meshData.mIndexCount = indexCount;
        startIndex += indexCount;

        geometry_splits.push_back( meshData );
    }

    auto skin = DeviceGlobals::SkinFuncs.GeometryGetSkin(
        static_cast<const RpGeometry *>( geom_io->GetThis() ) );

    // Vertex data
    auto *vertexData = new VertexDescPosColorUVNormals[static_cast<size_t>(
        geom_io->GetVertexCount() )];

    auto                   morph_target   = geom_io->GetMorphTarget( 0 );
    RwV3d *                vertexPos      = morph_target->verts;
    RwTexCoords *          vertexUV       = geom_io->GetTexCoordSetPtr( 0 );
    RwRGBA *               vertexColorPtr = geom_io->GetVertexColorPtr();
    RwV3d *                normalsPtr     = morph_target->normals;
    const RwMatrixWeights *weightsPtr =
        DeviceGlobals::SkinFuncs.GetVertexBoneWeights( skin );
    const uint32_t *boneIdsPtr =
        DeviceGlobals::SkinFuncs.GetVertexBoneIndices( skin );

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
        if ( weightsPtr )
        {
            desc.wx = weightsPtr->w0;
            desc.wy = weightsPtr->w1;
            desc.wz = weightsPtr->w2;
            desc.ww = weightsPtr->w3;
            weightsPtr++;
        }
        else
        {
            desc.wx = 0;
            desc.wy = 0;
            desc.wz = 0;
            desc.ww = 0;
        }
        if ( boneIdsPtr )
        {
            desc.bone_indices = *boneIdsPtr;
            boneIdsPtr++;
        }
        else
        {
            desc.bone_indices = 0;
        }
        vertexData[v_id] = desc;
        v_id++;
    }
    GenerateSkinNormals( vertexData,
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
            vertexData[indexBuffer[i]].material_idx = j;
        }
        j++;
    }

    SkinnedMeshInitData backendMeshInitData{};
    backendMeshInitData.mIndexCount = startIndex;
    backendMeshInitData.mVertexCount =
        static_cast<size_t>( geom_io->GetVertexCount() );
    backendMeshInitData.mIndexData =
        reinterpret_cast<uint16_t *>( indexBuffer );
    backendMeshInitData.mVertexData = vertexData;
    backendMeshInitData.mSplits     = geometry_splits;
    resEntry->meshData              = CreateSkinMesh( backendMeshInitData );
    delete[] vertexData;
    delete[] indexBuffer;

    resEntry->batchId = meshHeader->serialNum;

    return reinterpret_cast<RwResEntry *>( resEntry );
}

DirectX::XMFLOAT4X3 RwMatrixToDxMatrix( const RwMatrix *mtx )
{
    return { mtx->right.x, mtx->right.y, mtx->right.z, mtx->up.x,
             mtx->up.y,    mtx->up.z,    mtx->at.x,    mtx->at.y,
             mtx->at.z,    mtx->pos.x,   mtx->pos.y,   mtx->pos.z };
}

void PrepareBoneMatrices( DirectX::XMFLOAT4X3 *matrix_cache, RpAtomic *atomic,
                          IAnimHierarcy &anim_hier )
{
    auto _anim_hier =
        DeviceGlobals::SkinFuncs.AtomicGetHAnimHierarchy( atomic );
    auto skin = DeviceGlobals::SkinFuncs.GeometryGetSkin( atomic->geometry );
    if ( !_anim_hier || !skin )
        return;
    anim_hier.Init( _anim_hier );
    auto *skinToBoneMatrices =
        DeviceGlobals::SkinFuncs.GetSkinToBoneMatrices( skin );

    auto frame = static_cast<RwFrame *>( rwObject::GetParent( atomic ) );
    auto atomic_transform = rw::engine::RwFrameGetLTM( frame );

    DirectX::XMFLOAT4X3 world_transform =
        RwMatrixToDxMatrix( atomic_transform );
    DirectX::XMMATRIX world_inv_transform = DirectX::XMMatrixInverse(
        nullptr, DirectX::XMLoadFloat4x3( &world_transform ) );

    if ( anim_hier.GetFlags() & 2 ) // rpHANIMHIERARCHYNOMATRICES
    {
        for ( int i = 0; i < anim_hier.GetNumNodes(); i++ )
        {
            DirectX::XMFLOAT4X3 skin_to_bone_mtx_d3d =
                RwMatrixToDxMatrix( &skinToBoneMatrices[i] );
            DirectX::XMFLOAT4X3 frame_mtx_d3d = RwMatrixToDxMatrix(
                RwFrameGetLTM( anim_hier.GetNodeInfo()[i].pFrame ) );

            DirectX::XMMATRIX bone_transform =
                DirectX::XMLoadFloat4x3( &frame_mtx_d3d );
            DirectX::XMMATRIX skin_to_bone =
                DirectX::XMLoadFloat4x3( &skin_to_bone_mtx_d3d );

            auto temp = skin_to_bone * bone_transform * world_inv_transform;

            DirectX::XMStoreFloat4x3( &matrix_cache[i], temp );
        }
    }
    else
    {
        if ( anim_hier.GetFlags() &
             0x4000 ) // rpHANIMHIERARCHYLOCALSPACEMATRICES
        {
            for ( int i = 0; i < anim_hier.GetNumNodes(); i++ )
            {

                DirectX::XMFLOAT4X3 skin_to_bone_mtx_d3d =
                    RwMatrixToDxMatrix( &skinToBoneMatrices[i] );
                DirectX::XMFLOAT4X3 frame_mtx_d3d =
                    RwMatrixToDxMatrix( &anim_hier.GetSkinToBoneMatrices()[i] );

                DirectX::XMMATRIX bone_transform =
                    DirectX::XMLoadFloat4x3( &frame_mtx_d3d );
                DirectX::XMMATRIX skin_to_bone =
                    DirectX::XMLoadFloat4x3( &skin_to_bone_mtx_d3d );

                auto temp = skin_to_bone * bone_transform;

                DirectX::XMStoreFloat4x3( &matrix_cache[i], temp );
            }
        }
        else
        {
            for ( int i = 0; i < anim_hier.GetNumNodes(); i++ )
            {
                DirectX::XMFLOAT4X3 skin_to_bone_mtx_d3d =
                    RwMatrixToDxMatrix( &skinToBoneMatrices[i] );
                DirectX::XMFLOAT4X3 frame_mtx_d3d =
                    RwMatrixToDxMatrix( &anim_hier.GetSkinToBoneMatrices()[i] );

                DirectX::XMMATRIX bone_transform =
                    DirectX::XMLoadFloat4x3( &frame_mtx_d3d );
                DirectX::XMMATRIX skin_to_bone =
                    DirectX::XMLoadFloat4x3( &skin_to_bone_mtx_d3d );

                auto temp = skin_to_bone * bone_transform * world_inv_transform;

                DirectX::XMStoreFloat4x3( &matrix_cache[i], temp );
            }
        }
    }
}

RenderStatus RwRHInstanceSkinAtomic( RpAtomic *           atomic,
                                     RpGeometryInterface *geom_io )
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

    RwResEntry *resEntry = nullptr;

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
            // rh::engine::IPrimitiveBatch *resEntryHeader;
            // TODO: Deal with updates
            // if ( rEntry->batchId != meshHeader->serialNum )
            //{
            /* Destroy resources to force reinstance */
            /* if ( rEntry->frameId != GetCurrentSceneGraph()->mFrameId )
             {
                 DeviceGlobals::ResourceFuncs.FreeResourceEntry( resEntry );
                 // RHDebug::DebugLogger::Log( "test" );
                 rEntry->meshData = 0xBADF00D;
                 resEntry         = nullptr;
             }*/
            //}
        }
        if ( resEntry != nullptr )
            return RenderStatus::Instanced;
        RwResEntry *&resEntryPointer = geom_io->GetResEntryRef();
        void *       owner;
        meshHeader = geom_io->GetMeshHeader();
        if ( geom_io->GetMorphTargetCount() != 1 )
        {
            owner           = atomic;
            resEntryPointer = atomic->repEntry;
        }
        else
        {
            owner           = atomic->geometry;
            resEntryPointer = geom_io->GetResEntryRef();
        }

        auto anim_hier =
            DeviceGlobals::SkinFuncs.AtomicGetHAnimHierarchy( atomic );
        auto frame = static_cast<RwFrame *>( rwObject::GetParent( atomic ) );

        resEntry = RHInstanceSkinAtomicGeometry( geom_io, owner,
                                                 geom_io->GetResEntryRef(),
                                                 meshHeader, anim_hier, frame );
        if ( resEntry == nullptr )
            return RenderStatus::Failure;
        geom_io->Unlock();
        return RenderStatus::Instanced;
    }
    else
        return RenderStatus::Failure;
    return NotInstanced;
}
} // namespace rh::rw::engine