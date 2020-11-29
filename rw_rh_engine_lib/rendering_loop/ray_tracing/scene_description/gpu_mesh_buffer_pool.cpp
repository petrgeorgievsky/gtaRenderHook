//
// Created by peter on 15.05.2020.
//

#include "gpu_mesh_buffer_pool.h"
#include <Engine/Common/IDeviceState.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{

GPUModelBuffersPool::GPUModelBuffersPool( rh::engine::IDescriptorSet *desc_set,
                                          uint64_t buffer_count,
                                          uint32_t index_buffer_binding,
                                          uint32_t vertex_buffer_binding )
    : mGPUPool( desc_set ), mIndexBufferBinding( index_buffer_binding ),
      mVertexBufferBinding( vertex_buffer_binding )
{
    mSlotAvailability.resize( buffer_count, 1 );
    mBuffersRemap.resize( buffer_count, -1 );
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
        DeviceGlobals::RenderHookDevice->UpdateDescriptorSets( vbUpdateInfo );
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
        DeviceGlobals::RenderHookDevice->UpdateDescriptorSets( ibUpdateInfo );
    }
    mBuffersRemap[model_id] = id;
    mSlotAvailability[id]   = 0;
}
} // namespace rh::rw::engine