//
// Created by peter on 31.07.2021.
//

#include "restir_shadow_pass.h"
#include "light_sampling_pass.h"
#include "spatial_reuse_pass.h"
#include <imgui.h>

namespace rh::rw::engine::restir
{
struct ShadowsPassBind
{
    constexpr static auto TLAS          = 0;
    constexpr static auto Result        = 1;
    constexpr static auto NormalDepth   = 2;
    constexpr static auto Sampler       = 3;
    constexpr static auto Lights        = 4;
    constexpr static auto Reservoir     = 5;
    constexpr static auto PrevReservoir = 6;
    constexpr static auto Params        = 7;
    constexpr static auto SkyCfg        = 8;
};

ShadowsPass::ShadowsPass( const ShadowsInitParams &params )
    : Device( params.Device ), mScene( params.mScene ),
      mCamera( params.mCamera ), mWidth( params.mWidth ),
      mHeight( params.mHeight )
{
    mLightPopulationPass = new LightSamplingPass( LightPopulationPassBase{
        Device, mScene, mCamera, params.mWidth, params.mHeight,
        params.mNormalsView, params.mLightBuffer, params.mSkyCfg } );
    mSpatialReusePass    = new SpatialReusePass( SpatialReusePassBase{
        Device, params.mWidth, params.mHeight,
        mLightPopulationPass->GetResult(), params.mNormalsView,
        params.mLightBuffer, params.mSkyCfg, mCamera } );

    DescriptorGenerator descriptor_generator{ Device };

    descriptor_generator
        .AddDescriptor( 0, ShadowsPassBind::TLAS, 0,
                        DescriptorType::RTAccelerationStruct, 1,
                        ShaderStage::RayGen | ShaderStage::RayHit )
        .AddDescriptor( 0, ShadowsPassBind::Result, 0,
                        DescriptorType::StorageTexture, 1, ShaderStage::RayGen )
        .AddDescriptor( 0, ShadowsPassBind::NormalDepth, 0,
                        DescriptorType::StorageTexture, 1, ShaderStage::RayGen )
        .AddDescriptor(
            0, ShadowsPassBind::Sampler, 0, DescriptorType::Sampler, 1,
            ShaderStage::RayGen | ShaderStage::RayHit | ShaderStage::RayAnyHit )
        .AddDescriptor( 0, ShadowsPassBind::Lights, 0, DescriptorType::RWBuffer,
                        1, ShaderStage::RayGen )
        .AddDescriptor( 0, ShadowsPassBind::Reservoir, 0,
                        DescriptorType::RWBuffer, 1, ShaderStage::RayGen )
        .AddDescriptor( 0, ShadowsPassBind::PrevReservoir, 0,
                        DescriptorType::RWBuffer, 1, ShaderStage::RayGen )
        .AddDescriptor( 0, ShadowsPassBind::Params, 0, DescriptorType::ROBuffer,
                        1, ShaderStage::RayGen )
        .AddDescriptor( 0, ShadowsPassBind::SkyCfg, 0, DescriptorType::ROBuffer,
                        1, ShaderStage::RayGen );

    // Create Layouts
    mRayTraceSetLayout = descriptor_generator.FinalizeDescriptorSet( 0, 1 );

    // Desc set allocator
    mDescSetAlloc = descriptor_generator.FinalizeAllocator();

    std::array layout_array = {
        static_cast<IDescriptorSetLayout *>( mRayTraceSetLayout ),
        mCamera->GetSetLayout(), mScene->DescLayout() };

    mPipeLayout =
        Device.CreatePipelineLayout( { .mSetLayouts = layout_array } );

    std::array tex_layout_array = {
        static_cast<IDescriptorSetLayout *>( mRayTraceSetLayout ) };

    auto results = mDescSetAlloc->AllocateDescriptorSets(
        { .mLayouts = tex_layout_array } );
    mRayTraceSet = results[0];

    // setup camera stuff
    mTextureSampler =
        Device.CreateSampler( { Sampler{ SamplerFilter::Linear } } );

    // create shaders
    mRayGenShader = Device.CreateShader(
        { .mShaderPath =
              "shaders/vulkan/engine/restir/reservoir_visibility_pass.rgen",
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

    // create rt buffers
    mShadowsBuffer = Create2DRenderTargetBuffer(
        Device, params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );
    mShadowsBufferView =
        Device.CreateImageView( { mShadowsBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );

    mTempBlurShadowsBuffer = Create2DRenderTargetBuffer(
        Device, params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );
    mTempBlurShadowsBufferView = Device.CreateImageView(
        { mTempBlurShadowsBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );
    mBlurredShadowsBuffer = Create2DRenderTargetBuffer(
        Device, params.mWidth, params.mHeight, ImageBufferFormat::RGBA16 );

    mBlurredShadowsBufferView = Device.CreateImageView(
        { mBlurredShadowsBuffer, ImageBufferFormat::RGBA16,
          ImageViewUsage::RWTexture } );

    mParamsBuffer =
        Device.CreateBuffer( { .mSize  = sizeof( ShadowsProperties ),
                               .mUsage = BufferUsage::ConstantBuffer } );

    DescSetUpdateBatch{ Device }
        .Begin( mRayTraceSet )
        .UpdateImage(
            ShadowsPassBind::Result, DescriptorType::StorageTexture,
            { { ImageLayout::General, mShadowsBufferView, nullptr } } )
        .UpdateImage(
            ShadowsPassBind::NormalDepth, DescriptorType::StorageTexture,
            { { ImageLayout::General, params.mNormalsView, nullptr } } )
        .UpdateImage(
            ShadowsPassBind::Sampler, DescriptorType::Sampler,
            { { ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } } )
        .UpdateBuffer(
            ShadowsPassBind::Reservoir, DescriptorType::RWBuffer,
            { { 0, VK_WHOLE_SIZE, mSpatialReusePass->GetResult() } } )
        .UpdateBuffer(
            ShadowsPassBind::PrevReservoir, DescriptorType::RWBuffer,
            { { 0, VK_WHOLE_SIZE, mLightPopulationPass->GetPrevResult() } } )
        .UpdateBuffer( ShadowsPassBind::Lights, DescriptorType::RWBuffer,
                       { { 0, VK_WHOLE_SIZE, params.mLightBuffer } } )
        .UpdateBuffer( ShadowsPassBind::Params, DescriptorType::ROBuffer,
                       { { 0, VK_WHOLE_SIZE, mParamsBuffer } } )
        .UpdateBuffer( ShadowsPassBind::SkyCfg, DescriptorType::ROBuffer,
                       { { 0, VK_WHOLE_SIZE, params.mSkyCfg } } )
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
    auto *sbt_data = reinterpret_cast<uint8_t *>( mShaderBindTable->Lock() );
    for ( uint32_t g = 0; g < rt_groups.size(); g++ )
    {
        memcpy( sbt_data, sbt.data() + g * mPipeline->GetSBTHandleSizeUnalign(),
                mPipeline->GetSBTHandleSizeUnalign() );
        sbt_data += mPipeline->GetSBTHandleSize();
    }
    mShaderBindTable->Unlock();

    // Filtering
    mVarianceTAFilter = params.mTAFilterPipeline->GetFilter(
        VATAColorPassParam{ .mWidth         = mWidth,
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

void ShadowsPass::Execute( void *tlas, uint32_t light_count,
                           rh::engine::ICommandBuffer *cmd_buffer )
{
    mLightPopulationPass->Execute( light_count, cmd_buffer );
    mSpatialReusePass->Execute( light_count, cmd_buffer );
    mPassParams.Timestamp   = ( mPassParams.Timestamp + 1 ) % 100000;
    mPassParams.LightsCount = light_count;
    mParamsBuffer->Update( &mPassParams, sizeof( ShadowsProperties ) );

    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );

    std::array<AccelStructUpdateInfo, 1> accel_ui = { { tlas } };
    Device.UpdateDescriptorSets(
        { .mSet            = mRayTraceSet,
          .mBinding        = 0,
          .mDescriptorType = DescriptorType::RTAccelerationStruct,
          .mASUpdateInfo   = accel_ui } );

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
}

rh::engine::IImageView *ShadowsPass::GetShadowsView()
{
    return mBlurredShadowsBufferView;
}

void ShadowsPass::UpdateUI()
{
    if ( !ImGui::CollapsingHeader( "ReSTIR Parameters" ) )
        return;
    mSpatialReusePass->UpdateUI();
    mLightPopulationPass->UpdateUI();

    if ( !ImGui::CollapsingHeader( "Visibility pass" ) )
        return;
    ImGui::DragFloat( "Light radius", &mPassParams.LightRadius, 0.1f, 0.0f,
                      10.0f );
    ImGui::DragFloat( "Light intensity boost", &mPassParams.LightIntensityBoost,
                      0.1f, 0.1f, 100.0f );
}

} // namespace rh::rw::engine::restir
