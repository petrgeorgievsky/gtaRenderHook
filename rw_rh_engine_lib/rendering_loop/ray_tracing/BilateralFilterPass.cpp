//
// Created by peter on 30.06.2020.
//

#include "BilateralFilterPass.h"
#include "rendering_loop/DescriptorGenerator.h"
#include "rendering_loop/ray_tracing/DescriptorUpdater.h"
#include "utils.h"

#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>

namespace rh::rw::engine
{

BilateralFilterPass::BilateralFilterPass(
    BilateralFilterPipeline *pipeline, const BilateralFilterPassParams &params )
    : mPassParams( params ), mParent( pipeline )
{
    mTempImageBuffer   = params.mTempImageBuffer;
    mOutputImageBuffer = params.mOutputImageBuffer;
    using namespace rh::engine;

    auto desc_sets = mParent->mDescAllocator->AllocateDescriptorSets(
        { { mParent->mDynamicBlurDescSetLayout,
            mParent->mDynamicBlurDescSetLayout } } );
    mDescSetH = desc_sets[0];
    mDescSetV = desc_sets[1];

    DescSetUpdateBatch updateBatch{};
    updateBatch.Begin( mDescSetH )
        .UpdateImage( 0, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mInputImage } } )
        .UpdateImage(
            1, DescriptorType::StorageTexture,
            { { ImageLayout::General, mPassParams.mNormalDepthBuffer } } )
        .UpdateImage( 2, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mOutputImage } } )
        .UpdateImage( 3, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mBlurStrength } } )
        // Vertical Pass
        .Begin( mDescSetV )
        .UpdateImage( 0, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mTempImage } } )
        .UpdateImage(
            1, DescriptorType::StorageTexture,
            { { ImageLayout::General, mPassParams.mNormalDepthBuffer } } )
        .UpdateImage( 2, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mOutputImage } } )
        .UpdateImage( 3, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mPassParams.mBlurStrength } } );

    updateBatch.End();
}

void BilateralFilterPass::Execute( rh::engine::ICommandBuffer *dest )
{
    using namespace rh::engine;
    auto *vk_cmd = dynamic_cast<VulkanCommandBuffer *>( dest );

    auto copy_output_to_temp = [&]() {
        vk_cmd->PipelineBarrier(
            { .mSrcStage            = PipelineStage::Host,
              .mDstStage            = PipelineStage::Transfer,
              .mImageMemoryBarriers = {
                  GetLayoutTransformBarrier( mOutputImageBuffer,
                                             ImageLayout::General,
                                             ImageLayout::TransferSrc ),
                  GetLayoutTransformBarrier( mTempImageBuffer,
                                             ImageLayout::General,
                                             ImageLayout::TransferDst ) } } );

        // Copy to prev normals before rendering
        vk_cmd->CopyImageToImage(
            { .mSrc       = mOutputImageBuffer,
              .mDst       = mTempImageBuffer,
              .mSrcLayout = ImageLayout::TransferSrc,
              .mDstLayout = ImageLayout::TransferDst,
              .mRegions   = { { .mSrc =
                                  {
                                      .mSubresource = { .layerCount = 1 },
                                      .mExtentW     = 1920,
                                      .mExtentH     = 1080,
                                  },
                              .mDest = {
                                  .mSubresource = { .layerCount = 1 },
                                  .mExtentW     = 1920,
                                  .mExtentH     = 1080,
                              } } } } );

        vk_cmd->PipelineBarrier(
            { .mSrcStage            = PipelineStage::Transfer,
              .mDstStage            = PipelineStage::ComputeShader,
              .mImageMemoryBarriers = {
                  GetLayoutTransformBarrier( mOutputImageBuffer,
                                             ImageLayout::TransferSrc,
                                             ImageLayout::General ),
                  GetLayoutTransformBarrier( mTempImageBuffer,
                                             ImageLayout::TransferDst,
                                             ImageLayout::General ) } } );
    };

    vk_cmd->BindComputePipeline( mParent->mPipelineDynamicBlurH );

    vk_cmd->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::Compute,
          .mPipelineLayout    = mParent->mPipelineDynamicBlurLayout,
          .mDescriptorSets    = { mDescSetH } } );

    vk_cmd->DispatchCompute( { 1920 / 8, 1080 / 8, 1 } );

    copy_output_to_temp();

    vk_cmd->BindComputePipeline( mParent->mPipelineDynamicBlurV );

    vk_cmd->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::Compute,
          .mPipelineLayout    = mParent->mPipelineDynamicBlurLayout,
          .mDescriptorSets    = { mDescSetV } } );

    vk_cmd->DispatchCompute( { 1920 / 8, 1080 / 8, 1 } );

    for ( int i = 0; i < 2; i++ )
    {

        copy_output_to_temp();

        vk_cmd->BindComputePipeline( mParent->mPipelineDynamicBlurH );

        vk_cmd->DispatchCompute( { 1920 / 8, 1080 / 8, 1 } );

        copy_output_to_temp();

        vk_cmd->BindComputePipeline( mParent->mPipelineDynamicBlurV );

        vk_cmd->BindDescriptorSets(
            { .mPipelineBindPoint = PipelineBindPoint::Compute,
              .mPipelineLayout    = mParent->mPipelineDynamicBlurLayout,
              .mDescriptorSets    = { mDescSetV } } );

        vk_cmd->DispatchCompute( { 1920 / 8, 1080 / 8, 1 } );
    }

    // copy
}

BilateralFilterPass *
BilateralFilterPipeline::GetPass( const BilateralFilterPassParams &params )
{
    return new BilateralFilterPass( this, params );
}

BilateralFilterPipeline::BilateralFilterPipeline()
{
    using namespace rh::engine;
    DescriptorGenerator descriptorGenerator{};

    // Main Descriptor Set layout
    descriptorGenerator
        .AddDescriptor( 0, 0, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, 1, 1, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, 2, 2, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, 0, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, 1, 1, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, 2, 2, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, 3, 3, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute );

    mDescSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 16 );
    mDynamicBlurDescSetLayout =
        descriptorGenerator.FinalizeDescriptorSet( 1, 8 );
    mDescAllocator = descriptorGenerator.FinalizeAllocator();

    auto &device = (VulkanDeviceState &)*DeviceGlobals::RenderHookDevice;

    mPipelineLayout = device.CreatePipelineLayout(
        { .mSetLayouts = {
              static_cast<IDescriptorSetLayout *>( mDescSetLayout ) } } );
    mPipelineDynamicBlurLayout = device.CreatePipelineLayout(
        { .mSetLayouts = { static_cast<IDescriptorSetLayout *>(
              mDynamicBlurDescSetLayout ) } } );

    ShaderDesc shader_desc{ .mShaderPath =
                                "shaders/vulkan/engine/bilateral_filter.comp",
                            .mEntryPoint  = "main",
                            .mShaderStage = ShaderStage::Compute };
    mBilateralBlurShader = device.CreateShader( shader_desc );

    ShaderDesc dyn_shader_desc{
        .mShaderPath  = "shaders/vulkan/engine/bilateral_filter_dynamic.comp",
        .mEntryPoint  = "main",
        .mShaderStage = ShaderStage::Compute };
    mDynamicBilateralBlurShader = device.CreateShader( dyn_shader_desc );
    ShaderDesc dyn_shader_h_desc{
        .mShaderPath  = "shaders/vulkan/engine/bilateral_filter_dynamic_h.comp",
        .mEntryPoint  = "main",
        .mShaderStage = ShaderStage::Compute };
    mDynamicBilateralBlurHShader = device.CreateShader( dyn_shader_h_desc );
    ShaderDesc dyn_shader_v_desc{
        .mShaderPath  = "shaders/vulkan/engine/bilateral_filter_dynamic_v.comp",
        .mEntryPoint  = "main",
        .mShaderStage = ShaderStage::Compute };
    mDynamicBilateralBlurVShader = device.CreateShader( dyn_shader_v_desc );

    mPipeline = device.CreateComputePipeline(
        { .mLayout      = mPipelineLayout,
          .mShaderStage = { .mStage      = shader_desc.mShaderStage,
                            .mShader     = mBilateralBlurShader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );

    mPipelineDynamicBlur = device.CreateComputePipeline(
        { .mLayout      = mPipelineDynamicBlurLayout,
          .mShaderStage = { .mStage      = shader_desc.mShaderStage,
                            .mShader     = mDynamicBilateralBlurShader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );

    mPipelineDynamicBlurV = device.CreateComputePipeline(
        { .mLayout      = mPipelineDynamicBlurLayout,
          .mShaderStage = { .mStage      = dyn_shader_v_desc.mShaderStage,
                            .mShader     = mDynamicBilateralBlurVShader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );

    mPipelineDynamicBlurH = device.CreateComputePipeline(
        { .mLayout      = mPipelineDynamicBlurLayout,
          .mShaderStage = { .mStage      = dyn_shader_h_desc.mShaderStage,
                            .mShader     = mDynamicBilateralBlurHShader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );
}
} // namespace rh::rw::engine