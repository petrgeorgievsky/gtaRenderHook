//
// Created by peter on 24.10.2020.
//
#include "gpu_scene_materials_pool.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/descriptor_type.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;
GPUSceneMaterialsPool::GPUSceneMaterialsPool(
    rh::engine::IDescriptorSet *desc_set, uint64_t max_buffer_count,
    uint32_t materials_binding, uint32_t index_remap_binding )
    : mMaterialsBinding( materials_binding ),
      mIndexRemapBinding( index_remap_binding ), mGPUPool( desc_set )
{
}
void GPUSceneMaterialsPool::StoreMaterialData( MaterialUpdateData &model,
                                               uint64_t            instance_id )
{
    auto device = DeviceGlobals::RenderHookDevice.get();
    // index remap
    /*{
        std::array ib_list{ BufferUpdateInfo{
            0, VK_WHOLE_SIZE, model.mIndexRemapBuffer->Get() } };
        device->UpdateDescriptorSets(
            { .mSet              = mGPUPool,
              .mBinding          = mIndexRemapBinding,
              .mDescriptorType   = DescriptorType::RWBuffer,
              .mArrayStartIdx    = static_cast<uint32_t>( instance_id ),
              .mBufferUpdateInfo = ib_list } );
    }*/
    /*{
        auto buffer = device->CreateBuffer(
            { .mSize  = static_cast<uint32_t>( sizeof( MaterialData ) *
                                              model.mMaterialCount ),
              .mUsage = rh::engine::BufferUsage::StorageBuffer,
              .mInitDataPtr = reinterpret_cast<void *>( model.mMaterials ) } );
        mMaterialBuffers.push_back( buffer );
        std::array ib_list{ BufferUpdateInfo{ 0, VK_WHOLE_SIZE, buffer } };
        device->UpdateDescriptorSets(
            { .mSet              = mGPUPool,
              .mBinding          = mMaterialsBinding,
              .mDescriptorType   = DescriptorType::RWBuffer,
              .mArrayStartIdx    = static_cast<uint32_t>( instance_id ),
              .mBufferUpdateInfo = ib_list } );
    }*/
}
void GPUSceneMaterialsPool::ResetFrame()
{
    for ( auto b : mMaterialBuffers )
        delete b;
    mMaterialBuffers.clear();
}
} // namespace rh::rw::engine
