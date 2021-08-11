//
// Created by peter on 27.06.2020.
//

#include "RTSceneDescription.h"
#include "rendering_loop/DescriptorGenerator.h"
#include "scene_description/gpu_mesh_buffer_pool.h"
#include "scene_description/gpu_scene_materials_pool.h"
#include "scene_description/gpu_texture_pool.h"
#include <Engine/Common/IDeviceState.h>
#include <render_client/mesh_instance_state_recorder.h>
#include <render_driver/gpu_resources/resource_mgr.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{

using namespace rh::engine;

constexpr auto SceneDescCallbacksId = 0x52;

RTSceneDescription::RTSceneDescription(
    const RTSceneDescriptionCreateInfo &info )
    : Device( info.Device ), Resources( info.Resources )
{

    constexpr auto draw_count_limit     = 10000;
    constexpr auto model_count_limit    = 20000;
    constexpr auto texture_count_limit  = 20000;
    constexpr auto material_count_limit = 20001;

    constexpr auto scene_desc_bind_id       = 0;
    constexpr auto vertex_buff_desc_bind_id = 1;
    constexpr auto index_buff_bind_id       = 2;
    constexpr auto texture_desc_bind_id     = 3;
    constexpr auto material_buff_bind_id    = 4;

    mSceneDesc.resize( draw_count_limit );
    mSceneMaterials.resize( material_count_limit );

    DescriptorGenerator descriptorGenerator{ Device };
    // Scene desc
    descriptorGenerator.AddDescriptor(
        0, scene_desc_bind_id, 0, DescriptorType::RWBuffer, 1,
        ShaderStage::Compute | ShaderStage::RayHit | ShaderStage::RayGen |
            ShaderStage::RayAnyHit );
    // Materials
    descriptorGenerator.AddDescriptor(
        0, material_buff_bind_id, 0, DescriptorType::RWBuffer, 1,
        ShaderStage::Compute | ShaderStage::RayHit | ShaderStage::RayGen |
            ShaderStage::RayAnyHit );
    // Verts
    descriptorGenerator.AddDescriptor(
        0, vertex_buff_desc_bind_id, 0, DescriptorType::RWBuffer,
        model_count_limit,
        ShaderStage::Compute | ShaderStage::RayHit | ShaderStage::RayGen |
            ShaderStage::RayAnyHit,
        DescriptorFlags::dfPartiallyBound |
            DescriptorFlags::dfUpdateUnusedWhilePending );
    // Indices
    descriptorGenerator.AddDescriptor(
        0, index_buff_bind_id, 0, DescriptorType::RWBuffer, model_count_limit,
        ShaderStage::Compute | ShaderStage::RayHit | ShaderStage::RayGen |
            ShaderStage::RayAnyHit,
        DescriptorFlags::dfPartiallyBound |
            DescriptorFlags::dfUpdateUnusedWhilePending );
    // Textures
    descriptorGenerator.AddDescriptor(
        0, texture_desc_bind_id, 0, DescriptorType::ROTexture,
        texture_count_limit,
        ShaderStage::Compute | ShaderStage::RayHit | ShaderStage::RayGen |
            ShaderStage::RayAnyHit,
        DescriptorFlags::dfPartiallyBound |
            DescriptorFlags::dfUpdateUnusedWhilePending );

    // descriptor layout
    mSceneSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 1 );

    // Desc set allocator
    mDescSetAlloc = descriptorGenerator.FinalizeAllocator();

    // Allocate scene set
    std::vector layout_array = {
        static_cast<IDescriptorSetLayout *>( mSceneSetLayout ) };
    mSceneSet = mDescSetAlloc->AllocateDescriptorSets( { layout_array } )[0];

    mTexturePool = new GPUTexturePool(
        { Device, mSceneSet, texture_count_limit, texture_desc_bind_id } );
    mModelBuffersPool = new GPUModelBuffersPool(
        { Device, mSceneSet, model_count_limit, index_buff_bind_id,
          vertex_buff_desc_bind_id } );

    mSceneDescBuffer = Device.CreateBuffer(
        { .mSize        = sizeof( SceneObjDesc ) * draw_count_limit,
          .mUsage       = BufferUsage::StorageBuffer,
          .mFlags       = BufferFlags::Dynamic,
          .mInitDataPtr = nullptr } );

    mMaterialDescBuffer = Device.CreateBuffer(
        { .mSize        = sizeof( MaterialData ) * material_count_limit,
          .mUsage       = BufferUsage::StorageBuffer,
          .mFlags       = BufferFlags::Dynamic,
          .mInitDataPtr = nullptr } );

    std::array sdesc_buff_ui = {
        BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mSceneDescBuffer } };
    std::array mdesc_buff_ui = {
        BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mMaterialDescBuffer } };

    Device.UpdateDescriptorSets( { .mSet            = mSceneSet,
                                   .mBinding        = scene_desc_bind_id,
                                   .mDescriptorType = DescriptorType::RWBuffer,
                                   .mBufferUpdateInfo = sdesc_buff_ui } );
    Device.UpdateDescriptorSets( { .mSet            = mSceneSet,
                                   .mBinding        = material_buff_bind_id,
                                   .mDescriptorType = DescriptorType::RWBuffer,
                                   .mBufferUpdateInfo = mdesc_buff_ui } );

    /// Setup callbacks
    auto &raster_pool = Resources.GetRasterPool();
    auto &mesh_pool   = Resources.GetMeshPool();

    mesh_pool.AddOnRequestCallback(
        [this]( BackendMeshData &data, uint64_t id )
        {
            // Update global vb/ib descriptor set

            mModelBuffersPool->StoreModel( data, id );
        },
        SceneDescCallbacksId );

    mesh_pool.AddOnDestructCallback(
        [this]( BackendMeshData &data, uint64_t id )
        { mModelBuffersPool->RemoveModel( id ); },
        SceneDescCallbacksId );
    raster_pool.AddOnDestructCallback( [this]( RasterData &data, uint64_t id )
                                       { mTexturePool->RemoveTexture( id ); },
                                       SceneDescCallbacksId );
}

RTSceneDescription::~RTSceneDescription()
{
    auto &raster_pool = Resources.GetRasterPool();
    auto &mesh_pool   = Resources.GetMeshPool();
    mesh_pool.RemoveOnDestructCallback( SceneDescCallbacksId );
    mesh_pool.RemoveOnRequestCallback( SceneDescCallbacksId );
    raster_pool.RemoveOnDestructCallback( SceneDescCallbacksId );
}

rh::engine::IDescriptorSetLayout *RTSceneDescription::DescLayout()
{
    return mSceneSetLayout;
}
rh::engine::IDescriptorSet *RTSceneDescription::DescSet() { return mSceneSet; }
void                        RTSceneDescription::Update()
{

    mSceneDescBuffer->Update( mSceneDesc.data(),
                              mDrawCalls * sizeof( SceneObjDesc ) );
    mMaterialDescBuffer->Update( mSceneMaterials.data(),
                                 mMaterials * sizeof( MaterialData ) );
    // mSceneMaterialsPool->ResetFrame();
    mDrawCalls = 0;
    mMaterials = 0;
}

void RTSceneDescription::RecordDrawCall( const DrawCallInfo &dc,
                                         const MaterialData *materials,
                                         uint64_t            material_count )
{
    SceneObjDesc &obj_desc    = mSceneDesc[mDrawCalls];
    auto &        raster_pool = Resources.GetRasterPool();
    auto &        mesh_pool   = Resources.GetMeshPool();

    const auto &mesh = mesh_pool.GetResource( dc.MeshId );
    for ( auto i = 0; i < material_count; i++ )
    {
        mSceneMaterials[i + mMaterials] = materials[i];
        auto get_pool_id = [this, &raster_pool]( auto orig_tex_id )
        {
            if ( orig_tex_id == BackendRasterPlugin::NullRasterId )
                return -1;
            auto tex_pool_id = mTexturePool->GetTexId( orig_tex_id );
            if ( tex_pool_id < 0 )
            {
                auto img_view =
                    raster_pool.GetResource( orig_tex_id ).mImageView;
                tex_pool_id =
                    mTexturePool->StoreTexture( img_view, orig_tex_id );
            }
            return tex_pool_id;
        };

        auto tex_pool_id =
            get_pool_id( mSceneMaterials[i + mMaterials].mTexture );
        auto spec_tex_pool_id =
            get_pool_id( mSceneMaterials[i + mMaterials].mSpecTexture );

        mSceneMaterials[i + mMaterials].mTexture     = tex_pool_id;
        mSceneMaterials[i + mMaterials].mSpecTexture = spec_tex_pool_id;
    }

    obj_desc.objId         = mModelBuffersPool->GetModelId( dc.MeshId );
    obj_desc.txtOffset     = mMaterials;
    obj_desc.triangleCount = mesh.mIndexCount / 3;

    std::copy( &dc.WorldTransform.m[0][0], &dc.WorldTransform.m[0][0] + 3 * 4,
               &obj_desc.transform.m[0][0] );
    obj_desc.transform.m[3][3] = 1.0f;

    auto it_mtx = DirectX::XMMatrixTranspose( DirectX::XMMatrixInverse(
        nullptr, DirectX::XMMatrixTranspose(
                     DirectX::XMLoadFloat4x4( &obj_desc.transform ) ) ) );
    DirectX::XMStoreFloat4x4( &obj_desc.transfomIT, it_mtx );

    if ( dc.DrawCallId != 0 )
    {
        // update prev transform
        // TODO:
        auto iter = mPrevTransformMap.find( dc.DrawCallId );
        if ( iter != mPrevTransformMap.end() )
            obj_desc.prevTransfom = iter->second;
        else
            DirectX::XMStoreFloat4x4( &obj_desc.prevTransfom, it_mtx );

        mPrevTransformMap[dc.DrawCallId] = obj_desc.transform;
    }
    else
        assert( "draw call id is not set!" );

    mDrawCalls++;
    mMaterials += material_count;
}
} // namespace rh::rw::engine