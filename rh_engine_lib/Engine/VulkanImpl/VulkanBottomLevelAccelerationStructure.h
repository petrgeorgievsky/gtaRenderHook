//
// Created by peter on 29.04.2020.
//
#pragma once
#include <common.h>

#include "VulkanMemoryAllocator.h"

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
    IBuffer                   *mVertexBuffer;
    IBuffer                   *mIndexBuffer;
    uint32_t                   mVertexStride;
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
    explicit VulkanBottomLevelAccelerationStructure(
        const AccelerationStructureCreateInfoVulkan &create_info );
    ~VulkanBottomLevelAccelerationStructure();

    [[nodiscard]] std::uint64_t                GetScratchSize() const;
    std::uint64_t                GetAddress();
    vk::AccelerationStructureKHR GetImpl() const { return mAccel; }

    auto GetGeometry()
        -> const std::vector<vk::AccelerationStructureGeometryKHR> &
    {
        return mGeometry;
    }
    auto GetBuildRanges()
        -> const std::vector<vk::AccelerationStructureBuildRangeInfoKHR> &
    {
        return mBuildRanges;
    }

  private:
    vk::Device                                              mDevice;
    vk::AccelerationStructureKHR                            mAccel;
    std::vector<vk::AccelerationStructureGeometryKHR>       mGeometry{};
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> mBuildRanges{};
    VkBuffer      mBuffer{ VK_NULL_HANDLE };
    VmaAllocator  mAllocator;
    VmaAllocation mAllocation{};
    std::uint64_t mScratchSize;
    std::uint64_t mGPUHandle = 0;
};

} // namespace rh::engine