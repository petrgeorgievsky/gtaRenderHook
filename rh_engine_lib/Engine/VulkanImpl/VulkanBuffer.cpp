#include "VulkanBuffer.h"
#include "vk_mem_alloc.h"

using namespace rh::engine;

vk::BufferUsageFlags ConvertUsage( uint32_t flags )
{
    vk::BufferUsageFlags res_flags{};
    if ( flags & BufferUsage::VertexBuffer )
        res_flags |= vk::BufferUsageFlagBits::eVertexBuffer;
    if ( flags & BufferUsage::IndexBuffer )
        res_flags |= vk::BufferUsageFlagBits::eIndexBuffer;
    if ( flags & BufferUsage::ConstantBuffer )
        res_flags |= vk::BufferUsageFlagBits::eUniformBuffer;
    if ( flags & BufferUsage::StagingBuffer )
        res_flags |= vk::BufferUsageFlagBits::eTransferSrc;
    if ( flags & BufferUsage::RayTracingScratch )
        res_flags |= vk::BufferUsageFlagBits::eRayTracingNV;

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
    // mBuffer                    = mDevice.createBuffer( vk_create_info );
    // auto memory_req            = mDevice.getBufferMemoryRequirements( mBuffer
    // );

    // mDevice.memory
    /// TODO: Compute allocation requirements carefully, allow for custom
    /// allocations
    VmaAllocationCreateInfo allocInfo   = {};
    allocInfo.usage                     = VMA_MEMORY_USAGE_CPU_TO_GPU;
    VkBufferCreateInfo bufferCreateInfo = vk_create_info;
    VkBuffer           buffer;
    vmaCreateBuffer( mAllocator, &bufferCreateInfo, &allocInfo, &buffer,
                     &mAllocation, nullptr );
    mBuffer = buffer;
    /*
        VulkanMemoryAllocationInfo alloc_info{};
        alloc_info.mRequirements = memory_req;
        alloc_info.mDeviceLocal  = false;
        mBufferMemory = create_info.mAllocator->AllocateDeviceMemory( alloc_info
       );*/

    if ( create_info.mInitDataPtr )
    {
        void *mapped_memory;
        vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
        memcpy( mapped_memory, create_info.mInitDataPtr, create_info.mSize );
        vmaUnmapMemory( mAllocator, mAllocation );
        // mDevice.unmapMemory( mBufferMemory );
    }
    // mDevice.bindBufferMemory( mBuffer, mBufferMemory, 0 );
}

VulkanBuffer::~VulkanBuffer()
{
    /*mDevice.destroyBuffer( mBuffer );
    mDevice.freeMemory( mBufferMemory );*/
    vmaDestroyBuffer( mAllocator, mBuffer, mAllocation );
}

void VulkanBuffer::Update( const void *data, uint32_t size )
{
    void *mapped_memory;
    vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
    memcpy( mapped_memory, data, size );
    vmaUnmapMemory( mAllocator, mAllocation );
}

void rh::engine::VulkanBuffer::Update( const void *data, uint32_t size,
                                       uint32_t offset )
{
    void *mapped_memory;
    vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
    memcpy( reinterpret_cast<char *>( mapped_memory ) + offset, data, size );
    vmaUnmapMemory( mAllocator, mAllocation );
}
void *VulkanBuffer::Lock()
{
    void *mapped_memory;
    vmaMapMemory( mAllocator, mAllocation, &mapped_memory );
    return mapped_memory;
}
void VulkanBuffer::Unlock() { vmaUnmapMemory( mAllocator, mAllocation ); }
