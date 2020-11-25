#pragma once
#include <common.h>
VK_DEFINE_HANDLE( VmaAllocator )

namespace rh::engine
{
struct VulkanMemoryAllocationInfo
{
    vk::MemoryRequirements mRequirements;
    bool                   mDeviceLocal;
};
struct VulkanMemoryAllocatorCreateInfo
{
    vk::PhysicalDevice mPhysicalDevice;
    vk::Device         mDevice;
};

class VulkanMemoryAllocator
{
  public:
    VulkanMemoryAllocator( const VulkanMemoryAllocatorCreateInfo &create_info );
    ~VulkanMemoryAllocator();

    vk::DeviceMemory
                 AllocateDeviceMemory( const VulkanMemoryAllocationInfo &info );
    VmaAllocator GetImpl() { return mAllocator; }

  private:
    vk::PhysicalDevice mPhysDevice;
    vk::Device         mDevice;
    VmaAllocator       mAllocator{};
};
} // namespace rh::engine