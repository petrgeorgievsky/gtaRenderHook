//
// Created by peter on 15.05.2020.
//

#include "compute_skin_animation.h"
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDebugUtils.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <data_desc/frame_info.h>
#include <render_client/skin_instance_state_recorder.h>
#include <render_driver/frame_renderer.h>
#include <render_driver/gpu_resources/resource_mgr.h>
#include <span>

namespace rh::rw::engine
{

static const int bone_matrix_count         = 256;
constexpr int    bone_matrix_bind_idx      = 2;
constexpr int    prev_bone_matrix_bind_idx = 3;

SkinAnimationPipeline::SkinAnimationPipeline(
    const SkinAnimationPipelineCreateInfo &info )
    : Device( info.Device ), Resources( info.Resources ),
      mMaxAnims( info.AnimBufferLimit )
{
    using namespace rh::engine;
    auto &device   = (VulkanDeviceState &)Device;
    mAnimateFinish = device.CreateSyncPrimitive( SyncPrimitiveType::GPU );

#ifdef _DEBUG
    VulkanDebugUtils::SetDebugName( mAnimateFinish,
                                    std::string( "skin_animation_finish_sp" ) );
#endif

    DescriptorSetAllocatorCreateParams dsc_all_cp{};
    std::array                         dsc_pool_sizes = {
        DescriptorPoolSize{ DescriptorType::RWBuffer, 4 * mMaxAnims } };

    dsc_all_cp.mMaxSets         = mMaxAnims;
    dsc_all_cp.mDescriptorPools = dsc_pool_sizes;
    mDescAllocator = device.CreateDescriptorSetAllocator( dsc_all_cp );
    //
    // [ {type: rw_buff, count: 1, reg: 0} ]
    //

    constexpr std::array desc_set_bindings = {
        DescriptorBinding{ 0, DescriptorType::RWBuffer, 1, ShaderStage::Compute,
                           0, 0 },
        DescriptorBinding{ 1, DescriptorType::RWBuffer, 1, ShaderStage::Compute,
                           0, 0 },
        DescriptorBinding{ bone_matrix_bind_idx, DescriptorType::RWBuffer, 1,
                           ShaderStage::Compute, 0, bone_matrix_bind_idx },
        DescriptorBinding{ prev_bone_matrix_bind_idx, DescriptorType::RWBuffer,
                           1, ShaderStage::Compute, 0,
                           prev_bone_matrix_bind_idx } };

    mDescSetLayout = device.CreateDescriptorSetLayout( { desc_set_bindings } );

    std::vector<IDescriptorSetLayout *> layout_array( mMaxAnims,
                                                      mDescSetLayout );

    DescriptorSetsAllocateParams all_params{};
    all_params.mLayouts = layout_array;
    mDescSetPool        = mDescAllocator->AllocateDescriptorSets( all_params );

    mPipelineLayout = device.CreatePipelineLayout(
        { .mSetLayouts = {
              static_cast<IDescriptorSetLayout *>( mDescSetLayout ) } } );

    ShaderDesc ray_gen_desc{};
    ray_gen_desc.mShaderPath =
        "shaders/vulkan/engine/animate_skinned_mesh.comp";
    ray_gen_desc.mShaderStage = ShaderStage::Compute;
    ray_gen_desc.mEntryPoint  = "main";
    mAnimShader               = device.CreateShader( ray_gen_desc );

    ComputePipelineCreateParams create_info{};
    create_info.mShaderStage.mShader     = mAnimShader;
    create_info.mShaderStage.mEntryPoint = ray_gen_desc.mEntryPoint;
    create_info.mShaderStage.mStage      = ray_gen_desc.mShaderStage;
    create_info.mLayout                  = mPipelineLayout;
    mPipeline = device.CreateComputePipeline( create_info );

    mBoneMatrixPool.resize( mMaxAnims * 2 );

    BufferCreateInfo bone_buff_ci{};
    bone_buff_ci.mSize  = sizeof( DirectX::XMFLOAT4X3 ) * bone_matrix_count;
    bone_buff_ci.mUsage = BufferUsage::StorageBuffer;
    bone_buff_ci.mFlags = BufferFlags::Dynamic;
    for ( int idx = 0; idx < mMaxAnims; idx++ )
    {
        mBoneMatrixPool[idx] = device.CreateBuffer( bone_buff_ci );
#ifdef _DEBUG
        VulkanDebugUtils::SetDebugName( mBoneMatrixPool[idx],
                                        std::string( "bone_matrix_buffer_" ) +
                                            std::to_string( idx ) );
#endif
        std::array mtx_buffer_update = {
            BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mBoneMatrixPool[idx] } };
        DescriptorSetUpdateInfo mtx_updateInfo{};
        mtx_updateInfo.mSet              = mDescSetPool[idx];
        mtx_updateInfo.mBinding          = bone_matrix_bind_idx;
        mtx_updateInfo.mDescriptorType   = DescriptorType::RWBuffer;
        mtx_updateInfo.mBufferUpdateInfo = mtx_buffer_update;

        device.UpdateDescriptorSets( mtx_updateInfo );
    }

    for ( uint32_t idx = mMaxAnims; idx < mMaxAnims * 2; idx++ )
    {
        mBoneMatrixPool[idx] = device.CreateBuffer( bone_buff_ci );
#ifdef _DEBUG
        VulkanDebugUtils::SetDebugName( mBoneMatrixPool[idx],
                                        std::string( "bone_matrix_buffer_" ) +
                                            std::to_string( idx ) );
#endif
        std::array mtx_buffer_update = {
            BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mBoneMatrixPool[idx] } };
        DescriptorSetUpdateInfo mtx_updateInfo{};
        mtx_updateInfo.mSet              = mDescSetPool[idx - mMaxAnims];
        mtx_updateInfo.mBinding          = prev_bone_matrix_bind_idx;
        mtx_updateInfo.mDescriptorType   = DescriptorType::RWBuffer;
        mtx_updateInfo.mBufferUpdateInfo = mtx_buffer_update;

        device.UpdateDescriptorSets( mtx_updateInfo );
    }

    mCmdBuffer = (VulkanCommandBuffer *)device.CreateCommandBuffer();
#ifdef _DEBUG
    VulkanDebugUtils::SetDebugName(
        mCmdBuffer, std::string( "skin_animation_cmd_buffer" ) );
#endif
}

std::vector<AnimatedMeshDrawCall> SkinAnimationPipeline::AnimateSkinnedMeshes(
    const rh::engine::ArrayProxy<SkinDrawCallInfo> &draw_calls )
{
    using namespace rh::engine;
    //

    auto &dev_state      = dynamic_cast<VulkanDeviceState &>( Device );
    auto &skin_mesh_pool = Resources.GetSkinMeshPool();

    std::vector<AnimatedMeshDrawCall> result_drawcalls{};
    result_drawcalls.reserve( draw_calls.Size() );

    // update buffers
    struct AnimDispatch
    {
        IDescriptorSet *DescSet;
        uint32_t        ThreadCount;
    };

    std::vector<AnimDispatch> dispatch_list;
    dispatch_list.reserve( draw_calls.Size() );
    uint64_t idx = 0;
    for ( auto &dc : draw_calls )
    {
        const auto &mesh_info = skin_mesh_pool.GetResource( dc.MeshId );

        // Create temporary buffers
        BufferCreateInfo vb_info{
            .mSize =
                static_cast<uint32_t>( mesh_info.mVertexCount *
                                       sizeof( VertexDescPosColorUVNormals ) ),
            .mUsage = BufferUsage::VertexBuffer | BufferUsage::StorageBuffer };

        AnimatedMeshDrawCall anim_dc{};
        anim_dc.mInstanceId        = dc.DrawCallId;
        anim_dc.mMaterialListStart = dc.MaterialListStart;
        anim_dc.mMaterialListCount = dc.MaterialListCount;
        anim_dc.mData.mVertexCount = mesh_info.mVertexCount;
        anim_dc.mData.mIndexCount  = mesh_info.mIndexCount;
        anim_dc.mData.mIndexBuffer = mesh_info.mIndexBuffer;
        anim_dc.mData.mVertexBuffer =
            new RefCountedBuffer( dev_state.CreateBuffer( vb_info ) );
        anim_dc.mTransform = dc.WorldTransform;
        result_drawcalls.push_back( anim_dc );

        // update buffers

        mBoneMatrixPool[idx]->Update( &dc.BoneTransform->f[0],
                                      sizeof( DirectX::XMFLOAT4X3 ) * 256 );

        if ( auto it = mAnimationCache.find( anim_dc.mInstanceId );
             it != mAnimationCache.end() )
        {
            mBoneMatrixPool[idx + mMaxAnims]->Update(
                it->second.data(), sizeof( DirectX::XMFLOAT4X3 ) * 256 );
        }
        else
        {
            mBoneMatrixPool[idx + mMaxAnims]->Update(
                &dc.BoneTransform->f[0], sizeof( DirectX::XMFLOAT4X3 ) * 256 );
        }
        std::copy( std::begin( dc.BoneTransform ), std::end( dc.BoneTransform ),
                   std::begin( mAnimationCache[anim_dc.mInstanceId] ) );

        auto desc_set = mDescSetPool[idx];
        {
            std::array in_buffer_update  = { BufferUpdateInfo{
                0, VK_WHOLE_SIZE, mesh_info.mVertexBuffer->Get() } };
            std::array out_buffer_update = { BufferUpdateInfo{
                0, VK_WHOLE_SIZE, anim_dc.mData.mVertexBuffer->Get() } };

            DescriptorSetUpdateInfo in_updateInfo{};
            in_updateInfo.mBufferUpdateInfo = in_buffer_update;
            in_updateInfo.mBinding          = 0;
            in_updateInfo.mSet              = desc_set;
            in_updateInfo.mDescriptorType   = DescriptorType::RWBuffer;

            DescriptorSetUpdateInfo out_updateInfo{};
            out_updateInfo.mBufferUpdateInfo = out_buffer_update;
            out_updateInfo.mBinding          = 1;
            out_updateInfo.mSet              = desc_set;
            out_updateInfo.mDescriptorType   = DescriptorType::RWBuffer;

            // TODO: Merge via ArrayProxy if possible
            dev_state.UpdateDescriptorSets( in_updateInfo );
            dev_state.UpdateDescriptorSets( out_updateInfo );
        }

        idx++;

        dispatch_list.push_back(
            { desc_set, static_cast<uint32_t>(
                            ( mesh_info.mVertexCount + 256 - 1 ) / 256u ) } );
    }

    mCmdBuffer->BeginRecord();

    mCmdBuffer->BindComputePipeline( mPipeline );

    /// Animate meshes
    for ( auto [desc_set, thread_count] : dispatch_list )
    {
        // bind descriptors
        std::array desc_sets = { desc_set };

        DescriptorSetBindInfo bindInfo{};
        bindInfo.mPipelineBindPoint = PipelineBindPoint::Compute;
        bindInfo.mDescriptorSets    = desc_sets;
        bindInfo.mPipelineLayout    = mPipelineLayout;
        mCmdBuffer->BindDescriptorSets( bindInfo );

        mCmdBuffer->DispatchCompute( { thread_count, 1, 1 } );
    }

    mCmdBuffer->EndRecord();

    /// Flush Cmd buffer
    // dev_state->ExecuteCommandBuffer( mCmdBuffer, nullptr, mAnimateFinish );

    return result_drawcalls;
}
rh::engine::CommandBufferSubmitInfo SkinAnimationPipeline::GetAnimateSubmitInfo(
    rh::engine::ISyncPrimitive *dependency )
{
    return rh::engine::CommandBufferSubmitInfo{
        mCmdBuffer,
        dependency ? std::vector{ dependency }
                   : std::vector<rh::engine::ISyncPrimitive *>{},
        mAnimateFinish };
}

SkinAnimationPipeline::~SkinAnimationPipeline()
{
    for ( auto buffer : mBoneMatrixPool )
        delete buffer;
    for ( auto dset : mDescSetPool )
        delete dset;
}

void SkinAnimationPipeline::Update( const FrameState &state )
{
    using namespace rh::engine;

    auto &mesh_pool = Resources.GetMeshPool();
    for ( const auto &item : DrawCallList )
        mesh_pool.FreeResource( item.MeshId );
    DrawCallList.clear();

    if ( state.SkinInstances.DrawCalls.Size() <= 0 )
        return;

    // Generate Skin Meshes
    auto animated_meshes =
        AnimateSkinnedMeshes( state.SkinInstances.DrawCalls );
    if ( animated_meshes.empty() )
        return;

    // Generate dynamic geometry transforms
    for ( const auto &dc : animated_meshes )
    {
        BackendMeshData backendMeshData{};
        backendMeshData.mIndexBuffer  = dc.mData.mIndexBuffer;
        backendMeshData.mVertexBuffer = dc.mData.mVertexBuffer;
        backendMeshData.mVertexCount  = dc.mData.mVertexCount;
        backendMeshData.mIndexCount   = dc.mData.mIndexCount;
        backendMeshData.mIndexBuffer->AddRef();

        DrawCallInfo sdc{};
        sdc.DrawCallId = dc.mInstanceId;
        sdc.MeshId = mesh_pool.RequestResource( std::move( backendMeshData ) );
        sdc.WorldTransform    = dc.mTransform;
        sdc.MaterialListStart = dc.mMaterialListStart;
        sdc.MaterialListCount = dc.mMaterialListCount;

        DrawCallList.push_back( sdc );
    }
}
} // namespace rh::rw::engine