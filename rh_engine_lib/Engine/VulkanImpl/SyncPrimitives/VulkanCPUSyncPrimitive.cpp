#include "VulkanCPUSyncPrimitive.h"
#include <DebugUtils/DebugLogger.h>
namespace rh::engine
{

VulkanCPUSyncPrimitive::VulkanCPUSyncPrimitive( vk::Device device )
    : m_vkDevice( device )
{
    vk::FenceCreateInfo info{};

    auto result = m_vkDevice.createFence( info );
    if ( result.result != vk::Result::eSuccess )
        debug::DebugLogger::ErrorFmt( "Failed to create fence:%s",
                                      vk::to_string( result.result ).c_str() );
    m_vkSyncPrim = result.value;
}

VulkanCPUSyncPrimitive::~VulkanCPUSyncPrimitive()
{
    m_vkDevice.destroyFence( m_vkSyncPrim );
}

VulkanCPUSyncPrimitive::operator vk::Fence() { return m_vkSyncPrim; }
} // namespace rh::engine