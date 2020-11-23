//
// Created by peter on 02.08.2020.
//
#pragma once
#include <string_view>

namespace rh::engine
{
class IImageView;
class IImageBuffer;
class IDeviceState;
} // namespace rh::engine

namespace rh::tests
{
void LoadBMPImage( std::string_view path, engine::IDeviceState &device,
                   engine::IImageBuffer **res_buffer );
}