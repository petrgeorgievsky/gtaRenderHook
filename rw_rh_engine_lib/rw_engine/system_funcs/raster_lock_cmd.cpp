//
// Created by peter on 07.01.2021.
//

#include "raster_lock_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ScopedPtr.h>
#include <render_client/render_client.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/raster_backend.h>
namespace rh::rw::engine
{

RasterLockResult RasterLockCmdImpl::Invoke( const RasterLockParams &params )
{
    auto &           task_queue = gRenderClient->GetTaskQueue();
    RasterLockResult result{};

    task_queue.ExecuteTask(
        SharedMemoryTaskType::RASTER_LOCK,
        [params]( MemoryWriter &&writer ) { writer.Write( &params ); },
        [&result, &params]( MemoryReader &&memory_reader ) {
            // deserialize
            result = { *memory_reader.Read<RasterLockResultData>() };
            if ( params.mImageId != 0xBADF00D )
                result.mData = memory_reader.Read<uint8_t>(
                    result.mLockDataHeight * result.mLockDataStride );
        } );
    return result;
}

void RasterLockCallback( void *memory )
{
    // execute
    using namespace rh::engine;

    auto &device      = gRenderDriver->GetDeviceState();
    auto &resources   = gRenderDriver->GetResources();
    auto &raster_pool = resources.GetRasterPool();

    MemoryReader reader( memory );
    MemoryWriter writer( memory );

    RasterLockParams params = *reader.Read<RasterLockParams>();

    RasterLockResultData result{};
    result.mLockDataStride = 4 * params.mWidth;
    result.mLockDataHeight = params.mHeight;
    writer.Write( &result );
    auto &r = raster_pool.GetResource( params.mImageId );

    if ( ( params.mLockMode & RasterLockParams::LockRead ) != 0 )
    {
        ScopedPointer staging_buffer = device.CreateBuffer(
            { .mSize  = result.mLockDataStride * result.mLockDataHeight,
              .mUsage = BufferUsage::StagingBuffer } );
        ScopedPointer cmd_buffer = device.CreateCommandBuffer();

        cmd_buffer->BeginRecord();
        cmd_buffer->PipelineBarrier(
            { .mSrcStage            = PipelineStage::Host,
              .mDstStage            = PipelineStage::Transfer,
              .mImageMemoryBarriers = {
                  { .mImage           = r.mImageBuffer,
                    .mSrcLayout       = ImageLayout::ShaderReadOnly,
                    .mDstLayout       = ImageLayout::TransferSrc,
                    .mSrcMemoryAccess = MemoryAccessFlags::Unknown,
                    .mDstMemoryAccess = MemoryAccessFlags::MemoryWrite,
                    .mSubresRange     = { params.mMipLevel, 1, 0, 1 } } } } );

        // Copy image to CPU buffer
        cmd_buffer->CopyImageToBuffer(
            { .mBuffer      = staging_buffer,
              .mImage       = r.mImageBuffer,
              .mImageLayout = ImageLayout::TransferSrc,
              .mRegions     = {
                  { .mFrom = ImageRegion{ { .mipLevel       = params.mMipLevel,
                                            .baseArrayLayer = 0,
                                            .layerCount     = 1 },
                                          0,
                                          0,
                                          0,
                                          params.mWidth,
                                          params.mHeight,
                                          1 },
                    .mTo   = BufferRegion{ 0, 0, 0 } } } } );

        cmd_buffer->PipelineBarrier(
            { .mSrcStage            = PipelineStage::Transfer,
              .mDstStage            = PipelineStage::PixelShader,
              .mImageMemoryBarriers = {
                  { .mImage           = r.mImageBuffer,
                    .mSrcLayout       = ImageLayout::TransferSrc,
                    .mDstLayout       = ImageLayout::ShaderReadOnly,
                    .mSrcMemoryAccess = MemoryAccessFlags::MemoryWrite,
                    .mDstMemoryAccess = MemoryAccessFlags::MemoryRead,
                    .mSubresRange     = { params.mMipLevel, 1, 0, 1 } } } } );

        cmd_buffer->EndRecord();
        device.ExecuteCommandBuffer( cmd_buffer, nullptr, nullptr );
        device.Wait( { cmd_buffer->ExecutionFinishedPrimitive() } );

        auto data = reinterpret_cast<uint8_t *>( staging_buffer->Lock() );
        writer.Write( data, result.mLockDataStride * result.mLockDataHeight );
        staging_buffer->Unlock();
    }
}

void RasterLockCmdImpl::RegisterCallHandler( SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::RASTER_LOCK,
        std::make_unique<SharedMemoryTask>( RasterLockCallback ) );
}

} // namespace rh::rw::engine