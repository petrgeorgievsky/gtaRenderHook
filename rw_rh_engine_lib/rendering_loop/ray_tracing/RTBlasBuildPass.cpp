//
// Created by peter on 26.06.2020.
//

#include "RTBlasBuildPass.h"
#include <Engine/VulkanImpl/VulkanBottomLevelAccelerationStructure.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDebugUtils.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <render_driver/gpu_resources/resource_mgr.h>

#include <memory_resource>

namespace rh::rw::engine
{

constexpr uint64_t MeshPoolCallbackId = 0x4;

void RTBlasBuildPass::RequestBlasBuild( uint64_t mesh_id )
{
    BLASQueue.push( mesh_id );
}

RTBlasBuildPass::RTBlasBuildPass( const BlasBuildPassCreateInfo &info )
    : Device( info.Device ), Resources( info.Resources )
{
    using namespace rh::engine;
    BlasCmdBuffer =
        dynamic_cast<VulkanCommandBuffer *>( Device.CreateCommandBuffer() );
    BlasBuilt = Device.CreateSyncPrimitive( SyncPrimitiveType::GPU );
#ifdef _DEBUG
    rh::engine::VulkanDebugUtils::SetDebugName(
        BlasCmdBuffer, std::string( "blas_build_cmd_buffer" ) );
    rh::engine::VulkanDebugUtils::SetDebugName(
        BlasBuilt, std::string( "blas_build_finish_sp" ) );
#endif

    auto &mesh_pool = Resources.GetMeshPool();

    BLASPool.resize( mesh_pool.GetSize() );
    ScratchBuffers.resize( info.ScratchBufferCount );
    for ( auto &scratch_buffer : ScratchBuffers )
    {
        scratch_buffer.Data = Device.CreateBuffer(
            { info.ScratchBufferBaseSize, BufferUsage::RayTracingScratch,
              BufferFlags::DynamicGPUOnly } );
        scratch_buffer.Size = info.ScratchBufferBaseSize;
    }

    ScratchBufferCount = info.ScratchBufferCount;

    mesh_pool.AddOnRequestCallback(
        [this]( BackendMeshData &data, uint64_t id )
        {
            auto &device =
                dynamic_cast<rh::engine::VulkanDeviceState &>( Device );
            BLASPool[id].mHasEntry = true;
            rh::engine::AccelerationStructureCreateInfo ac_ci{};
            ac_ci.mVertexBuffer      = data.mVertexBuffer->Get();
            ac_ci.mIndexBuffer       = data.mIndexBuffer->Get();
            ac_ci.mIndexCount        = data.mIndexCount;
            ac_ci.mVertexCount       = data.mVertexCount;
            ac_ci.mSplits            = { { 0, 0,
                                static_cast<uint32_t>( data.mVertexCount ),
                                static_cast<uint32_t>( data.mIndexCount ) } };
            BLASPool[id].mData.mBLAS = device.CreateBLAS( ac_ci );
            // Add BLAS to build list
            RequestBlasBuild( id );
        },
        MeshPoolCallbackId );
    mesh_pool.AddOnDestructCallback(
        [this]( BackendMeshData &data, uint64_t id )
        {
            delete static_cast<
                rh::engine::VulkanBottomLevelAccelerationStructure *>(
                BLASPool[id].mData.mBLAS );
            BLASPool[id].mData.mBLAS      = nullptr;
            BLASPool[id].mData.mBlasBuilt = false;
            BLASPool[id].mHasEntry        = false;
        },
        MeshPoolCallbackId );
}

void RTBlasBuildPass::Execute()
{
    using namespace rh::engine;
    IsCompleted = false;
    if ( BLASQueue.empty() )
        return;
    using BlasBuildTask = std::pmr::vector<BlasBuildInfo>;

    auto buffer_size =
        sizeof( BlasBuildInfo ) * ScratchBufferCount + sizeof( BlasBuildTask );
    std::unique_ptr<char[]> memory_pool{ new char[buffer_size] };

    std::pmr::monotonic_buffer_resource buffer_resource{ memory_pool.get(),
                                                         buffer_size };

    std::pmr::vector<BlasBuildTask> blas_list{ &buffer_resource };
    blas_list.reserve( 25 );

    std::vector<uint64_t> scratch_buff_size{ ScratchBufferCount, 0u };
    auto                  max_blas_count  = 50;
    auto                  current_scratch = 0;
    blas_list.push_back( {} );
    while ( max_blas_count > 0 && !BLASQueue.empty() )
    {
        auto id = BLASQueue.front();
        BLASQueue.pop();
        auto &mesh_info = BLASPool[id].mData;
        auto  blas = (VulkanBottomLevelAccelerationStructure *)mesh_info.mBLAS;
        if ( !blas )
            continue;

        scratch_buff_size[current_scratch] = ( std::max )(
            blas->GetScratchSize(), scratch_buff_size[current_scratch] );
        blas_list.back().push_back(
            BlasBuildInfo{ blas, ScratchBuffers[current_scratch].Data } );
        mesh_info.mBlasBuilt = true;
        if ( current_scratch < ScratchBufferCount - 1 )
            current_scratch++;
        else
        {
            blas_list.push_back( {} );
            current_scratch = 0;
        }
        max_blas_count--;
    }
    if ( blas_list.back().empty() )
        blas_list.pop_back();

    for ( auto i = 0; i < ScratchBufferCount; i++ )
    {
        if ( ScratchBuffers[i].Size < scratch_buff_size[i] )
        {
            delete ScratchBuffers[i].Data;
            ScratchBuffers[i].Data = Device.CreateBuffer(
                { static_cast<uint32_t>( scratch_buff_size[i] ),
                  BufferUsage::RayTracingScratch,
                  BufferFlags::DynamicGPUOnly } );
            ScratchBuffers[i].Size = scratch_buff_size[i];
        }
        for ( auto &blas_task : blas_list )
        {
            if ( i < blas_task.size() )
                blas_task.at( i ).TempBuffer = ScratchBuffers[i].Data;
        }
    }

    BlasCmdBuffer->BeginRecord();

    for ( const auto &blas_task : blas_list )
    {
        BlasCmdBuffer->BuildBLAS(
            ArrayProxy{ blas_task.data(), blas_task.size() } );
        MemoryBarrierInfo mem_barr{
            .mSrcMemoryAccess = MemoryAccessFlags::AccelerationStructureWrite,
            .mDstMemoryAccess = MemoryAccessFlags::AccelerationStructureRead };
        std::array barriers = { mem_barr };

        PipelineBarrierInfo barrierInfo{ PipelineStage::BuildAcceleration,
                                         PipelineStage::BuildAcceleration,
                                         barriers };
        BlasCmdBuffer->PipelineBarrier( barrierInfo );
    }
    BlasCmdBuffer->EndRecord();
    IsCompleted = true;
}
rh::engine::CommandBufferSubmitInfo
RTBlasBuildPass::GetSubmitInfo( rh::engine::ISyncPrimitive *dependency )
{
    return { BlasCmdBuffer,
             dependency ? std::vector{ dependency }
                        : std::vector<rh::engine::ISyncPrimitive *>{},
             BlasBuilt };
}
RTBlasBuildPass::~RTBlasBuildPass()
{
    auto &mesh_pool = Resources.GetMeshPool();
    mesh_pool.RemoveOnRequestCallback( MeshPoolCallbackId );
    mesh_pool.RemoveOnDestructCallback( MeshPoolCallbackId );
    // Cleanup blas pool
    for ( auto blas : BLASPool )
    {
        delete static_cast<
            rh::engine::VulkanBottomLevelAccelerationStructure *>(
            blas.mData.mBLAS );
        blas.mData.mBLAS      = nullptr;
        blas.mData.mBlasBuilt = false;
        blas.mHasEntry        = false;
    }
    delete BlasCmdBuffer;
    delete BlasBuilt;
}
bool RTBlasBuildPass::Completed() const { return IsCompleted; }
} // namespace rh::rw::engine