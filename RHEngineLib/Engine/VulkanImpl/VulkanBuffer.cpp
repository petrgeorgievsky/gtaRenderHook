#include "VulkanBuffer.h"

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
    return res_flags;
}

VulkanBuffer::VulkanBuffer( const VulkanBufferCreateInfo &create_info )
    : mDevice( create_info.mDevice )
{
    vk::BufferCreateInfo vk_create_info{};
    vk_create_info.sharingMode = vk::SharingMode::eExclusive;
    vk_create_info.size        = create_info.mSize;
    vk_create_info.usage       = ConvertUsage( create_info.mUsage );
    mBuffer                    = mDevice.createBuffer( vk_create_info );
    auto memory_req            = mDevice.getBufferMemoryRequirements( mBuffer );
    // mDevice.memory
    /// TODO: Compute allocation requirements carefully
    vk::MemoryAllocateInfo mem_alloc_info{};
    mem_alloc_info.memoryTypeIndex = 10;
    mem_alloc_info.allocationSize  = memory_req.size;
    mBufferMemory                  = mDevice.allocateMemory( mem_alloc_info );

    mDevice.bindBufferMemory( mBuffer, mBufferMemory, 0 );
}

VulkanBuffer::~VulkanBuffer()
{
    mDevice.destroyBuffer( mBuffer );
    mDevice.freeMemory( mBufferMemory );
}

void VulkanBuffer::Update( const void *data, uint32_t size )
{

    auto mapped_memory = mDevice.mapMemory( mBufferMemory, 0, size );
    memcpy( mapped_memory, data, size );
    mDevice.unmapMemory( mBufferMemory );
}