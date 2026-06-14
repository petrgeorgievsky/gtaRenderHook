#pragma once
#include <cstdint>

namespace rh::engine
{

enum class DescriptorType : uint8_t
{
    Sampler,
    ROBuffer,
    ROTexture,
    RWTexture,
    RWBuffer,
    RTAccelerationStruct,
    StorageTexture
};

} // namespace rh::engine
