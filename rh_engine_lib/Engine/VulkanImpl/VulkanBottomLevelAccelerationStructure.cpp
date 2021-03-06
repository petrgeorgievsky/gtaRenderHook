//
// Created by peter on 29.04.2020.
//

#include "VulkanBottomLevelAccelerationStructure.h"
#include "VulkanBuffer.h"
#include "VulkanCommon.h"
#include <DebugUtils/DebugLogger.h>

#include <vk_mem_alloc.h>

using namespace rh::engine;

VulkanBottomLevelAccelerationStructure::VulkanBottomLevelAccelerationStructure(
    const AccelerationStructureCreateInfoVulkan &create_info )
    : mDevice( create_info.mDevice )
{
    vk::AccelerationStructureCreateInfoNV vk_ac_create_info{};

    for ( const auto &strip : create_info.mSplits )
    {
        vk::GeometryNV geometryNv{};

        geometryNv.geometryType = vk::GeometryTypeNV::eTriangles;
        //  geometryNv.flags        = vk::GeometryFlagBitsNV::eOpaque;
        geometryNv.geometry.triangles.indexType = vk::IndexType::eUint16;
        geometryNv.geometry.triangles.indexOffset =
            strip.mIndexOffset * sizeof( int16_t );
        geometryNv.geometry.triangles.indexCount = strip.mIndexCount;
        geometryNv.geometry.triangles.indexData =
            *dynamic_cast<VulkanBuffer *>( create_info.mIndexBuffer );
        geometryNv.geometry.triangles.vertexFormat =
            vk::Format::eR32G32B32Sfloat;
        // TODO: FIX
        geometryNv.geometry.triangles.vertexStride = 64 + 32;
        geometryNv.geometry.triangles.vertexOffset = 0;
        geometryNv.geometry.triangles.vertexCount  = create_info.mVertexCount;
        geometryNv.geometry.triangles.vertexData =
            *dynamic_cast<VulkanBuffer *>( create_info.mVertexBuffer );
        mGeometry.push_back( geometryNv );
    }

    mAccelInfo.geometryCount = static_cast<uint32_t>( mGeometry.size() );
    mAccelInfo.pGeometries   = mGeometry.data();
    mAccelInfo.type          = vk::AccelerationStructureTypeNV::eBottomLevel;
    mAccelInfo.flags =
        vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace;
    vk_ac_create_info.info = mAccelInfo;

    auto result = mDevice.createAccelerationStructureNV( vk_ac_create_info );

    if ( !CALL_VK_API( result.result,
                       TEXT( "Failed to create acceleration structure!" ) ) )
        return;
    mAccel = result.value;

    vk::AccelerationStructureMemoryRequirementsInfoNV
        accelerationStructureMemoryRequirementsInfoNv{};
    accelerationStructureMemoryRequirementsInfoNv.accelerationStructure =
        mAccel;
    vk::MemoryRequirements2 req =
        mDevice.getAccelerationStructureMemoryRequirementsNV(
            accelerationStructureMemoryRequirementsInfoNv );

    // allocate

    mAllocator = create_info.mAllocator->GetImpl();

    VkMemoryRequirements requirements = req.memoryRequirements;

    VmaAllocationInfo       allocationDetail{};
    VmaAllocationCreateInfo allocationCreateInfo{};

    allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
    // TODO: HANDLE ERRORS
    vmaAllocateMemory( mAllocator, &requirements, &allocationCreateInfo,
                       &mAllocation, &allocationDetail );

    /*VulkanMemoryAllocationInfo alloc_info{};
    alloc_info.mRequirements = req.memoryRequirements;
    alloc_info.mDeviceLocal  = true;
    mAccelMemory = create_info.mAllocator->AllocateDeviceMemory( alloc_info );*/

    // bind
    vk::BindAccelerationStructureMemoryInfoNV bind_info{};
    bind_info.accelerationStructure = mAccel;
    bind_info.memory                = allocationDetail.deviceMemory;
    bind_info.memoryOffset          = allocationDetail.offset;

    if ( mDevice.bindAccelerationStructureMemoryNV( 1, &bind_info ) !=
         vk::Result::eSuccess )
    {
        debug::DebugLogger::Error( "Failed to bind BLAS memory!" );
        std::terminate();
    }

    // compute scratch size
    vk::AccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{
        vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch,
        mAccel };
    mScratchSize = mDevice
                       .getAccelerationStructureMemoryRequirementsNV(
                           memoryRequirementsInfo )
                       .memoryRequirements.size;
}

VulkanBottomLevelAccelerationStructure::
    ~VulkanBottomLevelAccelerationStructure()
{
    mDevice.destroyAccelerationStructureNV( mAccel );
    vmaFreeMemory( mAllocator, mAllocation );
}
vk::DeviceSize VulkanBottomLevelAccelerationStructure::GetScratchSize() const
{
    return mScratchSize;
}
std::uint64_t VulkanBottomLevelAccelerationStructure::GetAddress()
{
    if ( mGPUHandle != 0 )
        return mGPUHandle;
    auto result = mDevice.getAccelerationStructureHandleNV(
        mAccel, sizeof( uint64_t ), &mGPUHandle );
    if ( result != vk::Result::eSuccess )
    {
        debug::DebugLogger::Error( "Failed to get BLAS address!" );
        std::terminate();
        return 0;
    }
    return mGPUHandle;
}
