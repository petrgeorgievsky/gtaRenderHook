//
// Created by peter on 27.06.2020.
//

#include "RTPrimaryRaysPass.h"
#include "CameraDescription.h"
#include "DescriptorUpdater.h"
#include "rendering_loop/DescriptorGenerator.h"
#include "utils.h"
#include <Engine/Common/types/sampler_filter.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <data_desc/sky_state.h>

namespace rh::rw::engine
{
using namespace rh::engine;

struct SkyCfg
{
    float sunDir[4];
    float sunColor[4];
    float horizonColor[4];
    float skyColor[4];
    float ambientColor[4];
};
SkyCfg gSkyCfg{};

RTPrimaryRaysPass::RTPrimaryRaysPass( const PrimaryRaysConfig &config )
    : Device( config.Device ), mScene( config.mScene ),
      mCamera( config.mCamera ), mWidth( config.mWidth ),
      mHeight( config.mHeight )
{
    auto &              device = Device;
    DescriptorGenerator descriptorGenerator{ device };

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
    mAlbedoBuffer = Create2DRenderTargetBuffer( mWidth, mHeight,
                                                ImageBufferFormat::RGBA16 );
    mAlbedoBufferView =
        device.CreateImageView( { mAlbedoBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );
    for ( auto i = 0; i < 2; i++ )
    {
        mNormalsBuffer[i] = Create2DRenderTargetBuffer(
            mWidth, mHeight, ImageBufferFormat::RGBA16,
            ImageBufferUsage::Storage | ImageBufferUsage::Sampled |
                ImageBufferUsage::TransferDst | ImageBufferUsage::TransferSrc );
        mNormalsBufferView[i] = device.CreateImageView(
            { mNormalsBuffer[i], ImageBufferFormat::RGBA16,
              ImageViewUsage::RWTexture } );
    }

    mMotionBuffer =
        Create2DRenderTargetBuffer( mWidth, mHeight, ImageBufferFormat::RG16 );
    mMotionBufferView = device.CreateImageView(
        { mMotionBuffer, ImageBufferFormat::RG16, ImageViewUsage::RWTexture } );

    mMaterialsBuffer = Create2DRenderTargetBuffer( mWidth, mHeight,
                                                   ImageBufferFormat::RGBA16 );
    mMaterialsBufferView =
        device.CreateImageView( { mMaterialsBuffer, ImageBufferFormat::RGBA16,
                                  ImageViewUsage::RWTexture } );

    // Bind descriptor buffers
    DescSetUpdateBatch descSetUpdateBatch{ device };

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

void RTPrimaryRaysPass::Execute( void *tlas, ICommandBuffer *cmd_buffer,
                                 const SkyState &state )
{
    mFrame                  = 1 - mFrame;
    gSkyCfg.skyColor[0]     = state.mSkyTopColor[0];
    gSkyCfg.skyColor[1]     = state.mSkyTopColor[1];
    gSkyCfg.skyColor[2]     = state.mSkyTopColor[2];
    gSkyCfg.skyColor[3]     = state.mSkyTopColor[3];
    gSkyCfg.ambientColor[0] = state.mAmbientColor[0];
    gSkyCfg.ambientColor[1] = state.mAmbientColor[1];
    gSkyCfg.ambientColor[2] = state.mAmbientColor[2];
    gSkyCfg.ambientColor[3] = state.mAmbientColor[3];
    gSkyCfg.horizonColor[0] = state.mSkyBottomColor[0];
    gSkyCfg.horizonColor[1] = state.mSkyBottomColor[1];
    gSkyCfg.horizonColor[2] = state.mSkyBottomColor[2];
    gSkyCfg.horizonColor[3] = state.mSkyBottomColor[3];
    gSkyCfg.sunDir[0]       = state.mSunDir[0];
    gSkyCfg.sunDir[1]       = state.mSunDir[1];
    gSkyCfg.sunDir[2]       = state.mSunDir[2];
    gSkyCfg.sunDir[3]       = 1.0f;
    mSkyCfg->Update( &gSkyCfg, sizeof( SkyCfg ) );
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );

    Device.UpdateDescriptorSets(
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
                                         ImageLayout::General ),
              GetLayoutTransformBarrier( mMaterialsBuffer,
                                         ImageLayout::Undefined,
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
                                  .mExtentW     = mWidth,
                                  .mExtentH     = mHeight,
                              },
                          .mDest = {
                              .mSubresource = { .layerCount = 1 },
                              .mExtentW     = mWidth,
                              .mExtentH     = mHeight,
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
                                 .mHitOffset    = sbt_size * 2,
                                 .mHitStride    = sbt_size,
                                 .mX            = mWidth,
                                 .mY            = mHeight,
                                 .mZ            = 1 } );
    vk_cmd_buff->PipelineBarrier(
        { .mSrcStage            = PipelineStage::RayTracing,
          .mDstStage            = PipelineStage::RayTracing,
          .mImageMemoryBarriers = {
              { .mImage           = mNormalsBuffer[0],
                .mSrcLayout       = ImageLayout::General,
                .mDstLayout       = ImageLayout::General,
                .mSrcMemoryAccess = MemoryAccessFlags::MemoryWrite,
                .mDstMemoryAccess = MemoryAccessFlags::MemoryRead,
                .mSubresRange     = { 0, 1, 0, 1 } } } } );
    /**/
}
void RTPrimaryRaysPass::ConvertNormalsToShaderRO(
    rh::engine::ICommandBuffer *dest )
{
    ImageMemoryBarrierInfo shader_ro_barrier = GetLayoutTransformBarrier(
        (rh::engine::IImageBuffer *)mNormalsBuffer[0], ImageLayout::General,
        ImageLayout::ShaderReadOnly );
    shader_ro_barrier.mDstMemoryAccess = MemoryAccessFlags::ShaderRead;

    dest->PipelineBarrier( { .mSrcStage = PipelineStage::ComputeShader,
                             .mDstStage = PipelineStage::PixelShader,
                             .mImageMemoryBarriers = { shader_ro_barrier } } );
}

} // namespace rh::rw::engine