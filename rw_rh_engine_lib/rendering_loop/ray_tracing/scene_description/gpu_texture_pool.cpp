//
// Created by peter on 15.05.2020.
//

#include "gpu_texture_pool.h"
#include <Engine/Common/IDeviceState.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{

GPUTexturePool::GPUTexturePool( rh::engine::IDescriptorSet *desc_set,
                                uint32_t binding_id, uint64_t descriptor_count )
    : mGPUPool( desc_set ), mBindingId( binding_id )
{
    mSlotAvailability.resize( descriptor_count, 1 );
    mBuffersRemap.resize( descriptor_count, -1 );
}

uint64_t GPUTexturePool::StoreTexture( rh::engine::IImageView *image,
                                       uint64_t                tex_id )
{
    using namespace rh::engine;
    uint64_t id = 0;

    // TODO: Can be improved and optimized, can be iterated from last free
    // resource
    while ( id < mSlotAvailability.size() && mSlotAvailability[id] == 0 )
        id++;

    std::array<ImageUpdateInfo, 1> img_upd_list = {
        { ImageLayout::ShaderReadOnly, image, nullptr } };

    DescriptorSetUpdateInfo imgUpdateInfo{};
    imgUpdateInfo.mSet             = mGPUPool;
    imgUpdateInfo.mBinding         = mBindingId;
    imgUpdateInfo.mDescriptorType  = DescriptorType::ROTexture;
    imgUpdateInfo.mArrayStartIdx   = id;
    imgUpdateInfo.mImageUpdateInfo = img_upd_list;
    DeviceGlobals::RenderHookDevice->UpdateDescriptorSets( imgUpdateInfo );

    mBuffersRemap[tex_id] = id;
    mSlotAvailability[id] = 0;
    return id;
}
void GPUTexturePool::RemoveTexture( uint64_t id )
{
    auto slot_id = GetTexId( id );
    if ( slot_id < 0 )
        return;
    mSlotAvailability[slot_id] = 1;
    mBuffersRemap[id]          = -1;
}
int32_t GPUTexturePool::GetTexId( uint64_t tex_id )
{
    return mBuffersRemap[tex_id];
}

} // namespace rh::rw::engine