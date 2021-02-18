//
// Created by peter on 03.08.2020.
//

#include "RTShadowsPass.h"
#include "BilateralFilterPass.h"
#include "CameraDescription.h"
#include "RTSceneDescription.h"
#include "VarAwareTempAccumFilterColor.h"
#include "rendering_loop/DescriptorGenerator.h"
#include "utils.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/sampler_filter.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <array>
#include <render_driver/gpu_resources/raster_pool.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;

RTShadowsPass::RTShadowsPass( const RTShadowsInitParams &params )
    : mCamera( params.mCamera ), mScene( params.mScene ),
      mWidth( params.mWidth ), mHeight( params.mHeight )
{
    auto &              device = gRenderDriver->GetDeviceState();
    DescriptorGenerator descriptorGenerator{ device };
    /// TLAS
    descriptorGenerator.AddDescriptor(
        0, 0, 0, DescriptorType::RTAccelerationStruct, 1,
        ShaderStage::RayGen | ShaderStage::RayHit );
    /// Result buffer
    descriptorGenerator.AddDescriptor( 0, 1, 0, DescriptorType::StorageTexture,
                                       1, ShaderStage::RayGen );
    /// Normal/depth buffer; TODO: REPLACE with ROTexture for performance
    descriptorGenerator.AddDescriptor( 0, 2, 0, DescriptorType::StorageTexture,
                                       1, ShaderStage::RayGen );
    /// Default sampler
    descriptorGenerator.AddDescriptor(
        0, 3, 0, DescriptorType::Sampler, 1,
        ShaderStage::RayGen | ShaderStage::RayHit | ShaderStage::RayAnyHit );
    /// Noise texture
    descriptorGenerator.AddDescriptor( 0, 4, 0, DescriptorType::ROTexture, 1,
                                       ShaderStage::RayGen );
    /// SkyCfg
    descriptorGenerator.AddDescriptor( 0, 5, 0, DescriptorType::ROBuffer, 1,
                                       ShaderStage::RayGen );
    /// ShadowCfg
    descriptorGenerator
        .AddDescriptor( 0, 6, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::RayGen )
        // pointlights
        .AddDescriptor( 0, 7, 0, DescriptorType::RWBuffer, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, 8, 0, DescriptorType::RWBuffer, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, 9, 0, DescriptorType::RWBuffer, 1,
                        ShaderStage::RayGen );

    // Create Layouts
    mRayTraceSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 1 );

    // Desc set allocator
    mDescSetAlloc = descriptorGenerator.FinalizeAllocator();

    std::array layout_array = {
        static_cast<IDescriptorSetLayout *>( mRayTraceSetLayout ),
        mCamera->GetSetLayout(), mScene->DescLayout() };

    mPipeLayout =
        device.CreatePipelineLayout( { .mSetLayouts = layout_array } );

    std::array tex_layout_array = {
        static_cast<IDescriptorSetLayout *>( mRayTraceSetLayout ) };

    auto results = mDescSetAlloc->AllocateDescriptorSets(
        { .mLayouts = tex_layout_array } );
    mRayTraceSet = results[0];

    // setup camera stuff
    mTextureSampler =
        device.CreateSampler( { Sampler{ SamplerFilter::Linear } } );

    std::array sampler_upd_info = { ImageUpdateInfo{
        ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } };
    device.UpdateDescriptorSets( { .mSet             = mRayTraceSet,
                                   .mBinding         = 3,
                                   .mDescriptorType  = DescriptorType::Sampler,
                                   .mImageUpdateInfo = sampler_upd_info } );

    // create shaders
    mRayGenShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_shadows.rgen",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayGen } );
    mClosestHitShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_shadows.rchit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayHit } );
    mAnyHitShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_shadows.rahit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayAnyHit } );
    mMissShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/raytrace_shadow.rmiss",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayMiss } );

    // create rt buffers
    mShadowsBuffer = Create2DRenderTargetBuffer( params.mWidth, params.mHeight,
                                                 ImageBufferFormat::RGBA16 );
    mShadowsBufferView =
        device.CreateImageView( { mShadowsBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );
    mTempBlurShadowsBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );
    mTempBlurShadowsBufferView = device.CreateImageView(
        { mTempBlurShadowsBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );
    mBlurredShadowsBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );
    mBlurredShadowsBufferView = device.CreateImageView(
        { mBlurredShadowsBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );

    mParamsBuffer =
        device.CreateBuffer( { .mSize  = sizeof( ShadowParams ),
                               .mUsage = BufferUsage::ConstantBuffer } );

    auto noise       = ReadBMP( "resources/blue_noise.bmp" );
    mNoiseBuffer     = noise.mImageBuffer;
    mNoiseBufferView = noise.mImageView;
    // Bind descriptor buffers
    {
        std::array<ImageUpdateInfo, 1> img_ui = {
            { ImageLayout::General, mShadowsBufferView, nullptr } };

        device.UpdateDescriptorSets(
            { .mSet             = mRayTraceSet,
              .mBinding         = 1,
              .mDescriptorType  = DescriptorType::StorageTexture,
              .mImageUpdateInfo = img_ui } );
    }
    {
        std::array<ImageUpdateInfo, 1> img_ui = {
            { ImageLayout::General, params.mNormalsView, nullptr } };
        device.UpdateDescriptorSets(
            { .mSet             = mRayTraceSet,
              .mBinding         = 2,
              .mDescriptorType  = DescriptorType::StorageTexture,
              .mImageUpdateInfo = img_ui } );
    }
    {
        std::array<ImageUpdateInfo, 1> img_ui = {
            { ImageLayout::ShaderReadOnly, mNoiseBufferView, nullptr } };
        device.UpdateDescriptorSets(
            { .mSet             = mRayTraceSet,
              .mBinding         = 4,
              .mDescriptorType  = DescriptorType::ROTexture,
              .mImageUpdateInfo = img_ui } );
    }
    {
        device.UpdateDescriptorSets(
            { .mSet              = mRayTraceSet,
              .mBinding          = 5,
              .mDescriptorType   = DescriptorType::ROBuffer,
              .mBufferUpdateInfo = { { .mOffset = 0,
                                       .mRange  = VK_WHOLE_SIZE,
                                       .mBuffer = params.mSkyCfg } } } );
    }
    {
        device.UpdateDescriptorSets(
            { .mSet              = mRayTraceSet,
              .mBinding          = 6,
              .mDescriptorType   = DescriptorType::ROBuffer,
              .mBufferUpdateInfo = { { .mOffset = 0,
                                       .mRange  = VK_WHOLE_SIZE,
                                       .mBuffer = mParamsBuffer } } } );
    }
    {
        device.UpdateDescriptorSets(
            { .mSet              = mRayTraceSet,
              .mBinding          = 7,
              .mDescriptorType   = DescriptorType::RWBuffer,
              .mBufferUpdateInfo = { { .mOffset = 0,
                                       .mRange  = VK_WHOLE_SIZE,
                                       .mBuffer = params.mLightBuffer } } } );
    }
    {
        device.UpdateDescriptorSets(
            { .mSet              = mRayTraceSet,
              .mBinding          = 8,
              .mDescriptorType   = DescriptorType::RWBuffer,
              .mBufferUpdateInfo = { { .mOffset = 0,
                                       .mRange  = VK_WHOLE_SIZE,
                                       .mBuffer = params.mTileBuffer } } } );
    }
    {
        device.UpdateDescriptorSets(
            { .mSet              = mRayTraceSet,
              .mBinding          = 9,
              .mDescriptorType   = DescriptorType::RWBuffer,
              .mBufferUpdateInfo = {
                  { .mOffset = 0,
                    .mRange  = VK_WHOLE_SIZE,
                    .mBuffer = params.mLightIdxBuffer } } } );
    }

    /// TODO: Make it easier to create raytracing hitgroups/sbt's
    std::vector<ShaderStageDesc> stage_descs{};
    std::vector<RayTracingGroup> rt_groups{};
    stage_descs.push_back(
        ShaderStageDesc{ ShaderStage::RayGen, mRayGenShader, "main" } );
    rt_groups.push_back( RayTracingGroup{ RTShaderGroupType::General, 0u } );

    stage_descs.push_back(
        ShaderStageDesc{ ShaderStage::RayMiss, mMissShader, "main" } );
    rt_groups.push_back( RayTracingGroup{ RTShaderGroupType::General, 1u } );

    stage_descs.push_back(
        ShaderStageDesc{ ShaderStage::RayAnyHit, mAnyHitShader, "main" } );
    stage_descs.push_back(
        ShaderStageDesc{ ShaderStage::RayHit, mClosestHitShader, "main" } );
    rt_groups.push_back( RayTracingGroup{
        RTShaderGroupType::TriangleHitGroup,
        ~0u,
        2u,
        3u,
        ~0u,
    } );

    mPipeline =
        dynamic_cast<VulkanDeviceState &>( device ).CreateRayTracingPipeline(
            { .mLayout       = mPipeLayout,
              .mShaderStages = stage_descs,
              .mShaderGroups = rt_groups } );

    auto sbt = mPipeline->GetShaderBindingTable();

    mShaderBindTable =
        device.CreateBuffer( { .mSize  = static_cast<uint32_t>( sbt.size() ),
                               .mUsage = BufferUsage::RayTracingScratch,
                               .mFlags = BufferFlags::Dynamic } );
    /// Weird stuff from nvidia tutorial, I guess they fill in some data with
    /// garbage?
    // TODO: move somewhere else
    auto *pData = reinterpret_cast<uint8_t *>( mShaderBindTable->Lock() );
    for ( uint32_t g = 0; g < rt_groups.size(); g++ )
    {
        memcpy( pData, sbt.data() + g * mPipeline->GetSBTHandleSizeUnalign(),
                mPipeline->GetSBTHandleSizeUnalign() );
        pData += mPipeline->GetSBTHandleSize();
    }
    mShaderBindTable->Unlock();

    // Filtering
    mVarianceTAFilter = params.mTAFilterPipeline->GetFilter(
        VATAColorPassParam{ .mDevice        = device,
                            .mWidth         = mWidth,
                            .mHeight        = mHeight,
                            .mInputValue    = mShadowsBufferView,
                            .mPrevDepth     = params.mPrevNormalsView,
                            .mCurrentDepth  = params.mNormalsView,
                            .mMotionVectors = params.mMotionVectorsView } );
    mBilFil0 = params.mBilFilterPipe->GetPass( BilateralFilterPassParams{
        .mInputImage        = mVarianceTAFilter->GetAccumulatedValue(),
        .mTempImage         = mTempBlurShadowsBufferView,
        .mOutputImage       = mBlurredShadowsBufferView,
        .mNormalDepthBuffer = params.mNormalsView,
        .mBlurStrength      = mVarianceTAFilter->GetBlurIntensity(),
        .mTempImageBuffer   = mTempBlurShadowsBuffer,
        .mOutputImageBuffer = mBlurredShadowsBuffer,
        .mWidth             = mWidth,
        .mHeight            = mHeight } );
}

void RTShadowsPass::Execute( void *                      tlas,
                             rh::engine::ICommandBuffer *cmd_buffer )
{
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );

    std::array<AccelStructUpdateInfo, 1> accel_ui = { { tlas } };
    gRenderDriver->GetDeviceState().UpdateDescriptorSets(
        { .mSet            = mRayTraceSet,
          .mBinding        = 0,
          .mDescriptorType = DescriptorType::RTAccelerationStruct,
          .mASUpdateInfo   = accel_ui } );

    mParams.time_stamp++;
    mParamsBuffer->Update( &mParams, sizeof( mParams ) );

    /// TRANSFORM TO GENERAL
    // Prepare image memory to transfer to
    // TODO: Add some sort of memory barrier mgr

    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::Host,
          .mDstStage            = PipelineStage::Transfer,
          .mImageMemoryBarriers = {
              GetLayoutTransformBarrier( mShadowsBuffer, ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mBlurredShadowsBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mTempBlurShadowsBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ) } } );
    // bind pipeline

    std::array desc_sets = { static_cast<IDescriptorSet *>( mRayTraceSet ),
                             mCamera->GetDescSet(), mScene->DescSet() };
    vk_cmd_buff->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::RayTracing,
          .mPipelineLayout    = mPipeLayout,
          .mDescriptorSets    = desc_sets } );

    uint32_t sbt_size = mPipeline->GetSBTHandleSize();
    vk_cmd_buff->BindRayTracingPipeline( mPipeline );

    vk_cmd_buff->DispatchRays( { mShaderBindTable, 0, mShaderBindTable,
                                 sbt_size, sbt_size, mShaderBindTable,
                                 sbt_size * 2, sbt_size, nullptr, 0, 0, mWidth,
                                 mHeight, 1 } );
    mVarianceTAFilter->Execute( vk_cmd_buff );
    mBilFil0->Execute( vk_cmd_buff );
    /*
        ImageMemoryBarrierInfo shader_ro_barrier = GetLayoutTransformBarrier(
            mShadowsBuffer, ImageLayout::General, ImageLayout::ShaderReadOnly );
        shader_ro_barrier.mDstMemoryAccess = MemoryAccessFlags::ShaderRead;

        std::array dest_image_barriers = { shader_ro_barrier };

        vk_cmd_buff->PipelineBarrier(
            { .mSrcStage            = PipelineStage::RayTracing,
              .mDstStage            = PipelineStage::PixelShader,
              .mImageMemoryBarriers = dest_image_barriers } );*/
}
rh::engine::IImageView *RTShadowsPass::GetShadowsView()
{
    return mBlurredShadowsBufferView;
}

} // namespace rh::rw::engine