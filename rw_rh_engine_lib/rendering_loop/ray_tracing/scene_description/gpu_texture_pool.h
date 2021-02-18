//
// Created by peter on 15.05.2020.
//
#pragma once
#include <vector>

namespace rh
{
namespace engine
{
class IDeviceState;
class IDescriptorSet;
class IImageView;
} // namespace engine

namespace rw::engine
{

struct GPUTexturePoolCreateInfo
{
    // dependencies
    rh::engine::IDeviceState &Device;

    // args
    rh::engine::IDescriptorSet *DescSet;
    uint64_t                    TextureCount;
    uint32_t                    TexturePoolBinding;
};

class GPUTexturePool
{
  public:
    explicit GPUTexturePool( const GPUTexturePoolCreateInfo &info );

    uint64_t StoreTexture( rh::engine::IImageView *image, uint64_t texture_id );
    void     RemoveTexture( uint64_t id );
    int32_t  GetTexId( uint64_t tex_id );

  private:
    rh::engine::IDeviceState &  Device;
    std::vector<uint8_t>        mSlotAvailability;
    std::vector<int32_t>        mBuffersRemap;
    rh::engine::IDescriptorSet *mGPUPool;
    uint32_t                    mBindingId;
};
} // namespace rw::engine

} // namespace rh
