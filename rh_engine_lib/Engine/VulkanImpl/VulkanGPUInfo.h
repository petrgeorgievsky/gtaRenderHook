//
// Created by peter on 25.11.2020.
//
#pragma once
#include "common.h"
#include <Engine/Common/IDeviceState.h>

namespace rh::engine
{

/**
 * Contains vulkan raytracing constants
 */
struct VulkanRayTracingInfo
{
    bool mSupported;
    // Size of a one record in shader group for RT shaders
    uint32_t mShaderGroupHandleSize;
    // Alignment of a one record in shader group for RT shaders
    uint32_t mShaderGroupHandleAlign;
    uint32_t mMaxRecursionDepth;
    uint32_t GetAlignedSGHandleSize() const
    {
        return ( mShaderGroupHandleSize + ( mShaderGroupHandleAlign - 1 ) ) &
               ~( mShaderGroupHandleAlign - 1 );
    }
};

class VulkanGPUInfo
{
  public:
    VulkanGPUInfo( vk::PhysicalDevice gpu );
    const VulkanRayTracingInfo &GetRayTracingInfo() const;
    const DeviceLimitsInfo &    GetLimitsInfo() const;

  private:
    DeviceLimitsInfo     LimitsInfo;
    VulkanRayTracingInfo mRTInfo;
};
} // namespace rh::engine