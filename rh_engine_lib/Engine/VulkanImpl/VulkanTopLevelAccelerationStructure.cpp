//
// Created by peter on 01.05.2020.
//

#include "VulkanTopLevelAccelerationStructure.h"
#include "vk_mem_alloc.h"
namespace rh::engine
{

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure(
    const TLASCreateInfoVulkan &create_info )
    : mDevice( create_info.mDevice )
{
    vk::AccelerationStructureCreateInfoNV vk_ac_create_info{};

    mAccelInfo.instanceCount = create_info.mMaxInstanceCount;
    mAccelInfo.type          = vk::AccelerationStructureTypeNV::eTopLevel;
    vk_ac_create_info.info   = mAccelInfo;

    mAccel = mDevice.createAccelerationStructureNV( vk_ac_create_info );
    vk::AccelerationStructureMemoryRequirementsInfoNV
        accelerationStructureMemoryRequirementsInfoNv{};
    accelerationStructureMemoryRequirementsInfoNv.accelerationStructure =
        mAccel;
    vk::MemoryRequirements2 req =
        mDevice.getAccelerationStructureMemoryRequirementsNV(
            accelerationStructureMemoryRequirementsInfoNv );

    // allocate
    /*VulkanMemoryAllocationInfo alloc_info{};
    alloc_info.mRequirements = req.memoryRequirements;
    alloc_info.mDeviceLocal  = true;
    mAccelMemory = create_info.mAllocator->AllocateDeviceMemory( alloc_info );*/

    mAllocator                        = create_info.mAllocator->GetImpl();
    VkMemoryRequirements requirements = req.memoryRequirements;

    VmaAllocationInfo       allocationDetail{};
    VmaAllocationCreateInfo allocationCreateInfo{};

    allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
    // TODO: HANLE ERRORS
    vmaAllocateMemory( mAllocator, &requirements, &allocationCreateInfo,
                       &mAllocation, &allocationDetail );
    // bind
    vk::BindAccelerationStructureMemoryInfoNV bind_info{};
    bind_info.accelerationStructure = mAccel;
    bind_info.memory                = allocationDetail.deviceMemory;
    bind_info.memoryOffset          = allocationDetail.offset;
    // bind_info.memory                = mAccelMemory;

    mDevice.bindAccelerationStructureMemoryNV( 1, &bind_info );

    // compute scratch size
    vk::AccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{
        vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch,
        mAccel };
    mScratchSize = mDevice
                       .getAccelerationStructureMemoryRequirementsNV(
                           memoryRequirementsInfo )
                       .memoryRequirements.size;
}
VulkanTopLevelAccelerationStructure::~VulkanTopLevelAccelerationStructure()
{
    mDevice.destroyAccelerationStructureNV( mAccel );
    vmaFreeMemory( mAllocator, mAllocation );
    /*mDevice.destroyAccelerationStructureNV( mAccel );
    mDevice.freeMemory( mAccelMemory );*/
}

std::uint64_t VulkanTopLevelAccelerationStructure::GetScratchSize()
{
    return mScratchSize;
}
} // namespace rh::engine