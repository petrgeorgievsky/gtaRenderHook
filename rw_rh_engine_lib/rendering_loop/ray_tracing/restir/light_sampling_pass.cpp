//
// Created by peter on 04.08.2021.
//

#include "light_sampling_pass.h"

#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rendering_loop/DescriptorGenerator.h>
#include <rendering_loop/DescriptorUpdater.h>
#include <rendering_loop/ray_tracing/CameraDescription.h>
#include <rendering_loop/ray_tracing/RTSceneDescription.h>

using namespace rh::engine;
namespace rh::rw::engine::restir
{

struct LightPopulationPassBind
{
    constexpr static auto NormalDepth   = 0;
    constexpr static auto Lights        = 1;
    constexpr static auto Params        = 2;
    constexpr static auto Reservoir     = 3;
    constexpr static auto PrevReservoir = 4;
    constexpr static auto SkyCfg        = 5;
    constexpr static auto TriLights     = 6;
    constexpr static auto MotionVectors = 7;
};

LightSamplingPass::LightSamplingPass( LightPopulationPassBase &&base )
    : LightPopulationPassBase( base )
{

    auto               &device = (VulkanDeviceState &)Device;
    DescriptorGenerator descriptor_generator{ device };

    // Main Descriptor Set layout
    descriptor_generator
        .AddDescriptor( 0, LightPopulationPassBind::NormalDepth,
                        LightPopulationPassBind::NormalDepth,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, LightPopulationPassBind::Lights,
                        LightPopulationPassBind::Lights,
                        DescriptorType::RWBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, LightPopulationPassBind::Params,
                        LightPopulationPassBind::Params,
                        DescriptorType::ROBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, LightPopulationPassBind::Reservoir,
                        LightPopulationPassBind::Reservoir,
                        DescriptorType::RWBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, LightPopulationPassBind::PrevReservoir,
                        LightPopulationPassBind::PrevReservoir,
                        DescriptorType::RWBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, LightPopulationPassBind::SkyCfg,
                        LightPopulationPassBind::SkyCfg,
                        DescriptorType::ROBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, LightPopulationPassBind::TriLights,
                        LightPopulationPassBind::TriLights,
                        DescriptorType::RWBuffer, 1, ShaderStage::Compute )
        .AddDescriptor( 0, LightPopulationPassBind::MotionVectors,
                        LightPopulationPassBind::MotionVectors,
                        DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute );

    DescSetLayout = descriptor_generator.FinalizeDescriptorSet( 0, 1 );
    DescSetAlloc  = descriptor_generator.FinalizeAllocator();

    DescSet = DescSetAlloc->AllocateDescriptorSets( { { DescSetLayout } } )[0];

    std::array layouts = { static_cast<IDescriptorSetLayout *>( DescSetLayout ),
                           Camera->GetSetLayout(), Scene->DescLayout() };
    PipeLayout = device.CreatePipelineLayout( { .mSetLayouts = layouts } );

    ShaderDesc shader_desc{
        .mShaderPath =
            "shaders/vulkan/engine/restir/reservoir_population_pass.comp",
        .mEntryPoint  = "main",
        .mShaderStage = ShaderStage::Compute };
    Shader = device.CreateShader( shader_desc );

    Pipeline = device.CreateComputePipeline(
        { .mLayout      = PipeLayout,
          .mShaderStage = { .mStage      = shader_desc.mShaderStage,
                            .mShader     = Shader,
                            .mEntryPoint = shader_desc.mEntryPoint } } );

    ReservoirBuffer     = device.CreateBuffer( BufferCreateInfo{
        .mSize  = static_cast<uint32_t>( sizeof( float ) * 4 * Width * Height ),
        .mUsage = BufferUsage::StorageBuffer,
        .mFlags = BufferFlags::DynamicGPUOnly,
        .mInitDataPtr = nullptr } );
    PrevReservoirBuffer = device.CreateBuffer( BufferCreateInfo{
        .mSize  = static_cast<uint32_t>( sizeof( float ) * 4 * Width * Height ),
        .mUsage = BufferUsage::StorageBuffer,
        .mFlags = BufferFlags::DynamicGPUOnly,
        .mInitDataPtr = nullptr } );

    ParamsBuffer =
        Device.CreateBuffer( { .mSize  = sizeof( LightPopulationPassParams ),
                               .mUsage = BufferUsage::ConstantBuffer } );

    /// Generate descriptors
    DescSetUpdateBatch{ device }
        .Begin( DescSet )
        .UpdateImage( LightPopulationPassBind::NormalDepth,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, Normals, nullptr } } )
        .UpdateImage( LightPopulationPassBind::MotionVectors,
                      DescriptorType::StorageTexture,
                      { { ImageLayout::General, MotionVectors, nullptr } } )
        .UpdateBuffer( LightPopulationPassBind::Lights,
                       DescriptorType::RWBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, Lights } } )
        .UpdateBuffer( LightPopulationPassBind::Params,
                       DescriptorType::ROBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, ParamsBuffer } } )
        .UpdateBuffer(
            LightPopulationPassBind::Reservoir, DescriptorType::RWBuffer,
            { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, ReservoirBuffer } } )
        .UpdateBuffer(
            LightPopulationPassBind::PrevReservoir, DescriptorType::RWBuffer,
            { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, PrevReservoirBuffer } } )
        .UpdateBuffer( LightPopulationPassBind::SkyCfg,
                       DescriptorType::ROBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, Sky } } )
        .UpdateBuffer( LightPopulationPassBind::TriLights,
                       DescriptorType::RWBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, TriLights } } )
        .End();
}

void LightSamplingPass::Execute( uint32_t                    light_count,
                                 rh::engine::ICommandBuffer *cmd_buffer )
{

    auto *vk_cmd_buff     = dynamic_cast<VulkanCommandBuffer *>( cmd_buffer );
    PassParams.LightCount = light_count;
    PassParams.Timestamp  = ( PassParams.Timestamp + 1 ) % 100000;
    ParamsBuffer->Update( &PassParams, sizeof( LightPopulationPassParams ) );

    vk_cmd_buff->BindComputePipeline( Pipeline );

    vk_cmd_buff->BindDescriptorSets(
        { PipelineBindPoint::Compute,
          PipeLayout,
          0,
          { DescSet, Camera->GetDescSet(), Scene->DescSet() } } );

    vk_cmd_buff->DispatchCompute(
        { .mX = Width / 8, .mY = Height / 8, .mZ = 1 } );
    PassParams.FirstTime = 0;
}

rh::engine::IBuffer *LightSamplingPass::GetResult() { return ReservoirBuffer; }

rh::engine::IBuffer *LightSamplingPass::GetPrevResult()
{
    return PrevReservoirBuffer;
}

void LightSamplingPass::UpdateUI()
{
    if ( !ImGui::CollapsingHeader( "Light sampling pass" ) )
        return;
    ImGui::DragInt( "Sample count", &PassParams.ReservoirSize, 1, 1, 1024 );
}
} // namespace rh::rw::engine::restir