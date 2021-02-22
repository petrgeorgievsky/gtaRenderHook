//
// Created by peter on 25.11.2020.
//

#include "VulkanGPUInfo.h"
namespace rh::engine
{

VulkanGPUInfo::VulkanGPUInfo( vk::PhysicalDevice gpu )
{

    // fill rt properties

    VkPhysicalDeviceRayTracingPropertiesNV rt_props{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV,
        .pNext = nullptr };
    VkPhysicalDeviceProperties2 props{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &rt_props };
    vkGetPhysicalDeviceProperties2( gpu, &props );
    mRTInfo.mShaderGroupHandleSize  = rt_props.shaderGroupHandleSize;
    mRTInfo.mShaderGroupHandleAlign = rt_props.shaderGroupBaseAlignment;
    mRTInfo.mMaxRecursionDepth      = rt_props.maxRecursionDepth;

    LimitsInfo.BufferOffsetMinAlign =
        props.properties.limits.minUniformBufferOffsetAlignment;
}

const VulkanRayTracingInfo &VulkanGPUInfo::GetRayTracingInfo() const
{
    return mRTInfo;
}

const DeviceLimitsInfo &VulkanGPUInfo::GetLimitsInfo() const
{
    return LimitsInfo;
}

} // namespace rh::engine