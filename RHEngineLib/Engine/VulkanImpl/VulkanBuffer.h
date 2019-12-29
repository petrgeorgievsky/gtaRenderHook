#pragma once
#include "Engine/Common/IBuffer.h"
#include <common.h>

namespace rh::engine
{

struct VulkanBufferCreateInfo : BufferCreateInfo
{
    // Dependencies...
    vk::Device mDevice;
};

class VulkanBuffer : public IBuffer
{
  public:
    VulkanBuffer( const VulkanBufferCreateInfo &create_info );
    ~VulkanBuffer() override;
    void Update( const void *data, uint32_t size ) override;

    operator vk::Buffer() { return mBuffer; }

  private:
    vk::Device       mDevice;
    vk::Buffer       mBuffer;
    vk::DeviceMemory mBufferMemory;
};

} // namespace rh::engine
