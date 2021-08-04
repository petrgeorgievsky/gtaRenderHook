//
// Created by peter on 04.08.2021.
//

#include "spatial_reuse_pass.h"

#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rendering_loop/DescriptorGenerator.h>
#include <rendering_loop/DescriptorUpdater.h>
#include <rendering_loop/ray_tracing/CameraDescription.h>

using namespace rh::engine;
namespace rh::rw::engine::restir
{

struct SpatialReusePassBind
{
    constexpr static auto Params          = 0;
    constexpr static auto Reservoir       = 1;
    constexpr static auto ResultReservoir = 2;
    constexpr static auto NormalDepth     = 3;
    constexpr static auto Lights          = 4;
    constexpr static auto Sky             = 5;
};

SpatialReusePass::SpatialReusePass( SpatialReusePassBase &&base )
    : SpatialReusePassBase( base )
{

    auto               &device = (VulkanDeviceState &)Device;
    DescriptorGenerator descriptor_generator{ device };

    // Main Descriptor Set layout
    descriptor_generator
        .AddDescriptor( 0, SpatialReusePassBind::Params, 0,
                        DescriptorType::ROBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, SpatialReusePassBind::Reservoir, 1,
                        DescriptorType::RWBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, SpatialReusePassBind::ResultReservoir, 2,
                        DescriptorType::RWBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, SpatialReusePassBind::NormalDepth, 2,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, SpatialReusePassBind::Lights, 2,
                        DescriptorType::RWBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, SpatialReusePassBind::Sky, 2,
                        DescriptorType::ROBuffer, 1, ShaderStage::Compute );

    mDescSetLayout = descriptor_generator.FinalizeDescriptorSet( 0, 1 );
    mDescSetAlloc  = descriptor_generator.FinalizeAllocator();

    mDescSet =
        mDescSetAlloc->AllocateDescriptorSets( { { mDescSetLayout } } )[0];

    std::array layouts = {
        static_cast<IDescriptorSetLayout *>( mDescSetLayout ),
        mCamera->GetSetLayout() };
    mPipeLayout = device.CreatePipelineLayout( { .mSetLayouts = layouts } );

    ShaderDesc shader_desc{
        .mShaderPath =
            "shaders/vulkan/engine/restir/reservoir_spatial_reuse_pass.comp",
        .mEntryPoint  = "main",
        .mShaderStage = ShaderStage::Compute };
    mShader = device.CreateShader( shader_desc );

    mPipeline = device.CreateComputePipeline(
        { .mLayout      = mPipeLayout,
          .mShaderStage = { .mStage      = shader_desc.mShaderStage,
                            .mShader     = mShader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );

    mResultReservoirBuffer = device.CreateBuffer( BufferCreateInfo{
        .mSize =
            static_cast<uint32_t>( sizeof( float ) * 4 * mWidth * mHeight ),
        .mUsage       = BufferUsage::StorageBuffer,
        .mFlags       = BufferFlags::Dynamic,
        .mInitDataPtr = nullptr } );

    mParamsBuffer =
        Device.CreateBuffer( { .mSize  = sizeof( SpatialReusePassParams ),
                               .mUsage = BufferUsage::ConstantBuffer } );
    /// Generate descriptors
    DescSetUpdateBatch{ device }
        .Begin( mDescSet )
        .UpdateBuffer( SpatialReusePassBind::Params, DescriptorType::ROBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mParamsBuffer } } )
        .UpdateBuffer(
            SpatialReusePassBind::Reservoir, DescriptorType::RWBuffer,
            { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mReservoirBuffer } } )
        .UpdateBuffer(
            SpatialReusePassBind::ResultReservoir, DescriptorType::RWBuffer,
            { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mResultReservoirBuffer } } )
        .UpdateImage( SpatialReusePassBind::NormalDepth,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, mNormalsView, nullptr } } )
        .UpdateBuffer( SpatialReusePassBind::Lights, DescriptorType::RWBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mLightBuffer } } )
        .UpdateBuffer( SpatialReusePassBind::Sky, DescriptorType::ROBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mSkyCfg } } )
        .End();
}

void SpatialReusePass::Execute( uint32_t                    light_count,
                                rh::engine::ICommandBuffer *cmd_buffer )
{

    auto *vk_cmd_buff       = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );
    mPassParams.ScreenWidth = mWidth;
    mPassParams.ScreenHeight = mHeight;
    mPassParams.Timestamp    = ( mPassParams.Timestamp + 1 ) % 100000;
    mPassParams.LightsCount  = light_count;
    mParamsBuffer->Update( &mPassParams, sizeof( SpatialReusePassParams ) );

    vk_cmd_buff->BindComputePipeline( mPipeline );

    vk_cmd_buff->BindDescriptorSets( { PipelineBindPoint::Compute,
                                       mPipeLayout,
                                       0,
                                       { mDescSet, mCamera->GetDescSet() } } );

    vk_cmd_buff->DispatchCompute(
        { .mX = mWidth / 8, .mY = mHeight / 8, .mZ = 1 } );
}

rh::engine::IBuffer *SpatialReusePass::GetResult()
{
    return mResultReservoirBuffer;
}

void SpatialReusePass::UpdateUI()
{
    if ( !ImGui::CollapsingHeader( "Spatial reuse pass" ) )
        return;
    ImGui::DragFloat( "Radius", &mPassParams.SpatialRadius, 0.5f, 0.5f,
                      10000.0f );
    ImGui::DragInt( "Neighbour count", &mPassParams.NeighbourCount, 1, 1, 256 );
}
} // namespace rh::rw::engine::restir