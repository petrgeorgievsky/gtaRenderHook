//
// Created by peter on 15.05.2020.
//

#pragma once

#include <vector>
namespace rh
{
namespace engine
{
class IDescriptorSet;
class IDeviceState;
class IImageView;
} // namespace engine
namespace rw::engine
{
struct BackendMeshData;

struct GPUModelBuffersPoolCreateInfo
{
    // dependencies
    rh::engine::IDeviceState &Device;

    // args
    rh::engine::IDescriptorSet *DescSet;
    uint64_t                    BufferCount;
    uint32_t                    IndexBufferBinding;
    uint32_t                    VertexBufferBinding;
};

class GPUModelBuffersPool
{
  public:
    GPUModelBuffersPool( const GPUModelBuffersPoolCreateInfo &info );

    void    StoreModel( const BackendMeshData &model, uint64_t model_id );
    int32_t GetModelId( uint64_t model_id );

    void RemoveModel( uint64_t id );

  private:
    rh::engine::IDeviceState &  Device;
    std::vector<char>           mSlotAvailability;
    std::vector<int32_t>        mBuffersRemap;
    rh::engine::IDescriptorSet *mGPUPool;
    uint32_t                    mIndexBufferBinding;
    uint32_t                    mVertexBufferBinding;
};
} // namespace rw::engine
} // namespace rh
