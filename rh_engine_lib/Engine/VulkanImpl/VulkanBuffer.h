#pragma once
#include "Engine/Common/IBuffer.h"
#include "VulkanDebugUtils.h"
#include "VulkanMemoryAllocator.h"

#include <common.h>
VK_DEFINE_HANDLE( VmaAllocator )
VK_DEFINE_HANDLE( VmaAllocation )
namespace rh::engine
{

struct VulkanBufferCreateInfo : BufferCreateInfo
{
    // Dependencies...
    vk::Device             mDevice;
    VulkanMemoryAllocator *mAllocator;
};

class VulkanBuffer : public IBuffer
{
  public:
    VulkanBuffer( const VulkanBufferCreateInfo &create_info );
    ~VulkanBuffer() override;
    void Update( const void *data, uint32_t size ) override;
    void Update( const void *data, uint32_t size, uint32_t offset ) override;

          operator vk::Buffer() { return mBuffer; }
    void *Lock() override;
    void  Unlock() override;

  private:
    vk::Device       mDevice;
    vk::Buffer       mBuffer;
    vk::DeviceMemory mBufferMemory;
    VmaAllocation    mAllocation;
    VmaAllocator     mAllocator;
    friend class VulkanDebugUtils;
};

} // namespace rh::engine
