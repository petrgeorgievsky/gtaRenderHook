//
// Created by peter on 25.10.2020.
//

#include "RTReflectionRaysPass.h"
#include "BilateralFilterPass.h"
#include "CameraDescription.h"
#include "DescriptorUpdater.h"
#include "VarAwareTempAccumFilter.h"
#include "VarAwareTempAccumFilterColor.h"
#include "utils.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/sampler_filter.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <render_driver/gpu_resources/raster_pool.h>
#include <rendering_loop/DescriptorGenerator.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;

RTReflectionRaysPass::RTReflectionRaysPass(
    const RTReflectionInitParams &params )
    : mCamera( params.mCamera ), mScene( params.mScene ),
      mHeight( params.mHeight ), mWidth( params.mWidth )
{

    auto &              device = gRenderDriver->GetDeviceState();
    DescriptorGenerator descriptorGenerator{};

    descriptorGenerator
        .AddDescriptor( 0, 0, 0, DescriptorType::RTAccelerationStruct, 1,
                        ShaderStage::RayGen | ShaderStage::RayHit )
        .AddDescriptor( 0, 1, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, 2, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, 3, 0, DescriptorType::Sampler, 1,
                        ShaderStage::RayGen | ShaderStage::RayHit |
                            ShaderStage::RayAnyHit )
        .AddDescriptor( 0, 4, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::RayMiss | ShaderStage::RayHit )
        .AddDescriptor( 0, 5, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, 6, 0, DescriptorType::ROTexture, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, 7, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 1, 0, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 1, 1, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute );

    // Create Layouts
    mRayTraceSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 1 );
    mBlurStrSetLayout  = descriptorGenerator.FinalizeDescriptorSet( 1, 1 );

    mPipeLayout = device.CreatePipelineLayout(
        { .mSetLayouts = { mRayTraceSetLayout, mCamera->GetSetLayout(),
                           mScene->DescLayout() } } );
    mBlurStrPipeLayout =
        device.CreatePipelineLayout( { .mSetLayouts = { mBlurStrSetLayout } } );

    mParamsBuffer =
        device.CreateBuffer( { .mSize  = sizeof( ReflParams ),
                               .mUsage = BufferUsage::ConstantBuffer } );
    // Desc set allocator
    mDescSetAlloc = descriptorGenerator.FinalizeAllocator();

    auto results = mDescSetAlloc->AllocateDescriptorSets(
        { .mLayouts = { mRayTraceSetLayout, mBlurStrSetLayout } } );
    mRayTraceSet = results[0];
    mBlurStrSet  = results[1];

    // setup camera stuff
    mTextureSampler =
        device.CreateSampler( { Sampler{ SamplerFilter::Linear } } );

    // create shaders
    mRayGenShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_reflection_rays.rgen",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayGen } );
    mClosestHitShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_reflections.rchit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayHit } );
    mAnyHitShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_reflections.rahit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayAnyHit } );
    mMissShader =
        device.CreateShader( { .mShaderPath = "shaders/vulkan/engine/sky.rmiss",
                               .mEntryPoint = "main",
                               .mShaderStage = ShaderStage::RayMiss } );
    mShadowMissShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/raytrace_refl_shadow.rmiss",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayMiss } );
    mRefRoughShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/reflection_blur_strength.comp",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::Compute } );

    // create rt buffers
    mReflectionBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );
    mReflectionBufferView =
        device.CreateImageView( { mReflectionBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );
    mTempBlurReflectionBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );
    mTempBlurReflectionBufferView = device.CreateImageView(
        { mTempBlurReflectionBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );
    mFilteredReflectionBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );
    mFilteredReflectionBufferView = device.CreateImageView(
        { mFilteredReflectionBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );

    mReflectionBlurStrBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::R16 );
    mReflectionBlurStrBufferView = device.CreateImageView(
        { mReflectionBlurStrBuffer, ImageBufferFormat::R16,
          ImageViewUsage::RWTexture } );

    mVarTAColorPass = params.mVarTAColorFilterPipe->GetFilter( {
        .mDevice        = device,
        .mWidth         = mWidth,
        .mHeight        = mHeight,
        .mInputValue    = mReflectionBufferView,
        .mPrevDepth     = params.mPrevNormalsView,
        .mCurrentDepth  = params.mNormalsView,
        .mMotionVectors = params.mMotionVectorsView,
    } );

    BilateralFilterPassParams p{};
    p.mNormalDepthBuffer = params.mNormalsView;
    p.mInputImage        = mVarTAColorPass->GetAccumulatedValue();
    p.mBlurStrength      = mVarTAColorPass->GetBlurIntensity();
    p.mOutputImage       = mFilteredReflectionBufferView;
    p.mTempImage         = mTempBlurReflectionBufferView;
    p.mOutputImageBuffer = mFilteredReflectionBuffer;
    p.mTempImageBuffer   = mTempBlurReflectionBuffer;
    p.mWidth             = mWidth;
    p.mHeight            = mHeight;
    mBilFil0             = params.mBilateralFilterPipe->GetPass( p );

    auto noise       = ReadBMP( "resources/blue_noise.bmp" );
    mNoiseBuffer     = noise.mImageBuffer;
    mNoiseBufferView = noise.mImageView;

    // Bind descriptor buffers
    DescSetUpdateBatch descSetUpdateBatch{};

    descSetUpdateBatch.Begin( mRayTraceSet )
        .UpdateImage(
            1, DescriptorType::StorageTexture,
            { { ImageLayout::General, params.mNormalsView, nullptr } } )
        .UpdateImage(
            2, DescriptorType::StorageTexture,
            { { ImageLayout::General, mReflectionBufferView, nullptr } } )
        .UpdateImage(
            3, DescriptorType::Sampler,
            { { ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } } )
        .UpdateBuffer( 4, DescriptorType::ROBuffer,
                       { { 0, VK_WHOLE_SIZE, params.mSkyCfg } } )
        .UpdateImage(
            5, DescriptorType::StorageTexture,
            { { ImageLayout::General, params.mMaterialsView, nullptr } } )
        .UpdateImage(
            6, DescriptorType::ROTexture,
            { { ImageLayout::ShaderReadOnly, mNoiseBufferView, nullptr } } )
        .UpdateBuffer(
            7, DescriptorType::ROBuffer,
            { { 0, sizeof( ReflParams ), (IBuffer *)mParamsBuffer } } )
        .End()
        // blur str
        .Begin( mBlurStrSet )
        .UpdateImage( 0, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mReflectionBlurStrBufferView,
                          nullptr } } )
        .UpdateImage(
            1, DescriptorType::StorageTexture,
            { { ImageLayout::General, params.mMaterialsView, nullptr } } )
        .End();

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
        ShaderStageDesc{ ShaderStage::RayMiss, mShadowMissShader, "main" } );
    rt_groups.push_back( RayTracingGroup{ RTShaderGroupType::General, 2u } );

    stage_descs.push_back(
        ShaderStageDesc{ ShaderStage::RayAnyHit, mAnyHitShader, "main" } );
    stage_descs.push_back(
        ShaderStageDesc{ ShaderStage::RayHit, mClosestHitShader, "main" } );
    rt_groups.push_back( RayTracingGroup{
        RTShaderGroupType::TriangleHitGroup,
        ~0u,
        3u,
        4u,
        ~0u,
    } );

    mBlurStrPipeline =
        dynamic_cast<VulkanDeviceState &>( device ).CreateComputePipeline(
            { .mLayout      = mBlurStrPipeLayout,
              .mShaderStage = { .mStage      = ShaderStage::Compute,
                                .mShader     = mRefRoughShader,
                                .mEntryPoint = "main" } } );
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
}
void RTReflectionRaysPass::Execute( void *                      tlas,
                                    rh::engine::ICommandBuffer *cmd_buffer )
{
    //
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );

    gRenderDriver->GetDeviceState().UpdateDescriptorSets(
        { .mSet            = mRayTraceSet,
          .mBinding        = 0,
          .mDescriptorType = DescriptorType::RTAccelerationStruct,
          .mASUpdateInfo   = { { tlas } } } );
    mParams.time_stamp++;
    mParamsBuffer->Update( &mParams, sizeof( mParams ) );

    /// TRANSFORM TO GENERAL
    // Prepare image memory to transfer to
    // TODO: Add some sort of memory barrier mgr

    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::Host,
          .mDstStage            = PipelineStage::RayTracing,
          .mImageMemoryBarriers = {
              GetLayoutTransformBarrier( mReflectionBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mReflectionBlurStrBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mTempBlurReflectionBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mFilteredReflectionBuffer,
                                         ImageLayout::Undefined,
                                         ImageLayout::General ) } } );
    // bind pipeline

    vk_cmd_buff->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::RayTracing,
          .mPipelineLayout    = mPipeLayout,
          .mDescriptorSets    = { mRayTraceSet, mCamera->GetDescSet(),
                               mScene->DescSet() } } );

    uint32_t sbt_size = mPipeline->GetSBTHandleSize();
    vk_cmd_buff->BindRayTracingPipeline( mPipeline );
    vk_cmd_buff->DispatchRays( { .mRayGenBuffer = mShaderBindTable,
                                 .mMissBuffer   = mShaderBindTable,
                                 .mMissOffset   = sbt_size,
                                 .mMissStride   = sbt_size,
                                 .mHitBuffer    = mShaderBindTable,
                                 .mHitOffset    = sbt_size * 3,
                                 .mHitStride    = sbt_size,
                                 .mX            = mWidth,
                                 .mY            = mHeight,
                                 .mZ            = 1 } );
    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage       = PipelineStage::RayTracing,
          .mDstStage       = PipelineStage::ComputeShader,
          .mMemoryBarriers = { { MemoryAccessFlags::MemoryWrite,
                                 MemoryAccessFlags::MemoryRead } } } );
    vk_cmd_buff->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::Compute,
          .mPipelineLayout    = mBlurStrPipeLayout,
          .mDescriptorSets    = { mBlurStrSet } } );
    vk_cmd_buff->BindComputePipeline( mBlurStrPipeline );
    vk_cmd_buff->DispatchCompute( { mWidth / 8, mHeight / 8, 1 } );

    mVarTAColorPass->Execute( vk_cmd_buff );
    mBilFil0->Execute( vk_cmd_buff );
}
} // namespace rh::rw::engine