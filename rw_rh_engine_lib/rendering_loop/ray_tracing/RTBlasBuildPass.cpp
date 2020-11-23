//
// Created by peter on 26.06.2020.
//

#include "RTBlasBuildPass.h"
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/ICommandBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/VulkanImpl/VulkanBottomLevelAccelerationStructure.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDebugUtils.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{

void RTBlasBuildPass::RequestBlasBuild( uint64_t mesh_id )
{
    mBLASQueue.push( mesh_id );
}

RTBlasBuildPass::RTBlasBuildPass()
{
    // todo: read from config
    auto &device = *DeviceGlobals::RenderHookDevice;

    mBlasCmdBuffer = dynamic_cast<rh::engine::VulkanCommandBuffer *>(
        device.CreateCommandBuffer() );
    mBlasBuilt =
        device.CreateSyncPrimitive( rh::engine::SyncPrimitiveType::GPU );
#ifdef _DEBUG
    rh::engine::VulkanDebugUtils::SetDebugName(
        mBlasCmdBuffer, std::string( "blas_build_cmd_buffer" ) );
    rh::engine::VulkanDebugUtils::SetDebugName(
        mBlasBuilt, std::string( "blas_build_finish_sp" ) );
#endif

    mBLASPool.resize( BackendMeshManager::SceneMeshData->GetSize() );

    BackendMeshManager::SceneMeshData->AddOnRequestCallback(
        [this]( BackendMeshData &data, uint64_t id ) {
            auto &device = dynamic_cast<rh::engine::VulkanDeviceState &>(
                *DeviceGlobals::RenderHookDevice );
            mBLASPool[id].mHasEntry = true;
            rh::engine::AccelerationStructureCreateInfo ac_ci{};
            ac_ci.mVertexBuffer       = data.mVertexBuffer->Get();
            ac_ci.mIndexBuffer        = data.mIndexBuffer->Get();
            ac_ci.mIndexCount         = data.mIndexCount;
            ac_ci.mVertexCount        = data.mVertexCount;
            ac_ci.mSplits             = { { 0, 0,
                                static_cast<uint32_t>( data.mVertexCount ),
                                static_cast<uint32_t>( data.mIndexCount ) } };
            mBLASPool[id].mData.mBLAS = device.CreateBLAS( ac_ci );
            // Add BLAS to build list
            RequestBlasBuild( id );
        } );
    BackendMeshManager::SceneMeshData->AddOnDestructCallback(
        [this]( BackendMeshData &data, uint64_t id ) {
            delete static_cast<
                rh::engine::VulkanBottomLevelAccelerationStructure *>(
                mBLASPool[id].mData.mBLAS );
            mBLASPool[id].mData.mBLAS      = nullptr;
            mBLASPool[id].mData.mBlasBuilt = false;
            mBLASPool[id].mHasEntry        = false;
        } );
}

void RTBlasBuildPass::Execute()
{
    using namespace rh::engine;
    mCompleted = false;
    if ( mBLASQueue.empty() )
        return;
    std::vector<VulkanBottomLevelAccelerationStructure *> blas_list{};
    blas_list.reserve( 50 );

    std::uint32_t scratch_buff_size{};
    auto          max_blas_count = 50;
    while ( max_blas_count > 0 && !mBLASQueue.empty() )
    {
        auto id = mBLASQueue.front();
        mBLASQueue.pop();
        auto &mesh_info = mBLASPool[id].mData;
        auto  blas = (VulkanBottomLevelAccelerationStructure *)mesh_info.mBLAS;
        if ( !blas )
            continue;
        scratch_buff_size = max( blas->GetScratchSize(), scratch_buff_size );
        blas_list.push_back( blas );
        mesh_info.mBlasBuilt = true;
        max_blas_count--;
    }

    if ( !mScratchBuffer || mScratchBufferSize < scratch_buff_size )
    {
        delete mScratchBuffer;
        mScratchBuffer = DeviceGlobals::RenderHookDevice->CreateBuffer(
            { scratch_buff_size, BufferUsage::RayTracingScratch,
              BufferFlags::Dynamic } );
        mScratchBufferSize = scratch_buff_size;
    }

    mBlasCmdBuffer->BeginRecord();
    for ( auto blas : blas_list )
    {
        mBlasCmdBuffer->BuildBLAS( blas, mScratchBuffer );

        MemoryBarrierInfo mem_barr{
            .mSrcMemoryAccess = MemoryAccessFlags::AccelerationStructureWrite,
            .mDstMemoryAccess = MemoryAccessFlags::AccelerationStructureRead };
        std::array barriers = { mem_barr };

        PipelineBarrierInfo barrierInfo{ PipelineStage::BuildAcceleration,
                                         PipelineStage::BuildAcceleration,
                                         barriers };
        mBlasCmdBuffer->PipelineBarrier( barrierInfo );
    }
    mBlasCmdBuffer->EndRecord();
    mCompleted = true;
}
rh::engine::CommandBufferSubmitInfo
RTBlasBuildPass::GetSubmitInfo( rh::engine::ISyncPrimitive *dependency )
{
    return { mBlasCmdBuffer,
             dependency ? std::vector{ dependency }
                        : std::vector<rh::engine::ISyncPrimitive *>{},
             mBlasBuilt };
}
RTBlasBuildPass::~RTBlasBuildPass()
{
    delete mScratchBuffer;
    delete mBlasCmdBuffer;
    delete mBlasBuilt;
}
bool RTBlasBuildPass::Completed() const { return mCompleted; }
} // namespace rh::rw::engine