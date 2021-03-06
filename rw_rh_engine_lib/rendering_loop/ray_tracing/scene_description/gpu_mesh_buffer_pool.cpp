//
// Created by peter on 15.05.2020.
//

#include "gpu_mesh_buffer_pool.h"
#include <Engine/Common/IDeviceState.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>

namespace rh::rw::engine
{

GPUModelBuffersPool::GPUModelBuffersPool(
    const GPUModelBuffersPoolCreateInfo &info )
    : Device( info.Device ), mGPUPool( info.DescSet ),
      mIndexBufferBinding( info.IndexBufferBinding ),
      mVertexBufferBinding( info.VertexBufferBinding )
{
    mSlotAvailability.resize( info.BufferCount, 1 );
    mBuffersRemap.resize( info.BufferCount, -1 );
}

int32_t GPUModelBuffersPool::GetModelId( uint64_t model_id )
{
    return mBuffersRemap[model_id];
}

void GPUModelBuffersPool::StoreModel( const BackendMeshData &model,
                                      uint64_t               model_id )
{
    using namespace rh::engine;
    uint64_t id = 0;

    // TODO: Can be improved and optimized, can be iterated from last free
    // resource
    while ( id < mSlotAvailability.size() && mSlotAvailability[id] == 0 )
        id++;

    {
        std::array<BufferUpdateInfo, 1> vb_list{
            { 0, VK_WHOLE_SIZE, model.mVertexBuffer->Get() } };

        DescriptorSetUpdateInfo vbUpdateInfo{};
        vbUpdateInfo.mBinding          = mVertexBufferBinding;
        vbUpdateInfo.mSet              = mGPUPool;
        vbUpdateInfo.mDescriptorType   = DescriptorType::RWBuffer;
        vbUpdateInfo.mArrayStartIdx    = id;
        vbUpdateInfo.mBufferUpdateInfo = vb_list;
        Device.UpdateDescriptorSets( vbUpdateInfo );
    }
    {
        std::array<BufferUpdateInfo, 1> ib_list{
            { 0, VK_WHOLE_SIZE, model.mIndexBuffer->Get() } };
        DescriptorSetUpdateInfo ibUpdateInfo{};
        ibUpdateInfo.mBinding          = mIndexBufferBinding;
        ibUpdateInfo.mSet              = mGPUPool;
        ibUpdateInfo.mDescriptorType   = DescriptorType::RWBuffer;
        ibUpdateInfo.mArrayStartIdx    = id;
        ibUpdateInfo.mBufferUpdateInfo = ib_list;
        Device.UpdateDescriptorSets( ibUpdateInfo );
    }
    mBuffersRemap[model_id] = id;
    mSlotAvailability[id]   = 0;
}
void GPUModelBuffersPool::RemoveModel( uint64_t id )
{
    auto slot_id = GetModelId( id );
    if ( slot_id < 0 )
        return;
    mSlotAvailability[slot_id] = 1;
    mBuffersRemap[id]          = -1;
}
} // namespace rh::rw::engine