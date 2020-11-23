//
// Created by peter on 26.10.2020.
//

#include "debug_pipeline.h"
#include "../DescriptorGenerator.h"
#include "DescriptorUpdater.h"
#include "utils.h"
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>

namespace rh::rw::engine
{
using namespace rh::engine;

DebugPipeline::DebugPipeline( const DebugPipelineInitParams &params )
    : mWidth( params.mWidth ), mHeight( params.mHeight )
{
    DescriptorGenerator descriptorGenerator{};

    // Main Descriptor Set layout
    descriptorGenerator.AddDescriptor( 0, 0, 0, DescriptorType::StorageTexture,
                                       1, ShaderStage::Compute );
    descriptorGenerator.AddDescriptor( 0, 1, 1, DescriptorType::RWBuffer, 1,
                                       ShaderStage::Compute );

    mDescSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 4 );
    mDescAllocator = descriptorGenerator.FinalizeAllocator();

    std::array layouts = {
        static_cast<IDescriptorSetLayout *>( mDescSetLayout ) };
    mDescSet = mDescAllocator->AllocateDescriptorSets( { layouts } )[0];

    auto &device = (VulkanDeviceState &)*DeviceGlobals::RenderHookDevice;

    mPipelineLayout = device.CreatePipelineLayout( { .mSetLayouts = layouts } );

    ShaderDesc shader_desc{ .mShaderPath =
                                "shaders/vulkan/engine/debug_view.comp",
                            .mEntryPoint  = "main",
                            .mShaderStage = ShaderStage::Compute };
    mCompositionShader = device.CreateShader( shader_desc );

    mPipeline = device.CreateComputePipeline(
        { .mLayout      = mPipelineLayout,
          .mShaderStage = { .mStage      = shader_desc.mShaderStage,
                            .mShader     = mCompositionShader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );

    mOutputBuffer =
        Create2DRenderTargetBuffer( 1920, 1080, ImageBufferFormat::RGBA16 );
    mOutputBufferView =
        device.CreateImageView( { mOutputBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );

    /// Generate descriptors
    DescSetUpdateBatch{}
        .Begin( mDescSet )
        .UpdateImage( 0, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mOutputBufferView, nullptr } } )
        .UpdateBuffer( 1, DescriptorType::RWBuffer,
                       { { 0, VK_WHOLE_SIZE, params.mTiledLightsList } } )
        .End();
}
void DebugPipeline::Execute( rh::engine::ICommandBuffer *dest )
{
    using namespace rh::engine;
    auto *vk_cmd = reinterpret_cast<VulkanCommandBuffer *>( dest );

    vk_cmd->PipelineBarrier(
        { .mSrcStage       = PipelineStage::ComputeShader,
          .mDstStage       = PipelineStage::ComputeShader,
          .mMemoryBarriers = { { MemoryAccessFlags::MemoryWrite,
                                 MemoryAccessFlags::MemoryRead } } } );
    vk_cmd->BindComputePipeline( mPipeline );

    vk_cmd->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::Compute,
          .mPipelineLayout    = mPipelineLayout,
          .mDescriptorSets = { static_cast<IDescriptorSet *>( mDescSet ) } } );

    vk_cmd->DispatchCompute( { 1920 / 8, 1080 / 8, 1 } );
}

} // namespace rh::rw::engine