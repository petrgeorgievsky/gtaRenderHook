//
// Created by peter on 04.07.2020.
//

#include "DeferredCompositionPass.h"
#include "rendering_loop/DescriptorGenerator.h"
#include "rendering_loop/DescriptorUpdater.h"
#include "rendering_loop/ray_tracing/CameraDescription.h"
#include "utils.h"

#include <Engine/Common/IDeviceState.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
#include <span>

namespace rh::rw::engine
{
DeferredCompositionPass::DeferredCompositionPass(
    const DeferredCompositionPassParams &params )
    : mPassParams( params )
{
    using namespace rh::engine;
    auto               &device = (VulkanDeviceState &)mPassParams.Device;
    DescriptorGenerator descriptorGenerator{ device };

    // Main Descriptor Set layout
    descriptorGenerator.AddDescriptor( 0, 0, 0, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 1, 1, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 2, 2, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 3, 3, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 4, 4, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 5, 5, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 6, 6, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 7, 7, DescriptorType::ROBuffer, 1,
                                       ShaderStage::Compute );

    mDescSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 4 );
    mDescAllocator = descriptorGenerator.FinalizeAllocator();

    std::array layouts = {
        static_cast<IDescriptorSetLayout *>( mDescSetLayout ),
        mPassParams.Camera->GetSetLayout() };
    mDescSet = mDescAllocator->AllocateDescriptorSets( { layouts } )[0];

    mPipelineLayout = device.CreatePipelineLayout( { .mSetLayouts = layouts } );

    ShaderDesc shader_desc{
        .mShaderPath  = "shaders/vulkan/engine/deferred_composition_pass.comp",
        .mEntryPoint  = "main",
        .mShaderStage = ShaderStage::Compute };
    mCompositionShader = device.CreateShader( shader_desc );

    mPipeline = device.CreateComputePipeline(
        { .mLayout      = mPipelineLayout,
          .mShaderStage = { .mStage      = shader_desc.mShaderStage,
                            .mShader     = mCompositionShader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );

    mOutputBuffer.Image = Create2DRenderTargetBuffer(
        device, mPassParams.mWidth, mPassParams.mHeight,
        ImageBufferFormat::RGBA16 );
    mOutputBuffer.View = device.CreateImageView(
        { mOutputBuffer.Image, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );

    /// Generate descriptors
    DescSetUpdateBatch{ device }
        .Begin( mDescSet )
        .UpdateImage(
            0, DescriptorType::StorageTexture,
            { { ImageLayout::General, mPassParams.mAlbedoBuffer, nullptr } } )
        .UpdateImage( 1, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mNormalDepthBuffer,
                          nullptr } } )
        .UpdateImage(
            2, DescriptorType::StorageTexture,
            { { ImageLayout::General, mPassParams.mAOBuffer, nullptr } } )
        .UpdateImage(
            3, DescriptorType::StorageTexture,
            { { ImageLayout::General, mPassParams.mLightingBuffer, nullptr } } )
        .UpdateImage( 4, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mReflectionBuffer,
                          nullptr } } )
        .UpdateImage( 5, DescriptorType::StorageTexture,
                      { { ImageLayout::General,
                          mPassParams.mMaterialParamsBuffer, nullptr } } )
        .UpdateImage(
            6, DescriptorType::StorageTexture,
            { { ImageLayout::General, mOutputBuffer.View, nullptr } } )
        .UpdateBuffer( 7, DescriptorType::ROBuffer,
                       { { 0, VK_WHOLE_SIZE, params.mSkyCfg } } )
        .End();
}
void DeferredCompositionPass::Execute( rh::engine::ICommandBuffer *dest )
{

    using namespace rh::engine;
    auto *vk_cmd = reinterpret_cast<VulkanCommandBuffer *>( dest );

    vk_cmd->PipelineBarrier(
        { .mSrcStage            = PipelineStage::RayTracing,
          .mDstStage            = PipelineStage::ComputeShader,
          .mImageMemoryBarriers = {
              mOutputBuffer.SetLayout( ImageLayout::General ) } } );

    vk_cmd->PipelineBarrier(
        { .mSrcStage       = PipelineStage::RayTracing,
          .mDstStage       = PipelineStage::ComputeShader,
          .mMemoryBarriers = { { MemoryAccessFlags::MemoryWrite,
                                 MemoryAccessFlags::MemoryRead } } } );
    vk_cmd->BindComputePipeline( mPipeline );

    vk_cmd->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::Compute,
          .mPipelineLayout    = mPipelineLayout,
          .mDescriptorSets    = { static_cast<IDescriptorSet *>( mDescSet ),
                               mPassParams.Camera->GetDescSet() } } );

    vk_cmd->DispatchCompute(
        { mPassParams.mWidth / 8, mPassParams.mHeight / 8, 1 } );

    ImageMemoryBarrierInfo shader_ro_barrier =
        mOutputBuffer.SetLayout( ImageLayout::ShaderReadOnly );
    shader_ro_barrier.mDstMemoryAccess = MemoryAccessFlags::ShaderRead;

    dest->PipelineBarrier( { .mSrcStage = PipelineStage::ComputeShader,
                             .mDstStage = PipelineStage::PixelShader,
                             .mImageMemoryBarriers = { shader_ro_barrier } } );
}
} // namespace rh::rw::engine