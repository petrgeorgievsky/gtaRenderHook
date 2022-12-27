//
// Created by peter on 04.08.2021.
//

#include "visibility_reuse_pass.h"

#include <Engine/Common/types/sampler_filter.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rendering_loop/DescriptorGenerator.h>
#include <rendering_loop/DescriptorUpdater.h>
#include <rendering_loop/ray_tracing/CameraDescription.h>
#include <rendering_loop/ray_tracing/RTSceneDescription.h>

namespace rh::rw::engine::restir
{
struct VisibilityReusePassBind
{
    constexpr static auto TLAS            = 0;
    constexpr static auto NormalDepth     = 1;
    constexpr static auto Lights          = 2;
    constexpr static auto Sampler         = 3;
    constexpr static auto Reservoir       = 4;
    constexpr static auto ResultReservoir = 5;
    constexpr static auto Params          = 6;
    constexpr static auto SkyCfg          = 7;
};
using namespace rh::engine;

//
VisibilityReusePass::VisibilityReusePass(
    const VisibilityReuseInitParams &params )
    : Device( params.Device ), mScene( params.Scene ), mCamera( params.Camera ),
      mWidth( params.Width ), mHeight( params.Height )
{
    DescriptorGenerator descriptor_generator{ Device };

    descriptor_generator
        .AddDescriptor( 0, VisibilityReusePassBind::TLAS, 0,
                        DescriptorType::RTAccelerationStruct, 1,
                        ShaderStage::RayGen | ShaderStage::RayHit )
        .AddDescriptor( 0, VisibilityReusePassBind::NormalDepth, 0,
                        DescriptorType::StorageTexture, 1, ShaderStage::RayGen )
        .AddDescriptor(
            0, VisibilityReusePassBind::Sampler, 0, DescriptorType::Sampler, 1,
            ShaderStage::RayGen | ShaderStage::RayHit | ShaderStage::RayAnyHit )
        .AddDescriptor( 0, VisibilityReusePassBind::Lights, 0,
                        DescriptorType::RWBuffer, 1, ShaderStage::RayGen )
        .AddDescriptor( 0, VisibilityReusePassBind::Reservoir, 0,
                        DescriptorType::RWBuffer, 1, ShaderStage::RayGen )
        .AddDescriptor( 0, VisibilityReusePassBind::ResultReservoir, 0,
                        DescriptorType::RWBuffer, 1, ShaderStage::RayGen )
        .AddDescriptor( 0, VisibilityReusePassBind::Params, 0,
                        DescriptorType::ROBuffer, 1, ShaderStage::RayGen )
        .AddDescriptor( 0, VisibilityReusePassBind::SkyCfg, 0,
                        DescriptorType::ROBuffer, 1, ShaderStage::RayGen );

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
              "shaders/vulkan/engine/restir/visibility_reuse_pass.rgen",
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
        Device.CreateBuffer( { .mSize  = sizeof( VisibilityReuseProperties ),
                               .mUsage = BufferUsage::ConstantBuffer } );

    DescSetUpdateBatch{ Device }
        .Begin( mRayTraceSet )
        .UpdateImage(
            VisibilityReusePassBind::NormalDepth,
            DescriptorType::StorageTexture,
            { { ImageLayout::General, params.NormalsView, nullptr } } )
        .UpdateImage(
            VisibilityReusePassBind::Sampler, DescriptorType::Sampler,
            { { ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } } )
        .UpdateBuffer( VisibilityReusePassBind::Reservoir,
                       DescriptorType::RWBuffer,
                       { { 0, VK_WHOLE_SIZE, params.InputReservoir } } )
        .UpdateBuffer( VisibilityReusePassBind::ResultReservoir,
                       DescriptorType::RWBuffer,
                       { { 0, VK_WHOLE_SIZE, params.OutputReservoir } } )
        .UpdateBuffer( VisibilityReusePassBind::Lights,
                       DescriptorType::RWBuffer,
                       { { 0, VK_WHOLE_SIZE, params.LightBuffer } } )
        .UpdateBuffer( VisibilityReusePassBind::Params,
                       DescriptorType::ROBuffer,
                       { { 0, VK_WHOLE_SIZE, mParamsBuffer } } )
        .UpdateBuffer( VisibilityReusePassBind::SkyCfg,
                       DescriptorType::ROBuffer,
                       { { 0, VK_WHOLE_SIZE, params.SkyCfg } } )
        .End();
    //

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
}

void VisibilityReusePass::Execute( void                       *tlas,
                                   VisibilityReuseProperties   properties,
                                   rh::engine::ICommandBuffer *cmd_buffer )
{
    mParamsBuffer->Update( &properties, sizeof( VisibilityReuseProperties ) );

    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );

    std::array<AccelStructUpdateInfo, 1> accel_ui = { { tlas } };
    Device.UpdateDescriptorSets(
        { .mSet            = mRayTraceSet,
          .mBinding        = 0,
          .mDescriptorType = DescriptorType::RTAccelerationStruct,
          .mASUpdateInfo   = accel_ui } );

    // bind pipeline

    std::array desc_sets = { static_cast<IDescriptorSet *>( mRayTraceSet ),
                             mCamera->GetDescSet(), mScene->DescSet() };
    vk_cmd_buff->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::RayTracing,
          .mPipelineLayout    = mPipeLayout,
          .mDescriptorSets    = desc_sets } );

    uint32_t sbt_size = mPipeline->GetSBTHandleSize();
    vk_cmd_buff->BindRayTracingPipeline( mPipeline );

    vk_cmd_buff->DispatchRays(
        VulkanRayDispatch{ .mRayGenBuffer   = mShaderBindTable,
                           .mRayGenOffset   = 0,
                           .mRayGenSize     = sbt_size,
                           .mMissBuffer     = mShaderBindTable,
                           .mMissOffset     = sbt_size,
                           .mMissStride     = sbt_size,
                           .mMissSize       = sbt_size,
                           .mHitBuffer      = mShaderBindTable,
                           .mHitOffset      = sbt_size * 2,
                           .mHitStride      = sbt_size,
                           .mHitSize        = sbt_size,
                           .mCallableBuffer = nullptr,
                           .mCallableOffset = 0,
                           .mCallableStride = 0,
                           .mCallableSize   = 0,
                           .mX              = mWidth,
                           .mY              = mHeight,
                           .mZ              = 1 } );
}

} // namespace rh::rw::engine::restir
