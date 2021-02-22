//
// Created by peter on 27.06.2020.
//

#include "utils.h"
#include <Engine/Common/IDeviceState.h>
#include <TestUtils/BitmapLoader.h>
#include <render_driver/gpu_resources/raster_pool.h>
#include <render_driver/render_driver.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;
ImageMemoryBarrierInfo GetLayoutTransformBarrier( IImageBuffer *buffer,
                                                  ImageLayout   src,
                                                  ImageLayout   dst )
{
    return { .mImage           = buffer,
             .mSrcLayout       = src,
             .mDstLayout       = dst,
             .mSrcMemoryAccess = MemoryAccessFlags::Unknown,
             .mDstMemoryAccess = MemoryAccessFlags::MemoryWrite,
             .mSubresRange     = { 0, 1, 0, 1 } };
}
IImageBuffer *Create2DRenderTargetBuffer( rh::engine::IDeviceState &device,
                                          uint32_t w, uint32_t h,
                                          ImageBufferFormat f, uint32_t usage )
{
    using namespace rh::engine;
    return device.CreateImageBuffer( { .mDimension = ImageDimensions::d2D,
                                       .mFormat    = f,
                                       .mUsage     = usage,
                                       .mHeight    = h,
                                       .mWidth     = w } );
}

RasterData ReadBMP( const std::string &path )
{
    RasterData result_data{};

    rh::tests::LoadBMPImage( path, gRenderDriver->GetDeviceState(),
                             &result_data.mImageBuffer );

    rh::engine::ImageViewCreateInfo shader_view_ci{};
    shader_view_ci.mBuffer = result_data.mImageBuffer;
    shader_view_ci.mFormat = rh::engine::ImageBufferFormat::RGBA8;
    shader_view_ci.mUsage  = rh::engine::ImageViewUsage::ShaderResource;

    result_data.mImageView =
        gRenderDriver->GetDeviceState().CreateImageView( shader_view_ci );

    return result_data;
}

} // namespace rh::rw::engine