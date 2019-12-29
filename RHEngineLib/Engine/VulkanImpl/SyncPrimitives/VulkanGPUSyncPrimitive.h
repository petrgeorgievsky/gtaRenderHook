#pragma once
#include <common.h>

#include "Engine/Common/ISyncPrimitive.h"

namespace rh::engine
{

/**
 * @brief CPU handle for GPU work synchronisation primitive(semaphore)
 * This primitive is sent to GPU and will be signaled
 */
class VulkanGPUSyncPrimitive : public ISyncPrimitive
{
  public:
    VulkanGPUSyncPrimitive( vk::Device device );
    ~VulkanGPUSyncPrimitive();

    operator vk::Semaphore();

  private:
    vk::Device    m_vkDevice = nullptr;
    vk::Semaphore m_vkSyncPrim;
};

} // namespace rh::engine
