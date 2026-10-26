#pragma once

#include <common.h>

#include <Engine/Common/ISyncPrimitive.h>

namespace rh::engine
{

/**
 * @brief CPU handle for GPU work synchronisation primitive(semaphore)
 * This primitive is sent to the driver, and will be signaled by GPU.
 */
class VulkanCPUSyncPrimitive final : public ISyncPrimitive
{
  public:
    explicit VulkanCPUSyncPrimitive( vk::Device device );
    ~VulkanCPUSyncPrimitive() override;

    explicit operator vk::Fence() const;

  private:
    vk::Device m_vkDevice = nullptr;
    vk::Fence  m_vkSyncPrim;
    friend class VulkanDebugUtils;
};

} // namespace rh::engine