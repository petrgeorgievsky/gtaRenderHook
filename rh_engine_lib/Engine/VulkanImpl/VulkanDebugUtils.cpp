//
// Created by peter on 03.06.2020.
//

#include "VulkanDebugUtils.h"
#include "SyncPrimitives/VulkanCPUSyncPrimitive.h"
#include "SyncPrimitives/VulkanGPUSyncPrimitive.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
namespace rh::engine
{

void VulkanDebugUtils::SetDebugName( IBuffer *buffer, std::string name )
{
    auto *buffer_impl = dynamic_cast<VulkanBuffer *>( buffer );

    vk::DebugUtilsObjectNameInfoEXT objectNameInfo{};
    objectNameInfo.objectHandle =
        ( uint64_t ) static_cast<VkBuffer>( buffer_impl->mBuffer );
    objectNameInfo.objectType  = vk::Buffer::objectType;
    objectNameInfo.pObjectName = name.data();

    buffer_impl->mDevice.setDebugUtilsObjectNameEXT( objectNameInfo );
}

void VulkanDebugUtils::SetDebugName( ICommandBuffer *buffer, std::string name )
{
    auto *buffer_impl = dynamic_cast<VulkanCommandBuffer *>( buffer );

    vk::DebugUtilsObjectNameInfoEXT objectNameInfo{};
    objectNameInfo.objectHandle =
        ( uint64_t ) static_cast<VkCommandBuffer>( buffer_impl->m_vkCmdBuffer );
    objectNameInfo.objectType  = vk::CommandBuffer::objectType;
    objectNameInfo.pObjectName = name.data();

    buffer_impl->mDevice.setDebugUtilsObjectNameEXT( objectNameInfo );
}

void VulkanDebugUtils::SetDebugName( ISyncPrimitive *sp, std::string name )
{
    if ( auto *gpu_impl = dynamic_cast<VulkanGPUSyncPrimitive *>( sp ) )
    {
        vk::DebugUtilsObjectNameInfoEXT objectNameInfo{};
        objectNameInfo.objectHandle =
            ( uint64_t ) static_cast<VkSemaphore>( gpu_impl->m_vkSyncPrim );
        objectNameInfo.objectType  = vk::Semaphore::objectType;
        objectNameInfo.pObjectName = name.data();

        gpu_impl->m_vkDevice.setDebugUtilsObjectNameEXT( objectNameInfo );
    }
    else if ( auto *cpu_impl = dynamic_cast<VulkanCPUSyncPrimitive *>( sp ) )
    {
        vk::DebugUtilsObjectNameInfoEXT objectNameInfo{};
        objectNameInfo.objectHandle =
            ( uint64_t ) static_cast<VkFence>( cpu_impl->m_vkSyncPrim );
        objectNameInfo.objectType  = vk::Fence::objectType;
        objectNameInfo.pObjectName = name.data();

        cpu_impl->m_vkDevice.setDebugUtilsObjectNameEXT( objectNameInfo );
    }
}

} // namespace rh::engine