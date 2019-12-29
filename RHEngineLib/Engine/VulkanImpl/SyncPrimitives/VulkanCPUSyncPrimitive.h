#pragma once

#include <common.h>

#include "Engine/Common/ISyncPrimitive.h"

namespace rh::engine
{

/**
 * @brief CPU handle for GPU work synchronisation primitive(semaphore)
 * This primitive is sent to GPU and will be signaled
 */
class VulkanCPUSyncPrimitive : public ISyncPrimitive
{
  public:
    VulkanCPUSyncPrimitive( vk::Device device );
    ~VulkanCPUSyncPrimitive();

    operator vk::Fence();

  private:
    vk::Device m_vkDevice = nullptr;
    vk::Fence  m_vkSyncPrim;
};

} // namespace rh::engine