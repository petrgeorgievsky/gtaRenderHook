#pragma once
#include <common.h>

#include <Engine/Common/ISyncPrimitive.h>

namespace rh::engine
{

/**
 * @brief Handle for GPU work synchronisation primitive(semaphore)
 * This primitive is sent to GPU and will be signaled
 */
class VulkanGPUSyncPrimitive final : public ISyncPrimitive
{
  public:
    explicit VulkanGPUSyncPrimitive( vk::Device device );
    ~VulkanGPUSyncPrimitive() override;

    explicit operator vk::Semaphore() const;

    vk::Semaphore &GetImpl() { return m_vkSyncPrim; }

  private:
    vk::Device    m_vkDevice = nullptr;
    vk::Semaphore m_vkSyncPrim;
    friend class VulkanDebugUtils;
};

} // namespace rh::engine
