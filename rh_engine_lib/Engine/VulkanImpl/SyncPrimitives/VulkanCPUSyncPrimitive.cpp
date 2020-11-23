#include "VulkanCPUSyncPrimitive.h"
using namespace rh::engine;

VulkanCPUSyncPrimitive::VulkanCPUSyncPrimitive( vk::Device device )
    : m_vkDevice( device )
{
    vk::FenceCreateInfo info{};
    m_vkSyncPrim = m_vkDevice.createFence( info );
}

VulkanCPUSyncPrimitive::~VulkanCPUSyncPrimitive()
{
    m_vkDevice.destroyFence( m_vkSyncPrim );
}

VulkanCPUSyncPrimitive::operator vk::Fence() { return m_vkSyncPrim; }
