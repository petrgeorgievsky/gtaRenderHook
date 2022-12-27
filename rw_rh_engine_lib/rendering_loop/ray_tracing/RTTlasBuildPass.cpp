//
// Created by peter on 26.06.2020.
//

#include "RTTlasBuildPass.h"
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>

namespace rh::rw::engine
{
using namespace rh::engine;

RTTlasBuildPass::RTTlasBuildPass( rh::engine::IDeviceState &device )
    : Device( device )
{
    mTlasScratchBuffer =
        Device.CreateBuffer( { .mSize  = 1024 * 1024 * 8,
                               .mUsage = BufferUsage::RayTracingScratch,
                               .mFlags = BufferFlags::Dynamic } );

    mTlasCmdBuffer =
        dynamic_cast<VulkanCommandBuffer *>( device.CreateCommandBuffer() );

    BufferCreateInfo bufferCreateInfo{
        .mSize  = 8000 * sizeof( VkAccelerationStructureInstanceKHR ),
        .mUsage = BufferUsage::RayTracingScratch,
        .mFlags = BufferFlags::Dynamic };
    mTlasBuffer = Device.CreateBuffer( bufferCreateInfo );
}

RTTlasBuildPass::~RTTlasBuildPass()
{
    delete mTlasCmdBuffer;
    delete mTlasScratchBuffer;
    delete mTlasBuffer;
}

VulkanTopLevelAccelerationStructure *RTTlasBuildPass::Execute(
    std::vector<VkAccelerationStructureInstanceKHR> &&instance_buffer )
{
    auto &device = dynamic_cast<VulkanDeviceState &>( Device );

    auto tlas = device.CreateTLAS( { .mMaxInstanceCount = static_cast<uint32_t>(
                                         instance_buffer.size() ) } );
    mTlasCmdBuffer->BeginRecord();

    mTlasCmdBuffer->BuildTLAS( tlas, mTlasScratchBuffer, mTlasBuffer );

    MemoryBarrierInfo mem_barr{
        .mSrcMemoryAccess = MemoryAccessFlags::AccelerationStructureWrite,
        .mDstMemoryAccess = MemoryAccessFlags::AccelerationStructureRead };
    std::array barriers = { mem_barr };

    mTlasCmdBuffer->PipelineBarrier(
        { .mSrcStage       = PipelineStage::BuildAcceleration,
          .mDstStage       = PipelineStage::BuildAcceleration,
          .mMemoryBarriers = barriers } );

    mTlasCmdBuffer->EndRecord();

    mTlasBuffer->Update( instance_buffer.data(),
                         instance_buffer.size() *
                             sizeof( VkAccelerationStructureInstanceKHR ) );
    return tlas;
}
CommandBufferSubmitInfo
RTTlasBuildPass::GetSubmitInfo( ISyncPrimitive *dependency )
{
    return { mTlasCmdBuffer,
             dependency ? std::vector{ dependency }
                        : std::vector<ISyncPrimitive *>{},
             nullptr };
}

} // namespace rh::rw::engine
