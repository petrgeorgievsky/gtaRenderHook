//
// Created by peter on 22.10.2020.
//

#include "VarAwareTempAccumFilter.h"
#include "rendering_loop/DescriptorUpdater.h"
#include "utils.h"

#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rendering_loop/DescriptorGenerator.h>

namespace rh::rw::engine
{
using namespace rh::engine;

namespace reproject_slot_ids
{
constexpr uint32_t r_OldFrame         = 0;
constexpr uint32_t r_ReprojectedFrame = 1;
constexpr uint32_t r_OldNormalDepth   = 2;
constexpr uint32_t r_NewNormalDepth   = 3;
constexpr uint32_t r_MotionVectors    = 4;
constexpr uint32_t r_OldTSPPCache     = 5;
constexpr uint32_t r_NewTSPPCache     = 6;
}; // namespace reproject_slot_ids

namespace accum_slot_ids
{
constexpr uint32_t a_NewFrame         = 0;
constexpr uint32_t a_ReprojectedFrame = 1;
constexpr uint32_t a_ResultFrame      = 2;
constexpr uint32_t a_Params           = 3;
constexpr uint32_t a_BlurStrength     = 4;
constexpr uint32_t a_CurrentTSPP      = 5;
constexpr uint32_t a_NewTSPP          = 6;
}; // namespace accum_slot_ids

VarAwareTempAccumFilterPipe::VarAwareTempAccumFilterPipe(
    rh::engine::IDeviceState &device )
    : Device( device )
{
    auto &vk_device = (VulkanDeviceState &)Device;

    /// Shaders
    mReProjectShader = Device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/reverse_reproject_pass.comp",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::Compute } );
    mAccumulateShader = Device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/history_accum_pass.comp",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::Compute } );

    /// Descriptor layouts
    DescriptorGenerator desc_gen{ Device };
    desc_gen
        .AddDescriptor( 0, reproject_slot_ids::r_OldFrame, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, reproject_slot_ids::r_ReprojectedFrame, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, reproject_slot_ids::r_OldNormalDepth, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, reproject_slot_ids::r_NewNormalDepth, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, reproject_slot_ids::r_MotionVectors, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, reproject_slot_ids::r_OldTSPPCache, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, reproject_slot_ids::r_NewTSPPCache, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        //
        .AddDescriptor( 1, accum_slot_ids::a_NewFrame, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, accum_slot_ids::a_ReprojectedFrame, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, accum_slot_ids::a_ResultFrame, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, accum_slot_ids::a_Params, 0,
                        DescriptorType::ROBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 1, accum_slot_ids::a_BlurStrength, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, accum_slot_ids::a_CurrentTSPP, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, accum_slot_ids::a_NewTSPP, 0,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute );

    mReProjectDescSetLayout  = desc_gen.FinalizeDescriptorSet( 0, 2 );
    mAccumulateDescSetLayout = desc_gen.FinalizeDescriptorSet( 1, 2 );
    mDescSetAlloc            = desc_gen.FinalizeAllocator();

    mReProjectLayout =
        Device.CreatePipelineLayout( { { mReProjectDescSetLayout } } );
    mAccumulateLayout =
        Device.CreatePipelineLayout( { { mAccumulateDescSetLayout } } );

    // pipelines
    mReProjectPipeline = vk_device.CreateComputePipeline(
        { mReProjectLayout,
          { ShaderStage::Compute, mReProjectShader, "main" } } );

    mAccumulatePipeline = vk_device.CreateComputePipeline(
        { mAccumulateLayout,
          { ShaderStage::Compute, mAccumulateShader, "main" } } );
}

VATAFilterPass *
VarAwareTempAccumFilterPipe::GetFilter( const VATAPassParam &params )
{
    return new VATAFilterPass( this, params );
}

VATAFilterPass::VATAFilterPass( VarAwareTempAccumFilterPipe *pipeline,
                                const VATAPassParam &        params )
    : mParent( pipeline ), mWidth( params.mWidth ), mHeight( params.mHeight )
{
    auto &device = mParent->Device;

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
        device, mWidth, mHeight, ImageBufferFormat::RG16 );
    mAccumulateValueView = device.CreateImageView(
        { mAccumulateValueBuffer, ImageBufferFormat::RG16,
          ImageViewUsage::RWTexture } );

    mReProjectValueBuffer = Create2DRenderTargetBuffer(
        device, mWidth, mHeight, ImageBufferFormat::RG16 );
    mReProjectValueView = device.CreateImageView(
        { mReProjectValueBuffer, ImageBufferFormat::RG16,
          ImageViewUsage::RWTexture } );

    mBlurStrengthBuffer = Create2DRenderTargetBuffer( device, mWidth, mHeight,
                                                      ImageBufferFormat::R16 );
    mBlurStrengthValueView =
        device.CreateImageView( { mBlurStrengthBuffer, ImageBufferFormat::R16,
                                  ImageViewUsage::RWTexture } );

    /// Bind descriptors

    DescSetUpdateBatch descBatch{ device };
    descBatch.Begin( mReprojDescSet )
        .UpdateImage( reproject_slot_ids::r_OldFrame,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, mAccumulateValueView } } )
        .UpdateImage( reproject_slot_ids::r_ReprojectedFrame,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, mReProjectValueView } } )
        .UpdateImage( reproject_slot_ids::r_OldNormalDepth,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mPrevDepth } } )
        .UpdateImage( reproject_slot_ids::r_NewNormalDepth,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mCurrentDepth } } )
        .UpdateImage( reproject_slot_ids::r_MotionVectors,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mMotionVectors } } )
        .UpdateImage(
            reproject_slot_ids::r_OldTSPPCache, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[0] } } )
        .UpdateImage(
            reproject_slot_ids::r_NewTSPPCache, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[1] } } )
        .End()
        // Accumulation descset
        .Begin( mAccumDescSet )
        .UpdateImage( accum_slot_ids::a_ReprojectedFrame,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General,
                          static_cast<IImageView *>( mReProjectValueView ) } } )
        .UpdateBuffer(
            accum_slot_ids::a_Params, DescriptorType::ROBuffer,
            { BufferUpdateInfo{ 0, sizeof( TAParams ),
                                static_cast<IBuffer *>( mParamsBuffer ) } } )
        .UpdateImage( accum_slot_ids::a_NewFrame,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mInputValue } } )
        .UpdateImage( accum_slot_ids::a_ResultFrame,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, mAccumulateValueView } } )
        .UpdateImage( accum_slot_ids::a_BlurStrength,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, mBlurStrengthValueView } } )
        .UpdateImage(
            accum_slot_ids::a_CurrentTSPP, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[1] } } )
        .UpdateImage(
            accum_slot_ids::a_NewTSPP, DescriptorType::StorageTexture,
            { { ImageLayout::General, mTemporalSamplePerPixelView[0] } } )
        .End();
    ;
}
void VATAFilterPass::Execute( rh::engine::ICommandBuffer *dest )
{
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( dest );
    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::Host,
          .mDstStage            = PipelineStage::ComputeShader,
          .mImageMemoryBarriers = {
              GetLayoutTransformBarrier( mAccumulateValueBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mBlurStrengthBuffer,
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