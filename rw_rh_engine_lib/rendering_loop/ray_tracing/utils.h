//
// Created by peter on 27.06.2020.
//
#pragma once
#include "Engine/Common/types/image_buffer_format.h"
#include "Engine/Common/types/image_layout.h"
#include <Engine/Common/IImageBuffer.h>
#include <cstdint>
#include <string>

namespace rh::engine
{
class IImageBuffer;
class IDeviceState;
struct ImageMemoryBarrierInfo;
} // namespace rh::engine
namespace rh::rw::engine
{
struct RasterData;

rh::engine::IImageBuffer *Create2DRenderTargetBuffer(
    rh::engine::IDeviceState &device, uint32_t w, uint32_t h,
    rh::engine::ImageBufferFormat f,
    uint32_t usage = rh::engine::ImageBufferUsage::Storage |
                     rh::engine::ImageBufferUsage::Sampled |
                     rh::engine::ImageBufferUsage::TransferDst |
                     rh::engine::ImageBufferUsage::TransferSrc );

rh::engine::ImageMemoryBarrierInfo
GetLayoutTransformBarrier( rh::engine::IImageBuffer *buffer,
                           rh::engine::ImageLayout   src,
                           rh::engine::ImageLayout   dst );

RasterData ReadBMP( const std::string &path );
} // namespace rh::rw::engine