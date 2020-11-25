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
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV, nullptr };
    VkPhysicalDeviceProperties2 props{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &rt_props };
    vkGetPhysicalDeviceProperties2( gpu, &props );
    mRTInfo.mShaderGroupHandleSize  = rt_props.shaderGroupHandleSize;
    mRTInfo.mShaderGroupHandleAlign = rt_props.shaderGroupBaseAlignment;
    mRTInfo.mMaxRecursionDepth      = rt_props.maxRecursionDepth;
}

const VulkanRayTracingInfo &VulkanGPUInfo::GetRayTracingInfo() const
{
    return mRTInfo;
}

} // namespace rh::engine