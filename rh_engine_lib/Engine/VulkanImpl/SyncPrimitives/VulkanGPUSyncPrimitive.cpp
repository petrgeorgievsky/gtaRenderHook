#include "VulkanGPUSyncPrimitive.h"
#include <DebugUtils/DebugLogger.h>

namespace rh::engine
{

VulkanGPUSyncPrimitive::VulkanGPUSyncPrimitive( vk::Device device )
    : m_vkDevice( device )
{
    auto result = m_vkDevice.createSemaphore( {} );
    if ( result.result != vk::Result::eSuccess )
    {
        debug::DebugLogger::ErrorFmt( "Failed to create sampler:%s",
                                      vk::to_string( result.result ).c_str() );
    }
    else
        m_vkSyncPrim = result.value;
}

VulkanGPUSyncPrimitive::~VulkanGPUSyncPrimitive()
{
    m_vkDevice.destroySemaphore( m_vkSyncPrim );
}

VulkanGPUSyncPrimitive::operator vk::Semaphore() { return m_vkSyncPrim; }
} // namespace rh::engine