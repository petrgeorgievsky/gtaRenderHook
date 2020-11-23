//
// Created by peter on 27.06.2020.
//

#include "RTPrimaryRaysPass.h"
#include "CameraDescription.h"
#include "DescriptorUpdater.h"
#include "rendering_loop/DescriptorGenerator.h"
#include "utils.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/sampler_filter.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;

const uint32_t rtx_resolution_w = 1920;
const uint32_t rtx_resolution_h = 1080;

struct SkyCfg
{
    float sunDir[4];
    float sunColor[4];
    float horizonColor[4];
    float skyColor[4];
};
SkyCfg gSkyCfg{};

RTPrimaryRaysPass::RTPrimaryRaysPass( RTSceneDescription *scene,
                                      CameraDescription * camera )
    : mScene( scene ), mCamera( camera )
{
    auto &              device = *DeviceGlobals::RenderHookDevice;
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
                        ShaderStage::RayMiss )
        .AddDescriptor( 0, 5, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::RayGen )
        .AddDescriptor( 0, 6, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::RayGen );

    // Create Layouts
    mRayTraceSetLayout = descriptorGenerator.FinalizeDescriptorSet( 0, 1 );

    mPipeLayout = device.CreatePipelineLayout(
        { .mSetLayouts = { mRayTraceSetLayout, mCamera->GetSetLayout(),
                           mScene->DescLayout() } } );

    // Desc set allocator
    mDescSetAlloc = descriptorGenerator.FinalizeAllocator();

    auto results = mDescSetAlloc->AllocateDescriptorSets(
        { .mLayouts = { mRayTraceSetLayout } } );
    mRayTraceSet = results[0];

    // setup camera stuff
    mTextureSampler =
        device.CreateSampler( { Sampler{ SamplerFilter::Linear } } );

    gSkyCfg             = {};
    gSkyCfg.sunDir[0]   = 1.0f;
    gSkyCfg.sunDir[1]   = -1.0f;
    gSkyCfg.sunDir[2]   = 1.0f;
    gSkyCfg.skyColor[0] = 90.0f / 255.0f;
    gSkyCfg.skyColor[1] = 205.0f / 255.0f;
    gSkyCfg.skyColor[2] = 1.0f;
    // setup skycfg stuff
    mSkyCfg = device.CreateBuffer(
        { sizeof( SkyCfg ), ConstantBuffer, Dynamic, &gSkyCfg } );

    // create shaders
    mRayGenShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/rt_prim_rays.rgen",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayGen } );
    mClosestHitShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/prim_rays.rchit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayHit } );
    mAnyHitShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/prim_rays.rahit",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::RayAnyHit } );
    mMissShader =
        device.CreateShader( { .mShaderPath = "shaders/vulkan/engine/sky.rmiss",
                               .mEntryPoint = "main",
                               .mShaderStage = ShaderStage::RayMiss } );

    // create rt buffers
    mAlbedoBuffer = Create2DRenderTargetBuffer(
        rtx_resolution_w, rtx_resolution_h, ImageBufferFormat::RGBA16 );
    mAlbedoBufferView =
        device.CreateImageView( { mAlbedoBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );
    for ( auto i = 0; i < 2; i++ )
    {
        mNormalsBuffer[i] = Create2DRenderTargetBuffer(
            rtx_resolution_w, rtx_resolution_h, ImageBufferFormat::RGBA16,
            ImageBufferUsage::Storage | ImageBufferUsage::Sampled |
                ImageBufferUsage::TransferDst | ImageBufferUsage::TransferSrc );
        mNormalsBufferView[i] = device.CreateImageView(
            { mNormalsBuffer[i], ImageBufferFormat::RGBA16,
              ImageViewUsage::RWTexture } );
    }

    mMotionBuffer = Create2DRenderTargetBuffer(
        rtx_resolution_w, rtx_resolution_h, ImageBufferFormat::RG16 );
    mMotionBufferView = device.CreateImageView(
        { mMotionBuffer, ImageBufferFormat::RG16, ImageViewUsage::RWTexture } );

    mMaterialsBuffer = Create2DRenderTargetBuffer(
        rtx_resolution_w, rtx_resolution_h, ImageBufferFormat::RGBA16 );
    mMaterialsBufferView =
        device.CreateImageView( { mMaterialsBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );

    // Bind descriptor buffers
    DescSetUpdateBatch descSetUpdateBatch{};

    descSetUpdateBatch.Begin( mRayTraceSet )
        .UpdateImage( 1, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mAlbedoBufferView, nullptr } } )
        .UpdateImage(
            2, DescriptorType::StorageTexture,
            { { ImageLayout::General, mNormalsBufferView[0], nullptr } } )
        .UpdateImage(
            3, DescriptorType::Sampler,
            { { ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } } )
        .UpdateBuffer( 4, DescriptorType::ROBuffer,
                       { { 0, sizeof( SkyCfg ), mSkyCfg } } )
        .UpdateImage( 5, DescriptorType::StorageTexture,
                      { { ImageLayout::General, mMotionBufferView, nullptr } } )
        .UpdateImage(
            6, DescriptorType::StorageTexture,
            { { ImageLayout::General, mMaterialsBufferView, nullptr } } )
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
        dynamic_cast<VulkanDeviceState &>( device ).CreateRayTracingPipeline(
            { .mLayout       = mPipeLayout,
              .mShaderStages = stage_descs,
              .mShaderGroups = rt_groups } );

    auto sbt = mPipeline->GetShaderBindingTable();

    mShaderBindTable =
        device.CreateBuffer( { .mSize  = static_cast<uint32_t>( sbt.size() ),
                               .mUsage = BufferUsage::RayTracingScratch,
                               .mFlags = BufferFlags::Immutable,
                               .mInitDataPtr = sbt.data() } );
}

void RTPrimaryRaysPass::Execute( void *tlas, ICommandBuffer *cmd_buffer,
                                 const FrameInfo &frame )
{
    mFrame                  = 1 - mFrame;
    gSkyCfg.skyColor[0]     = frame.mSkyTopColor[0];
    gSkyCfg.skyColor[1]     = frame.mSkyTopColor[1];
    gSkyCfg.skyColor[2]     = frame.mSkyTopColor[2];
    gSkyCfg.skyColor[3]     = frame.mSkyTopColor[3];
    gSkyCfg.horizonColor[0] = frame.mSkyBottomColor[0];
    gSkyCfg.horizonColor[1] = frame.mSkyBottomColor[1];
    gSkyCfg.horizonColor[2] = frame.mSkyBottomColor[2];
    gSkyCfg.horizonColor[3] = frame.mSkyBottomColor[3];
    gSkyCfg.sunDir[0]       = frame.mSunDir[0];
    gSkyCfg.sunDir[1]       = frame.mSunDir[1];
    gSkyCfg.sunDir[2]       = frame.mSunDir[2];
    gSkyCfg.sunDir[3]       = 1.0f;
    mSkyCfg->Update( &gSkyCfg, sizeof( SkyCfg ) );
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );

    DeviceGlobals::RenderHookDevice->UpdateDescriptorSets(
        { .mSet            = mRayTraceSet,
          .mBinding        = 0,
          .mDescriptorType = DescriptorType::RTAccelerationStruct,
          .mASUpdateInfo   = { { tlas } } } );

    /// TRANSFORM TO GENERAL
    // Prepare image memory to transfer to
    // TODO: Add some sort of memory barrier mgr

    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::Host,
          .mDstStage            = PipelineStage::Transfer,
          .mImageMemoryBarriers = {
              GetLayoutTransformBarrier( mAlbedoBuffer, ImageLayout::Undefined,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mNormalsBuffer[0],
                                         ImageLayout::Undefined,
                                         ImageLayout::TransferSrc ),
              GetLayoutTransformBarrier( mNormalsBuffer[1],
                                         ImageLayout::Undefined,
                                         ImageLayout::TransferDst ),
              GetLayoutTransformBarrier( mMotionBuffer, ImageLayout::Undefined,
                                         ImageLayout::General ) } } );

    // Copy to prev normals before rendering
    vk_cmd_buff->CopyImageToImage(
        { .mSrc       = mNormalsBuffer[0],
          .mDst       = mNormalsBuffer[1],
          .mSrcLayout = ImageLayout::TransferSrc,
          .mDstLayout = ImageLayout::TransferDst,
          .mRegions   = { { .mSrc =
                              {
                                  .mSubresource = { .layerCount = 1 },
                                  .mExtentW     = rtx_resolution_w,
                                  .mExtentH     = rtx_resolution_h,
                              },
                          .mDest = {
                              .mSubresource = { .layerCount = 1 },
                              .mExtentW     = rtx_resolution_w,
                              .mExtentH     = rtx_resolution_h,
                          } } } } );

    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::Transfer,
          .mDstStage            = PipelineStage::RayTracing,
          .mImageMemoryBarriers = {
              GetLayoutTransformBarrier( mNormalsBuffer[0],
                                         ImageLayout::TransferSrc,
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mNormalsBuffer[1],
                                         ImageLayout::TransferDst,
                                         ImageLayout::ShaderReadOnly ) } } );
    // bind pipeline

    vk_cmd_buff->BindDescriptorSets(
        { .mPipelineBindPoint = PipelineBindPoint::RayTracing,
          .mPipelineLayout    = mPipeLayout,
          .mDescriptorSets    = { mRayTraceSet, mCamera->GetDescSet(),
                               mScene->DescSet() } } );

    uint32_t sbt_size = 32;
    vk_cmd_buff->BindRayTracingPipeline( mPipeline );
    vk_cmd_buff->DispatchRays( { .mRayGenBuffer = mShaderBindTable,
                                 .mMissBuffer   = mShaderBindTable,
                                 .mMissOffset   = sbt_size,
                                 .mMissStride   = sbt_size,
                                 .mHitBuffer    = mShaderBindTable,
                                 .mHitOffset    = sbt_size * 2,
                                 .mHitStride    = sbt_size,
                                 .mX            = rtx_resolution_w,
                                 .mY            = rtx_resolution_h,
                                 .mZ            = 1 } );
    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::RayTracing,
          .mDstStage            = PipelineStage::RayTracing,
          .mImageMemoryBarriers = {
              { .mImage           = mNormalsBuffer[0],
                .mSrcLayout       = ImageLayout::General,
                .mDstLayout       = ImageLayout::ShaderReadOnly,
                .mSrcMemoryAccess = MemoryAccessFlags::MemoryWrite,
                .mDstMemoryAccess = MemoryAccessFlags::MemoryRead,
                .mSubresRange     = { 0, 1, 0, 1 } } } } );
    /*ImageMemoryBarrierInfo shader_ro_barrier = GetLayoutTransformBarrier(
        mNormalsBuffer, ImageLayout::General, ImageLayout::ShaderReadOnly );
    shader_ro_barrier.mDstMemoryAccess = MemoryAccessFlags::ShaderRead;

    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::RayTracing,
          .mDstStage            = PipelineStage::PixelShader,
          .mImageMemoryBarriers = { shader_ro_barrier } } );*/
}

} // namespace rh::rw::engine