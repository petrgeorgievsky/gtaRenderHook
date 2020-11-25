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
class IImageView;
} // namespace engine
namespace rw::engine
{
struct BackendMeshData;

class GPUModelBuffersPool
{
  public:
    GPUModelBuffersPool( rh::engine::IDescriptorSet *desc_set,
                         uint64_t buffer_count, uint32_t index_buffer_binding,
                         uint32_t vertex_buffer_binding );

    void    StoreModel( const BackendMeshData &model, uint64_t model_id );
    int32_t GetModelId( uint64_t model_id );

    void RemoveModel( uint64_t id )
    {
        auto slot_id = GetModelId( id );
        if ( slot_id < 0 )
            return;
        mSlotAvailability[slot_id] = 1;
        mBuffersRemap[id]          = -1;
    }

  private:
    std::vector<char>           mSlotAvailability;
    std::vector<int32_t>        mBuffersRemap;
    rh::engine::IDescriptorSet *mGPUPool;
    uint32_t                    mIndexBufferBinding;
    uint32_t                    mVertexBufferBinding;
};
} // namespace rw::engine
} // namespace rh
