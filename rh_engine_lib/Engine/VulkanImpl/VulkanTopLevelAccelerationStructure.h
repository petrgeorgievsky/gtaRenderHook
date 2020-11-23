//
// Created by peter on 01.05.2020.
//
#pragma once
#include "VulkanMemoryAllocator.h"
#include <common.h>
#include <cstdint>

VK_DEFINE_HANDLE( VmaAllocator )
VK_DEFINE_HANDLE( VmaAllocation )

namespace rh::engine
{
class IBuffer;
struct TLASCreateInfo
{
    uint32_t mMaxInstanceCount;
};
struct TLASCreateInfoVulkan : TLASCreateInfo
{
    // Dependencies...
    vk::Device             mDevice;
    VulkanMemoryAllocator *mAllocator;
};

struct VkTransformMatrixKHR
{
    float matrix[3][4];
};

struct VkAccelerationStructureInstanceNV
{
    VkTransformMatrixKHR      transform;
    uint32_t                  instanceCustomIndex : 24;
    uint32_t                  mask : 8;
    uint32_t                  instanceShaderBindingTableRecordOffset : 24;
    VkGeometryInstanceFlagsNV flags : 8;
    uint64_t                  accelerationStructureReference;
};

class VulkanTopLevelAccelerationStructure
{
  public:
    VulkanTopLevelAccelerationStructure(
        const TLASCreateInfoVulkan &create_info );
    virtual ~VulkanTopLevelAccelerationStructure();

    std::uint64_t                   GetScratchSize();
    vk::AccelerationStructureNV     GetImpl() { return mAccel; }
    vk::AccelerationStructureInfoNV GetImplInfo() { return mAccelInfo; }

  private:
    vk::Device                      mDevice;
    vk::AccelerationStructureNV     mAccel;
    vk::AccelerationStructureInfoNV mAccelInfo;

    VmaAllocator     mAllocator;
    VmaAllocation    mAllocation;
    vk::DeviceMemory mAccelMemory;
    std::uint64_t    mScratchSize;
};
} // namespace rh::engine