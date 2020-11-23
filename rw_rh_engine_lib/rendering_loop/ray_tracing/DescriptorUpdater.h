//
// Created by peter on 09.08.2020.
//
#pragma once
#include <Engine/Common/IDeviceState.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
namespace rh::rw::engine
{
class DescSetUpdateBatch
{
  private:
    rh::engine::IDescriptorSet *mActiveSet = nullptr;
    rh::engine::IDeviceState &  mDeviceState =
        *rh::rw::engine::DeviceGlobals::RenderHookDevice;

  public:
    [[nodiscard]] DescSetUpdateBatch &Begin( rh::engine::IDescriptorSet *set )
    {
        mActiveSet = set;
        return *this;
    }
    DescSetUpdateBatch &UpdateImage(
        uint32_t binding, rh::engine::DescriptorType type,
        rh::engine::ArrayProxy<rh::engine::ImageUpdateInfo> update_info,
        uint32_t array_start_idx = 0 )
    {
        rh::engine::DescriptorSetUpdateInfo updateInfo{};
        updateInfo.mSet             = mActiveSet;
        updateInfo.mBinding         = binding;
        updateInfo.mDescriptorType  = type;
        updateInfo.mImageUpdateInfo = update_info;
        updateInfo.mArrayStartIdx   = array_start_idx;
        mDeviceState.UpdateDescriptorSets( updateInfo );
        return *this;
    }

    DescSetUpdateBatch &UpdateBuffer(
        uint32_t binding, rh::engine::DescriptorType type,
        rh::engine::ArrayProxy<rh::engine::BufferUpdateInfo> update_info,
        uint32_t array_start_idx = 0 )
    {
        rh::engine::DescriptorSetUpdateInfo updateInfo{};
        updateInfo.mSet              = mActiveSet;
        updateInfo.mBinding          = binding;
        updateInfo.mDescriptorType   = type;
        updateInfo.mBufferUpdateInfo = update_info;
        updateInfo.mArrayStartIdx    = array_start_idx;
        mDeviceState.UpdateDescriptorSets( updateInfo );
        return *this;
    }
    DescSetUpdateBatch &End()
    {
        mActiveSet = nullptr;
        return *this;
    }
};
} // namespace rh::rw::engine