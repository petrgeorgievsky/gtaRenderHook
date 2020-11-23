//
// Created by peter on 29.04.2020.
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

struct GeometryStrip
{
    uint32_t mVertexOffset;
    uint32_t mIndexOffset;
    uint32_t mVertexCount;
    uint32_t mIndexCount;
};

struct AccelerationStructureCreateInfo
{
    IBuffer *                  mVertexBuffer;
    IBuffer *                  mIndexBuffer;
    uint32_t                   mVertexCount;
    uint32_t                   mIndexCount;
    std::vector<GeometryStrip> mSplits;
};

struct AccelerationStructureCreateInfoVulkan : AccelerationStructureCreateInfo
{
    // Dependencies...
    vk::Device             mDevice;
    VulkanMemoryAllocator *mAllocator;
};
class VulkanBottomLevelAccelerationStructure
{
  public:
    VulkanBottomLevelAccelerationStructure(
        const AccelerationStructureCreateInfoVulkan &create_info );
    ~VulkanBottomLevelAccelerationStructure();

    std::uint64_t                   GetScratchSize() const;
    std::uint64_t                   GetAddress();
    vk::AccelerationStructureNV     GetImpl() { return mAccel; }
    vk::AccelerationStructureInfoNV GetImplInfo() { return mAccelInfo; }

  private:
    vk::Device                      mDevice;
    vk::AccelerationStructureNV     mAccel;
    vk::AccelerationStructureInfoNV mAccelInfo;
    std::vector<vk::GeometryNV>     mGeometry;
    vk::DeviceMemory                mAccelMemory;
    VmaAllocator                    mAllocator;
    VmaAllocation                   mAllocation{};
    std::uint64_t                   mScratchSize;
    std::uint64_t                   mGPUHandle = 0;
};
} // namespace rh::engine