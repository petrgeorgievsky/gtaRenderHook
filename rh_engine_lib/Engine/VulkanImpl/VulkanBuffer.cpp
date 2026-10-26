#include "VulkanBuffer.h"

#include <vk_mem_alloc.h>

namespace rh::engine
{

vk::BufferUsageFlags ConvertUsage( uint32_t flags )
{
    vk::BufferUsageFlags res_flags{};
    if ( flags & BufferUsage::VertexBuffer )
        res_flags |= vk::BufferUsageFlagBits::eVertexBuffer |
                     vk::BufferUsageFlagBits::
                         eAccelerationStructureBuildInputReadOnlyKHR |
                     vk::BufferUsageFlagBits::eShaderDeviceAddress;
    if ( flags & BufferUsage::IndexBuffer )
        res_flags |= vk::BufferUsageFlagBits::eIndexBuffer |
                     vk::BufferUsageFlagBits::
                         eAccelerationStructureBuildInputReadOnlyKHR |
                     vk::BufferUsageFlagBits::eShaderDeviceAddress;
    if ( flags & BufferUsage::ConstantBuffer )
        res_flags |= vk::BufferUsageFlagBits::eUniformBuffer;
    if ( flags & BufferUsage::StagingBuffer )
        res_flags |= vk::BufferUsageFlagBits::eTransferSrc |
                     vk::BufferUsageFlagBits::eTransferDst;
    if ( flags & BufferUsage::RayTracingScratch )
        res_flags |= vk::BufferUsageFlagBits::eStorageBuffer |
                     vk::BufferUsageFlagBits::
                         eAccelerationStructureBuildInputReadOnlyKHR |
                     vk::BufferUsageFlagBits::eShaderDeviceAddress;
    if ( flags & BufferUsage::RayTracingShaderBindTable )
        res_flags |= vk::BufferUsageFlagBits::eStorageBuffer |
                     vk::BufferUsageFlagBits::eShaderBindingTableKHR |
                     vk::BufferUsageFlagBits::eShaderDeviceAddress;

    if ( flags & BufferUsage::StorageBuffer )
        res_flags |= vk::BufferUsageFlagBits::eStorageBuffer;

    return res_flags;
}

VulkanBuffer::VulkanBuffer( const VulkanBufferCreateInfo &create_info )
    : mDevice( create_info.mDevice )
{
    vk::BufferCreateInfo vk_create_info{};
    vk_create_info.sharingMode = vk::SharingMode::eExclusive;
    vk_create_info.size        = create_info.mSize;
    vk_create_info.usage       = ConvertUsage( create_info.mUsage );
    mAllocator                 = create_info.mAllocator->GetImpl();

    VmaAllocationCreateInfo allocInfo = {};
    if ( create_info.mFlags == DynamicGPUOnly )
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    else
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    VkBufferCreateInfo bufferCreateInfo = vk_create_info;
    VkBuffer           buffer;
    vmaCreateBuffer( mAllocator, &bufferCreateInfo, &allocInfo, &buffer,
                     &mAllocation, nullptr );
    mBuffer = buffer;

    if ( create_info.mInitDataPtr )
    {
        void *mapped_memory;
        vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
        memcpy( mapped_memory, create_info.mInitDataPtr, create_info.mSize );
        vmaUnmapMemory( mAllocator, mAllocation );
    }
}

VulkanBuffer::~VulkanBuffer()
{
    vmaDestroyBuffer( mAllocator, mBuffer, mAllocation );
}

void VulkanBuffer::Update( const void *data, uint32_t size )
{
    void *mapped_memory;
    vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
    memcpy( mapped_memory, data, size );
    vmaUnmapMemory( mAllocator, mAllocation );
}

void VulkanBuffer::Update( const void *data, uint32_t size,
                                       uint32_t offset )
{
    void *mapped_memory;
    vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
    memcpy( static_cast<char *>( mapped_memory ) + offset, data, size );
    vmaUnmapMemory( mAllocator, mAllocation );
}

void *VulkanBuffer::Lock()
{
    void *mapped_memory;
    vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
    return mapped_memory;
}

void VulkanBuffer::Unlock() { vmaUnmapMemory( mAllocator, mAllocation ); }

}
