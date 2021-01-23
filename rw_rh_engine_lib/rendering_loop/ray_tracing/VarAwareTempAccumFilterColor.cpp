//
// Created by peter on 22.10.2020.
//

#include "VarAwareTempAccumFilterColor.h"
#include "DescriptorUpdater.h"
#include "utils.h"

#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rendering_loop/DescriptorGenerator.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;

enum reproject_slot_ids
{
    r_OldMoments         = 0,
    r_ReprojectedMoments = 1,
    r_OldColor           = 2,
    r_ReprojectedColor   = 3,
    r_OldNormalDepth     = 4,
    r_NewNormalDepth     = 5,
    r_MotionVectors      = 6,
    r_OldTSPPCache       = 7,
    r_NewTSPPCache       = 8,
};

enum accum_slot_ids
{
    a_NewColor           = 0,
    a_ReprojectedColor   = 1,
    a_ResultColor        = 2,
    a_ReprojectedMoments = 3,
    a_ResultMoments      = 4,
    a_Params             = 5,
    a_BlurStrength       = 6,
    a_CurrentTSPP        = 7,
    a_NewTSPP            = 8
};

VarAwareTempAccumColorFilterPipe::VarAwareTempAccumColorFilterPipe()
{
    auto &device = (VulkanDeviceState &)gRenderDriver->GetDeviceState();

    /// Shaders
    mReProjectShader = device.CreateShader(
        { .mShaderPath =
              "shaders/vulkan/engine/reverse_reproject_pass_rgb.comp",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::Compute } );
    mAccumulateShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/history_accum_pass_rgb.comp",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::Compute } );

    /// Descriptor layouts

    DescriptorGenerator desc_gen{};
    desc_gen
        .AddDescriptor( 0, r_OldColor, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, r_ReprojectedColor, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, r_OldMoments, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, r_ReprojectedMoments, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, r_OldNormalDepth, 0, DescriptorType::StorageTexture,
                        1, ShaderStage::Compute )
        .AddDescriptor( 0, r_NewNormalDepth, 0, DescriptorType::StorageTexture,
                        1, ShaderStage::Compute )
        .AddDescriptor( 0, r_MotionVectors, 0, DescriptorType::StorageTexture,
                        1, ShaderStage::Compute )
        .AddDescriptor( 0, r_OldTSPPCache, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, r_NewTSPPCache, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        //
        .AddDescriptor( 1, a_NewColor, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, a_ReprojectedColor, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, a_ResultColor, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, a_ReprojectedMoments, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, a_ResultMoments, 0, DescriptorType::StorageTexture,
                        1, ShaderStage::Compute )
        .AddDescriptor( 1, a_Params, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, a_BlurStrength, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, a_CurrentTSPP, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, a_NewTSPP, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute );

    mReProjectDescSetLayout  = desc_gen.FinalizeDescriptorSet( 0, 2 );
    mAccumulateDescSetLayout = desc_gen.FinalizeDescriptorSet( 1, 2 );
    mDescSetAlloc            = desc_gen.FinalizeAllocator();

    mReProjectLayout =
        device.CreatePipelineLayout( { { mReProjectDescSetLayout } } );
    mAccumulateLayout =
        device.CreatePipelineLayout( { { mAccumulateDescSetLayout } } );

    // pipelines
    mReProjectPipeline = device.CreateComputePipeline(
        { mReProjectLayout,
          { ShaderStage::Compute, mReProjectShader, "main" } } );

    mAccumulatePipeline = device.CreateComputePipeline(
        { mAccumulateLayout,
          { ShaderStage::Compute, mAccumulateShader, "main" } } );
}

VATAColorFilterPass *
VarAwareTempAccumColorFilterPipe::GetFilter( const VATAColorPassParam &params )
{
    return new VATAColorFilterPass( this, params );
}

VATAColorFilterPass::VATAColorFilterPass(
    VarAwareTempAccumColorFilterPipe *pipeline,
    const VATAColorPassParam &        params )
    : mParent( pipeline ), mWidth( params.mWidth ), mHeight( params.mHeight )
{
    auto &device = params.mDevice;

    auto desc_sets = pipeline->mDescSetAlloc->AllocateDescriptorSets(
        { { pipeline->mAccumulateDescSetLayout,
            pipeline->mReProjectDescSetLayout } } );
    mAccumDescSet  = desc_sets[0];
    mReprojDescSet = desc_sets[1];

    mParamsBuffer =
        device.CreateBuffer( { .mSize        = sizeof( TAParams ),
                               .mUsage       = BufferUsage::ConstantBuffer,
                               .mFlags       = Dynamic,
                               .mInitDataPtr = &mParams } );

    for ( auto i = 0; i < 2; i++ )
    {
        mTemporalSamplePerPixel[i] = device.CreateImageBuffer(
            { ImageDimensions::d2D, ImageBufferFormat::R8Uint,
              ImageBufferUsage::Storage, mHeight, mWidth } );
        mTemporalSamplePerPixelView[i] = device.CreateImageView(
            { mTemporalSamplePerPixel[i], ImageBufferFormat::R8Uint,
              ImageViewUsage::RWTexture } );
    }

    mAccumulateValueBuffer = Create2DRenderTargetBuffer(
        mWidth, mHeight, ImageBufferFormat::RGBA16 );
    mAccumulateValueView = device.CreateImageView(
        { mAccumulateValueBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );

    mReProjectValueBuffer = Create2DRenderTargetBuffer(
        mWidth, mHeight, ImageBufferFormat::RGBA16 );
    mReProjectValueView = device.CreateImageView(
        { mReProjectValueBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );

    mAccumulateMomentsBuffer =
        Create2DRenderTargetBuffer( mWidth, mHeight, ImageBufferFormat::RG16 );
    mAccumulateMomentsView = device.CreateImageView(
        { mAccumulateMomentsBuffer, ImageBufferFormat::RG16,
          ImageViewUsage::RWTexture } );

    mReProjectMomentsBuffer =
        Create2DRenderTargetBuffer( mWidth, mHeight, ImageBufferFormat::RG16 );
    mReProjectMomentsView = device.CreateImageView(
        { mReProjectMomentsBuffer, ImageBufferFormat::RG16,
          ImageViewUsage::RWTexture } );

    mBlurStrengthBuffer =
        Create2DRenderTargetBuffer( mWidth, mHeight, ImageBufferFormat::R16 );
    mBlurStrengthValueView =
        device.CreateImageView( { mBlurStrengthBuffer, ImageBufferFormat::R16,
                                  ImageViewUsage::RWTexture } );

    /// Bind descriptors

    DescSetUpdateBatch descBatch{};
    descBatch.Begin( mReprojDescSet )
        .UpdateImage( r_OldColor, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mAccumulateValueView } } )
        .UpdateImage( r_ReprojectedColor, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mReProjectValueView } } )
        .UpdateImage( r_OldMoments, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mAccumulateMomentsView } } )
        .UpdateImage( r_ReprojectedMoments, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mReProjectMomentsView } } )
        .UpdateImage( r_OldNormalDepth, DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mPrevDepth } } )
        .UpdateImage( r_NewNormalDepth, DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mCurrentDepth } } )
        .UpdateImage( r_MotionVectors, DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mMotionVectors } } )
        .UpdateImage(
            r_OldTSPPCache, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[0] } } )
        .UpdateImage(
            r_NewTSPPCache, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[1] } } )
        .End()
        // Accumulation descset
        .Begin( mAccumDescSet )
        .UpdateImage( a_ReprojectedColor, DescriptorType::StorageTexture,
                      { { ImageLayout::General,
                          static_cast<IImageView *>( mReProjectValueView ) } } )
        .UpdateBuffer(
            a_Params, DescriptorType::ROBuffer,
            { BufferUpdateInfo{ 0, sizeof( TAParams ),
                                static_cast<IBuffer *>( mParamsBuffer ) } } )
        .UpdateImage( a_NewColor, DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mInputValue } } )
        .UpdateImage( a_ResultColor, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mAccumulateValueView } } )
        .UpdateImage( a_ReprojectedMoments, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mReProjectMomentsView } } )
        .UpdateImage( a_ResultMoments, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mAccumulateMomentsView } } )
        .UpdateImage( a_BlurStrength, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mBlurStrengthValueView } } )
        .UpdateImage(
            a_CurrentTSPP, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[1] } } )
        .UpdateImage(
            a_NewTSPP, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[0] } } )
        .End();
    ;
}
void VATAColorFilterPass::Execute( rh::engine::ICommandBuffer *dest )
{
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( dest );
    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::Host,
          .mDstStage            = PipelineStage::ComputeShader,
          .mImageMemoryBarriers = {
              GetLayoutTransformBarrier( mAccumulateMomentsBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mAccumulateValueBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mBlurStrengthBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mReProjectMomentsBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mReProjectValueBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mTemporalSamplePerPixel[0],
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mTemporalSamplePerPixel[1],
                                         ImageLayout::Undefined,
                                         ImageLayout::General ) } } );
    vk_cmd_buff->BindComputePipeline( mParent->mReProjectPipeline );
    vk_cmd_buff->BindDescriptorSets( { PipelineBindPoint::Compute,
                                       mParent->mReProjectLayout,
                                       0,
                                       { mReprojDescSet } } );

    vk_cmd_buff->DispatchCompute(
        { .mX = mWidth / 8, .mY = mHeight / 8, .mZ = 1 } );

    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage       = PipelineStage::ComputeShader,
          .mDstStage       = PipelineStage::Transfer,
          .mMemoryBarriers = { { MemoryAccessFlags::MemoryWrite,
                                 MemoryAccessFlags::MemoryRead } } } );

    vk_cmd_buff->BindComputePipeline( mParent->mAccumulatePipeline );
    vk_cmd_buff->BindDescriptorSets( { PipelineBindPoint::Compute,
                                       mParent->mAccumulateLayout,
                                       0,
                                       { mAccumDescSet } } );

    vk_cmd_buff->DispatchCompute(
        { .mX = mWidth / 8, .mY = mHeight / 8, .mZ = 1 } );
}
} // namespace rh::rw::engine