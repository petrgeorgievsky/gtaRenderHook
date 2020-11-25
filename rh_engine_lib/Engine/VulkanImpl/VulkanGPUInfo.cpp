//
// Created by peter on 25.11.2020.
//

#include "VulkanGPUInfo.h"
namespace rh::engine
{

VulkanGPUInfo::VulkanGPUInfo( vk::PhysicalDevice gpu )
{

    // fill rt properties

    auto properties =
        gpu.getProperties2<vk::PhysicalDeviceProperties2,
                           vk::PhysicalDeviceRayTracingPropertiesNV>();
    auto rt_props = properties.get<vk::PhysicalDeviceRayTracingPropertiesNV>();
    mRTInfo.mShaderGroupHandleSize  = rt_props.shaderGroupHandleSize;
    mRTInfo.mShaderGroupHandleAlign = rt_props.shaderGroupBaseAlignment;
    mRTInfo.mMaxRecursionDepth      = rt_props.maxRecursionDepth;
}

const VulkanRayTracingInfo &VulkanGPUInfo::GetRayTracingInfo() const
{
    return mRTInfo;
}

} // namespace rh::engine