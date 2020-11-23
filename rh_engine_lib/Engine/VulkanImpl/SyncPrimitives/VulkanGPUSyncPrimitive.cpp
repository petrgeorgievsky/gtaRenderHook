#include "VulkanGPUSyncPrimitive.h"

using namespace rh::engine;

VulkanGPUSyncPrimitive::VulkanGPUSyncPrimitive( vk::Device device )
    : m_vkDevice( device )
{
    m_vkSyncPrim = m_vkDevice.createSemaphore( {} );
}

VulkanGPUSyncPrimitive::~VulkanGPUSyncPrimitive()
{
    m_vkDevice.destroySemaphore( m_vkSyncPrim );
}

VulkanGPUSyncPrimitive::operator vk::Semaphore() { return m_vkSyncPrim; }