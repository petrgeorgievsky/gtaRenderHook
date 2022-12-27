//
// Created by peter on 01.05.2020.
//

#include "VulkanTopLevelAccelerationStructure.h"
#include "VulkanCommon.h"
#include <DebugUtils/DebugLogger.h>

#include <vk_mem_alloc.h>

namespace rh::engine
{

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure(
    const TLASCreateInfoVulkan &create_info )
    : mDevice( create_info.mDevice ),
      mAllocator( create_info.mAllocator->GetImpl() )
{
    vk::AccelerationStructureGeometryKHR geometry_khr{};
    geometry_khr.geometryType = vk::GeometryTypeKHR::eInstances;
    geometry_khr.geometry.instances.sType =
        vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;
    mMaxInstances = create_info.mMaxInstanceCount;

    vk::AccelerationStructureBuildGeometryInfoKHR build_geometry_info_khr{};
    build_geometry_info_khr.type = vk::AccelerationStructureTypeKHR::eTopLevel;
    build_geometry_info_khr.mode =
        vk::BuildAccelerationStructureModeKHR::eBuild;
    build_geometry_info_khr.pGeometries   = &geometry_khr;
    build_geometry_info_khr.geometryCount = 1;

    auto build_sizes = mDevice.getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice, build_geometry_info_khr,
        { create_info.mMaxInstanceCount } );

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkBufferCreateInfo buffer_create_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    buffer_create_info.size = build_sizes.accelerationStructureSize;
    buffer_create_info.usage =
        VkBufferUsageFlagBits::
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vmaCreateBuffer( mAllocator, &buffer_create_info, &alloc_info, &mBuffer,
                     &mAllocation, nullptr );

    vk::AccelerationStructureCreateInfoKHR vk_ac_create_info{};

    vk_ac_create_info.type   = vk::AccelerationStructureTypeKHR::eTopLevel;
    vk_ac_create_info.size   = build_sizes.accelerationStructureSize;
    vk_ac_create_info.buffer = mBuffer; //

    auto result = mDevice.createAccelerationStructureKHR( vk_ac_create_info );
    if ( !CALL_VK_API( result.result,
                       TEXT( "Failed to create acceleration structure!" ) ) )
        return;
    mAccel = result.value;

    mScratchSize = build_sizes.buildScratchSize;
}
VulkanTopLevelAccelerationStructure::~VulkanTopLevelAccelerationStructure()
{
    mDevice.destroyAccelerationStructureKHR( mAccel );
    mDevice.destroyBuffer( mBuffer );
    vmaFreeMemory( mAllocator, mAllocation );
    /*mDevice.destroyAccelerationStructureNV( mAccel );
    mDevice.freeMemory( mAccelMemory );*/
}

std::uint64_t VulkanTopLevelAccelerationStructure::GetScratchSize()
{
    return mScratchSize;
}
} // namespace rh::engine