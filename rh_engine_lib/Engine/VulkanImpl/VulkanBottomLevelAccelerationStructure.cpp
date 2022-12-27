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
    : mDevice( create_info.mDevice ),
      mAllocator( create_info.mAllocator->GetImpl() )
{
    using namespace vk;
    std::vector<uint32_t> max_prim_count{};
    for ( const auto &strip : create_info.mSplits )
    {
        AccelerationStructureGeometryKHR geometry{};

        geometry.geometryType = GeometryTypeKHR::eTriangles;
        //  geometryNv.flags        = vk::GeometryFlagBitsNV::eOpaque;
        auto idx_buffer =
            dynamic_cast<VulkanBuffer *>( create_info.mIndexBuffer );
        auto vtx_buffer =
            dynamic_cast<VulkanBuffer *>( create_info.mVertexBuffer );
        geometry.geometry.triangles.indexType = IndexType::eUint16;
        geometry.geometry.triangles.indexData =
            mDevice.getBufferAddress(
                vk::BufferDeviceAddressInfo{ *idx_buffer } ) +
            strip.mIndexOffset * sizeof( int16_t );
        geometry.geometry.triangles.vertexFormat = Format::eR32G32B32Sfloat;
        // TODO: FIX
        geometry.geometry.triangles.vertexStride = 64 + 32;
        geometry.geometry.triangles.maxVertex    = create_info.mVertexCount;
        geometry.geometry.triangles.vertexData   = mDevice.getBufferAddress(
            vk::BufferDeviceAddressInfo{ *vtx_buffer } );
        mGeometry.push_back( geometry );

        vk::AccelerationStructureBuildRangeInfoKHR build_range_info{};
        build_range_info.primitiveCount  = strip.mIndexCount / 3;
        build_range_info.primitiveOffset = 0;
        build_range_info.firstVertex     = strip.mVertexOffset;
        build_range_info.transformOffset = 0;
        mBuildRanges.push_back( build_range_info );

        max_prim_count.push_back( build_range_info.primitiveCount );
    }

    AccelerationStructureBuildGeometryInfoKHR build_geom_info{};
    build_geom_info.type          = AccelerationStructureTypeKHR::eBottomLevel;
    build_geom_info.mode          = BuildAccelerationStructureModeKHR::eBuild;
    build_geom_info.pGeometries   = mGeometry.data();
    build_geom_info.geometryCount = mGeometry.size();

    auto build_sizes = mDevice.getAccelerationStructureBuildSizesKHR(
        AccelerationStructureBuildTypeKHR::eHostOrDevice, build_geom_info,
        max_prim_count );

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkBufferCreateInfo buffer_create_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    buffer_create_info.size = build_sizes.accelerationStructureSize;
    buffer_create_info.usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vmaCreateBuffer( mAllocator, &buffer_create_info, &alloc_info, &mBuffer,
                     &mAllocation, nullptr );

    AccelerationStructureCreateInfoKHR as_create_info{};
    as_create_info.size   = build_sizes.accelerationStructureSize;
    as_create_info.buffer = mBuffer;
    as_create_info.type   = AccelerationStructureTypeKHR::eBottomLevel;

    // allocate

    mScratchSize = build_sizes.buildScratchSize;
    auto result  = mDevice.createAccelerationStructureKHR( as_create_info );

    if ( !CALL_VK_API( result.result,
                       TEXT( "Failed to create acceleration structure!" ) ) )
        return;
    mAccel       = result.value;
    mScratchSize = build_sizes.buildScratchSize;
}

VulkanBottomLevelAccelerationStructure::
    ~VulkanBottomLevelAccelerationStructure()
{
    mDevice.destroyAccelerationStructureKHR( mAccel );
    mDevice.destroyBuffer( mBuffer );
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
    mGPUHandle = mDevice.getBufferAddress( { mBuffer } );
    return mGPUHandle;
}
