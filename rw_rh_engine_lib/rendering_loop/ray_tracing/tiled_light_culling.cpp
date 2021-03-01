//
// Created by peter on 24.10.2020.
//

#include "tiled_light_culling.h"
#include "CameraDescription.h"
#include "RTShadowsPass.h"
#include "rendering_loop/DescriptorUpdater.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <data_desc/light_system/lighting_state.h>
#include <rendering_loop/DescriptorGenerator.h>
namespace rh::rw::engine
{
using namespace rh::engine;
struct TileData
{
    uint32_t light_count;
    uint32_t light_offset;
};
enum tile_build_slot_ids
{
    tb_Depth        = 0,
    tb_TileCfg      = 1,
    tb_Lights       = 2,
    tb_LightsIdList = 3,
    tb_TileInfo     = 4
};

TiledLightCulling::TiledLightCulling( const TiledLightCullingParams &params )
    : mCameraDesc( params.mCameraDesc ), mWidth( params.mWidth ),
      mHeight( params.mHeight )
{
    auto &device = dynamic_cast<VulkanDeviceState &>( params.mDevice );

    mBuildTilesShader = device.CreateShader(
        { .mShaderPath  = "shaders/vulkan/engine/build_tiles.comp",
          .mEntryPoint  = "main",
          .mShaderStage = ShaderStage::Compute } );

    DescriptorGenerator desc_gen{ device };
    desc_gen
        .AddDescriptor( 0, tb_Depth, 0, DescriptorType::StorageTexture, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, tb_TileCfg, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, tb_Lights, 0, DescriptorType::RWBuffer, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, tb_LightsIdList, 0, DescriptorType::RWBuffer, 1,
                        ShaderStage::Compute )
        .AddDescriptor( 0, tb_TileInfo, 0, DescriptorType::RWBuffer, 1,
                        ShaderStage::Compute );

    mBuildTilesDescSetLayout = desc_gen.FinalizeDescriptorSet( 0, 2 );
    mDescSetAlloc            = desc_gen.FinalizeAllocator();

    auto desc_sets = mDescSetAlloc->AllocateDescriptorSets(
        { { mBuildTilesDescSetLayout } } );
    mBuildTilesDescSet = desc_sets[0];

    mBuildTilesLayout = device.CreatePipelineLayout(
        { { mBuildTilesDescSetLayout, params.mCameraDesc->GetSetLayout() } } );

    // pipelines
    mBuildTilesPipeline = device.CreateComputePipeline(
        { mBuildTilesLayout,
          { ShaderStage::Compute, mBuildTilesShader, "main" } } );

    // Buffers
    uint32_t tile_size           = 8;
    uint32_t max_lights_per_tile = 32;
    //
    uint32_t max_lights = 1024;
    uint32_t tile_count =
        ( params.mWidth * params.mHeight ) / ( tile_size * tile_size );

    mTileConfigBuffer   = device.CreateBuffer( BufferCreateInfo{
        .mSize        = static_cast<uint32_t>( sizeof( TileConfig ) ),
        .mUsage       = BufferUsage::ConstantBuffer,
        .mFlags       = BufferFlags::Dynamic,
        .mInitDataPtr = nullptr } );
    mTileBuffer         = device.CreateBuffer( BufferCreateInfo{
        .mSize  = static_cast<uint32_t>( sizeof( TileData ) * tile_count ),
        .mUsage = BufferUsage::StorageBuffer,
        .mFlags = BufferFlags::Dynamic,
        .mInitDataPtr = nullptr } );
    mLightIdxListBuffer = device.CreateBuffer( BufferCreateInfo{
        .mSize        = static_cast<uint32_t>( sizeof( uint32_t ) * tile_count *
                                        max_lights_per_tile ),
        .mUsage       = BufferUsage::StorageBuffer,
        .mFlags       = BufferFlags::Dynamic,
        .mInitDataPtr = nullptr } );
    std::vector<uint32_t> idx_list( tile_count * max_lights_per_tile, 0 );
    mLightIdxListBuffer->Update( idx_list.data(), sizeof( uint32_t ) *
                                                      tile_count *
                                                      max_lights_per_tile );
    mLightBuffer = device.CreateBuffer( BufferCreateInfo{
        .mSize  = static_cast<uint32_t>( sizeof( PointLight ) * max_lights ),
        .mUsage = BufferUsage::StorageBuffer,
        .mFlags = BufferFlags::Dynamic,
        .mInitDataPtr = nullptr } );

    // tb_Depth = 0, tb_TileCfg = 1, tb_Lights = 2, tb_LightsIdList = 3,
    // tb_TileInfo = 4
    DescSetUpdateBatch descBatch{ device };
    descBatch.Begin( mBuildTilesDescSet )
        .UpdateImage( tb_Depth, DescriptorType::StorageTexture,
                      { { ImageLayout::General, params.mCurrentDepth } } )
        .UpdateBuffer(
            tb_TileCfg, DescriptorType::ROBuffer,
            { BufferUpdateInfo{ 0, sizeof( TileConfig ), mTileConfigBuffer } } )
        .UpdateBuffer( tb_Lights, DescriptorType::RWBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mLightBuffer } } )
        .UpdateBuffer(
            tb_LightsIdList, DescriptorType::RWBuffer,
            { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mLightIdxListBuffer } } )
        .UpdateBuffer( tb_TileInfo, DescriptorType::RWBuffer,
                       { BufferUpdateInfo{ 0, VK_WHOLE_SIZE, mTileBuffer } } )
        .End();
}

void TiledLightCulling::Execute( rh::engine::ICommandBuffer *dest,
                                 const AnalyticLightsState & info )
{
    auto *vk_cmd_buff = dynamic_cast<VulkanCommandBuffer *>( dest );
    if ( info.PointLights.Size() > 0 )
        mLightBuffer->Update( info.PointLights.Data(),
                              min( 1024u, info.PointLights.Size() ) *
                                  sizeof( PointLight ) );
    Config.max_light_count   = info.PointLights.Size();
    Config.max_light_in_tile = 32;
    mTileConfigBuffer->Update( &Config, sizeof( TileConfig ) );

    vk_cmd_buff->BindComputePipeline( mBuildTilesPipeline );

    vk_cmd_buff->BindDescriptorSets(
        { PipelineBindPoint::Compute,
          mBuildTilesLayout,
          0,
          { mBuildTilesDescSet, mCameraDesc->GetDescSet() } } );

    vk_cmd_buff->DispatchCompute(
        { .mX = mWidth / 8, .mY = mHeight / 8, .mZ = 1 } );
}
} // namespace rh::rw::engine