//
// Created by peter on 09.08.2020.
//
#pragma once
#include <Engine/Common/IDeviceState.h>

namespace rh::rw::engine
{
class DescSetUpdateBatch
{
  private:
    rh::engine::IDeviceState &  Device;
    rh::engine::IDescriptorSet *mActiveSet = nullptr;

  public:
    DescSetUpdateBatch( rh::engine::IDeviceState &device );

    [[nodiscard]] DescSetUpdateBatch &Begin( rh::engine::IDescriptorSet *set );
    DescSetUpdateBatch &              End();

    DescSetUpdateBatch &UpdateImage(
        uint32_t binding, rh::engine::DescriptorType type,
        rh::engine::ArrayProxy<rh::engine::ImageUpdateInfo> update_info,
        uint32_t array_start_idx = 0 );
    DescSetUpdateBatch &UpdateBuffer(
        uint32_t binding, rh::engine::DescriptorType type,
        rh::engine::ArrayProxy<rh::engine::BufferUpdateInfo> update_info,
        uint32_t array_start_idx = 0 );
};
} // namespace rh::rw::engine