//
// Created by peter on 27.06.2020.
//

#include "RTAOPass.h"
#include "BilateralFilterPass.h"
#include "CameraDescription.h"
#include "DescriptorUpdater.h"
#include "RTSceneDescription.h"
#include "VarAwareTempAccumFilter.h"
#include "rendering_loop/DescriptorGenerator.h"
#include "utils.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <render_driver/gpu_resources/raster_pool.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;

enum rtao_slot_ids
{
    rtaoRTAS           = 0,
    rtaoResult         = 1,
    rtaoNormalDepth    = 2,
    rtaoDefaultSampler = 3,
    rtaoNoiseTexture   = 4,
    rtaoMotionVectors  = 5,
    rtaoParamsBuffer   = 6,
};

RTAOPass::RTAOPass( const RTAOInitParams &params )
    : Device( params.Device ), mScene( params.mScene ),
      mCamera( params.mCamera ), mWidth( params.mWidth ),
      mHeight( params.mHeight )
{
    DescriptorGenerator descriptorGenerator{ Device };
    descriptorGenerator
        .AddDescriptor( 0, rtaoRTAS, 0, DescriptorType::RTAccelerationStruct, 1,
                        ShaderStage::RayGen | ShaderStage::RayHit )
        .AddDescriptor( 0, rtaoResult, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, rtaoNormalDepth, 0, DescriptorType::StorageTexture,
                        1, ShaderStage::RayGen )
        .AddDescriptor( 0, rtaoDefaultSampler, 0, DescriptorType::Sampler, 1,
                        ShaderStage::RayGen | ShaderStage::RayHit |
                            ShaderStage::RayAnyHit )
        .AddDescriptor( 0, rtaoNoiseTexture, 0, DescriptorType::ROTexture, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, rtaoMotionVectors, 0, DescriptorType::StorageTexture,
                        1, ShaderStage::RayGen )
        .AddDescriptor( 0, rtaoParamsBuffer, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::RayGen );
    // Create Layouts
    mRayTraceSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 2 );

    std::array layout_array = {
        static_cast<IDescriptorSetLayout *>( mRayTraceSetLayout ),
        mCamera->GetSetLayout(), mScene->DescLayout() };

    mPipeLayout =
        Device.CreatePipelineLayout( { .mSetLayouts = layout_array } );

    // Desc set allocator
    mDescSetAlloc = descriptorGenerator.FinalizeAllocator();

    std::array tex_layout_array = {
        static_cast<IDescriptorSetLayout *>( mRayTraceSetLayout ) };

    auto results = mDescSetAlloc->AllocateDescriptorSets(
        { .mLayouts = tex_layout_array } );
    mRayTraceSet = results[0];

    // setup camera stuff
    mTextureSampler = Device.CreateSampler( {} );

    // create shaders
    mRayGenShader = Device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_ao.rgen",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayGen } );
    mClosestHitShader = Device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_shadows.rchit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayHit } );
    mAnyHitShader = Device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_shadows.rahit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayAnyHit } );
    mMissShader = Device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/raytrace_shadow.rmiss",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayMiss } );

    mParamsBuffer =
        Device.CreateBuffer( { .mSize  = sizeof( AOParams ),
                               .mUsage = BufferUsage::ConstantBuffer } );

    // create rt buffers
    for ( auto i = 0; i < 2; i++ )
    {
        mAOBuffer[i] = Create2DRenderTargetBuffer(
            params.mWidth, params.mHeight, ImageBufferFormat::RG16 );
        mAOBufferView[i] =
            Device.CreateImageView( { mAOBuffer[i], ImageBufferFormat::RG16,
                                      ImageViewUsage::RWTexture } );
    }

    mTempBlurAOBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::R16 );
    mTempBlurAOBufferView =
        Device.CreateImageView( { mTempBlurAOBuffer, ImageBufferFormat::R16,
                                  ImageViewUsage::RWTexture } );
    mBlurredAOBuffer = Create2DRenderTargetBuffer(
        params.mWidth, params.mHeight, ImageBufferFormat::R16 );
    mBlurredAOBufferView =
        Device.CreateImageView( { mBlurredAOBuffer, ImageBufferFormat::R16,
                                  ImageViewUsage::RWTexture } );

    auto noise       = ReadBMP( "resources/blue_noise.bmp" );
    mNoiseBuffer     = noise.mImageBuffer;
    mNoiseBufferView = noise.mImageView;
    // Bind descriptor buffers

    DescSetUpdateBatch setUpdateBatch{ Device };
    // clang-format off
    setUpdateBatch.Begin( mRayTraceSet )
        .UpdateImage( rtaoResult,
                                DescriptorType::StorageTexture,
                        { { ImageLayout::General, mAOBufferView[0], nullptr } } )
        .UpdateImage( rtaoNormalDepth,
                                DescriptorType::StorageTexture,
                        { { ImageLayout::General, params.mNormalsView, nullptr } } )
        .UpdateImage( rtaoDefaultSampler,
                                DescriptorType::Sampler,
                        { { ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } } )
        .UpdateImage( rtaoNoiseTexture,
                                DescriptorType::ROTexture,
                        { { ImageLayout::ShaderReadOnly, mNoiseBufferView, nullptr } } )
        .UpdateImage( rtaoMotionVectors,
                            DescriptorType::StorageTexture,
                { { ImageLayout::General, params.mMotionVectorsView, nullptr } } )
        .UpdateBuffer( rtaoParamsBuffer,
                                DescriptorType::ROBuffer,
                       { { 0, sizeof( AOParams ), (IBuffer *)mParamsBuffer } } )
        .End();
    // clang-format on

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
        dynamic_cast<VulkanDeviceState &>( Device ).CreateRayTracingPipeline(
            { .mLayout       = mPipeLayout,
              .mShaderStages = stage_descs,
              .mShaderGroups = rt_groups } );

    auto sbt = mPipeline->GetShaderBindingTable();

    mShaderBindTable =
        Device.CreateBuffer( { .mSize  = static_cast<uint32_t>( sbt.size() ),
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

    mVarianceTAFilter = params.mTAFilterPipeline->GetFilter(
        VATAPassParam{ .mWidth         = mWidth,
                       .mHeight        = mHeight,
                       .mInputValue    = mAOBufferView[0],
                       .mPrevDepth     = params.mPrevNormalsView,
                       .mCurrentDepth  = params.mNormalsView,
                       .mMotionVectors = params.mMotionVectorsView } );

    mBilFil0 = params.mBilateralFilterPipe->GetPass( BilateralFilterPassParams{
        .mInputImage        = mVarianceTAFilter->GetAccumulatedValue(),
        .mTempImage         = mTempBlurAOBufferView,
        .mOutputImage       = mBlurredAOBufferView,
        .mNormalDepthBuffer = params.mNormalsView,
        .mBlurStrength      = mVarianceTAFilter->GetBlurIntensity(),
        .mTempImageBuffer   = mTempBlurAOBuffer,
        .mOutputImageBuffer = mBlurredAOBuffer,
        .mWidth             = mWidth,
        .mHeight            = mHeight,
    } );
}

void RTAOPass::Execute( void *tlas, rh::engine::ICommandBuffer *cmd_buffer )
{
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );

    std::array<AccelStructUpdateInfo, 1> accel_ui = { { tlas } };
    Device.UpdateDescriptorSets(
        { .mSet            = mRayTraceSet,
          .mBinding        = 0,
          .mDescriptorType = DescriptorType::RTAccelerationStruct,
          .mASUpdateInfo   = accel_ui } );
    mParams.time_stamp++;
    mParamsBuffer->Update( &mParams, sizeof( mParams ) );

    /// TRANSFORM TO GENERAL
    // Prepare image memory to transfer to
    // TODO: Add some sort of memory barrier mgr
    std::array image_barriers = {
        GetLayoutTransformBarrier( mAOBuffer[0], ImageLayout::Undefined,
                                   ImageLayout::General ),
        GetLayoutTransformBarrier( mTempBlurAOBuffer, ImageLayout::Undefined,
                                   ImageLayout::General ),
        GetLayoutTransformBarrier( mBlurredAOBuffer, ImageLayout::Undefined,
                                   ImageLayout::General ) };

    vk_cmd_buff->PipelineBarrier( { .mSrcStage = PipelineStage::Host,
                                    .mDstStage = PipelineStage::Transfer,
                                    .mImageMemoryBarriers = image_barriers } );
    // bind pipeline

    std::array desc_sets = { static_cast<IDescriptorSet *>( mRayTraceSet ),
                             mCamera->GetDescSet(), mScene->DescSet() };
    vk_cmd_buff->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::RayTracing,
          .mPipelineLayout    = mPipeLayout,
          .mDescriptorSets    = desc_sets } );

    uint32_t sbt_size = mPipeline->GetSBTHandleSize();
    vk_cmd_buff->BindRayTracingPipeline( mPipeline );
    vk_cmd_buff->DispatchRays( { .mRayGenBuffer = mShaderBindTable,
                                 .mMissBuffer   = mShaderBindTable,
                                 .mMissOffset   = sbt_size,
                                 .mMissStride   = sbt_size,
                                 .mHitBuffer    = mShaderBindTable,
                                 .mHitOffset    = sbt_size * 2,
                                 .mHitStride    = sbt_size,
                                 .mX            = mWidth,
                                 .mY            = mHeight,
                                 .mZ            = 1 } );

    mVarianceTAFilter->Execute( vk_cmd_buff );
    mBilFil0->Execute( vk_cmd_buff );
}
rh::engine::IImageView *RTAOPass::GetAOView() { return mBlurredAOBufferView; }

} // namespace rh::rw::engine