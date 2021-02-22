#include "VulkanMemoryAllocator.h"
#include <DebugUtils/DebugLogger.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#pragma warning( push, 0 )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#pragma clang diagnostic pop
#pragma warning( pop )

rh::engine::VulkanMemoryAllocator::VulkanMemoryAllocator(
    const VulkanMemoryAllocatorCreateInfo &create_info )
    : mPhysDevice( create_info.mPhysicalDevice ), mDevice( create_info.mDevice )
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice         = mPhysDevice;
    allocatorInfo.device                 = mDevice;
#ifdef ARCH_64BIT
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT |
                          VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
#endif
    vmaCreateAllocator( &allocatorInfo, &mAllocator );
}

rh::engine::VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
    char *stats_data = nullptr;
    vmaBuildStatsString( mAllocator, &stats_data, true );
    debug::DebugLogger::Log( std::string( stats_data ) );
    vmaFreeStatsString( mAllocator, stats_data );
    vmaDestroyAllocator( mAllocator );
}

vk::DeviceMemory rh::engine::VulkanMemoryAllocator::AllocateDeviceMemory(
    const VulkanMemoryAllocationInfo &info )
{
    auto memory_props = mPhysDevice.getMemoryProperties();

    auto select_memory_type_id =
        [&memory_props](
            uint32_t                                     memoryTypeBits,
            const vk::Flags<vk::MemoryPropertyFlagBits> &properties ) {
            auto        memory_type_count = memory_props.memoryTypeCount;
            const auto &memory_types      = memory_props.memoryTypes;
            for ( uint32_t i = 0; i < memory_type_count; ++i )
            {
                if ( ( memoryTypeBits & ( 1u << i ) ) &&
                     ( ( memory_types[i].propertyFlags & properties ) ==
                       properties ) )
                    return i;
            }
            return ~1u;
        };

    vk::MemoryAllocateInfo memory_alloc_info{};
    memory_alloc_info.allocationSize  = info.mRequirements.size;
    memory_alloc_info.memoryTypeIndex = select_memory_type_id(
        info.mRequirements.memoryTypeBits,
        info.mDeviceLocal ? vk::MemoryPropertyFlagBits::eDeviceLocal
                          : vk::MemoryPropertyFlagBits::eHostVisible );
    return vk::DeviceMemory();
    // return mDevice.allocateMemory( memory_alloc_info );
}
