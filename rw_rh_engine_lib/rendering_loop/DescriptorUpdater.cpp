//
// Created by peter on 09.08.2020.
//

#include "rendering_loop/DescriptorUpdater.h"
namespace rh::rw::engine
{
DescSetUpdateBatch::DescSetUpdateBatch( rh::engine::IDeviceState &device )
    : Device( device )
{
}
DescSetUpdateBatch &DescSetUpdateBatch::Begin( rh::engine::IDescriptorSet *set )
{
    mActiveSet = set;
    return *this;
}
DescSetUpdateBatch &DescSetUpdateBatch::UpdateImage(
    uint32_t binding, rh::engine::DescriptorType type,
    rh::engine::ArrayProxy<rh::engine::ImageUpdateInfo> update_info,
    uint32_t                                            array_start_idx )
{
    rh::engine::DescriptorSetUpdateInfo updateInfo{};
    updateInfo.mSet             = mActiveSet;
    updateInfo.mBinding         = binding;
    updateInfo.mDescriptorType  = type;
    updateInfo.mImageUpdateInfo = update_info;
    updateInfo.mArrayStartIdx   = array_start_idx;
    Device.UpdateDescriptorSets( updateInfo );
    return *this;
}
DescSetUpdateBatch &DescSetUpdateBatch::UpdateBuffer(
    uint32_t binding, rh::engine::DescriptorType type,
    rh::engine::ArrayProxy<rh::engine::BufferUpdateInfo> update_info,
    uint32_t                                             array_start_idx )
{
    rh::engine::DescriptorSetUpdateInfo updateInfo{};
    updateInfo.mSet              = mActiveSet;
    updateInfo.mBinding          = binding;
    updateInfo.mDescriptorType   = type;
    updateInfo.mBufferUpdateInfo = update_info;
    updateInfo.mArrayStartIdx    = array_start_idx;
    Device.UpdateDescriptorSets( updateInfo );
    return *this;
}
DescSetUpdateBatch &DescSetUpdateBatch::End()
{
    mActiveSet = nullptr;
    return *this;
}
} // namespace rh::rw::engine
