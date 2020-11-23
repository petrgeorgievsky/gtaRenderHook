//
// Created by peter on 15.05.2020.
//
#pragma once
#include <unordered_map>
#include <vector>
namespace rh
{
namespace engine
{
class IDescriptorSet;
class IImageView;
} // namespace engine
namespace rw::engine
{
class GPUTexturePool
{
  public:
    explicit GPUTexturePool( rh::engine::IDescriptorSet *desc_set,
                             uint32_t binding_id, uint64_t descriptor_count );

    uint64_t StoreTexture( rh::engine::IImageView *image, uint64_t texture_id );
    void     RemoveTexture( uint64_t id );
    int32_t  GetTexId( uint64_t tex_id );

  private:
    std::vector<uint8_t>        mSlotAvailability;
    std::vector<int32_t>        mBuffersRemap;
    rh::engine::IDescriptorSet *mGPUPool;
    uint32_t                    mBindingId;
};
} // namespace rw::engine

} // namespace rh
