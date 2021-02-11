//
// Created by peter on 10.02.2021.
//

#include "raster_load_cmd.h"
#include "rw_device_system_globals.h"
#include <Engine/Common/IDeviceState.h>
#include <ipc/shared_memory_queue_client.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{
RasterLoadCmdImpl::RasterLoadCmdImpl( SharedMemoryTaskQueue &task_queue )
    : TaskQueue( task_queue )
{
}

int64_t RasterLoadCmdImpl::Invoke( const RasterHeader &header,
                                   WriteMipLevelFunc   write_mip_level )
{
    int64_t result_raster = 0xBADF00D;
    TaskQueue.ExecuteTask(
        SharedMemoryTaskType::RASTER_LOAD,
        [&header, &write_mip_level]( MemoryWriter &&writer ) {
            // serialize
            writer.Write( &header );
            for ( uint32_t i = 0; i < header.mMipLevelCount; i++ )
                write_mip_level( writer, writer.Current<MipLevelHeader>() );
        },
        [&result_raster]( MemoryReader &&memory_reader ) {
            // deserialize
            result_raster = *memory_reader.Read<int64_t>();
        } );
    return result_raster;
}

void LoadTextureTaskImpl( void *memory )
{
    using namespace rh::engine;
    assert( gRenderDriver );
    auto &driver = *gRenderDriver;

    uint64_t     raster_id = 0;
    MemoryWriter writer( memory );
    MemoryReader reader( memory );

    auto header = *reader.Read<RasterHeader>();

    std::vector<ImageBufferInitData> buffer_init_data;
    buffer_init_data.reserve( header.mMipLevelCount );

    uint32_t non_zero_mip_lvl = 0;

    for ( uint32_t i = 0; i < header.mMipLevelCount; i++ )
    {
        auto mip_level_header = *reader.Read<MipLevelHeader>();

        ImageBufferInitData mip_data{};
        mip_data.mSize   = mip_level_header.mSize;
        mip_data.mStride = mip_level_header.mStride;
        mip_data.mData   = reader.Read<char>( mip_level_header.mSize );

        buffer_init_data.push_back( mip_data );
        if ( mip_data.mSize != 0 )
            non_zero_mip_lvl++;
    }

    auto format = static_cast<ImageBufferFormat>( header.mFormat );
    ImageBufferCreateParams image_buffer_ci{ .mDimension = ImageDimensions::d2D,
                                             .mFormat    = format,
                                             .mHeight    = header.mHeight,
                                             .mWidth     = header.mWidth,
                                             .mDepth     = header.mDepth,
                                             .mMipLevels = non_zero_mip_lvl,
                                             .mPreinitData = buffer_init_data };

    auto &device      = driver.GetDeviceState();
    auto &resources   = driver.GetResources();
    auto &raster_pool = resources.GetRasterPool();

    RasterData result_data{};
    result_data.mImageBuffer = device.CreateImageBuffer( image_buffer_ci );

    ImageViewCreateInfo shader_view_ci{ .mBuffer = result_data.mImageBuffer,
                                        .mFormat = format,
                                        .mUsage =
                                            ImageViewUsage::ShaderResource,
                                        .mLevelCount = non_zero_mip_lvl };
    result_data.mImageView = device.CreateImageView( shader_view_ci );

    raster_id = raster_pool.RequestResource( result_data );
    writer.Write( &raster_id );
}

void RasterLoadCmdImpl::RegisterCallHandler( SharedMemoryTaskQueue &task_queue )
{
    task_queue.RegisterTask(
        SharedMemoryTaskType::RASTER_LOAD,
        std::make_unique<SharedMemoryTask>( LoadTextureTaskImpl ) );
}
} // namespace rh::rw::engine