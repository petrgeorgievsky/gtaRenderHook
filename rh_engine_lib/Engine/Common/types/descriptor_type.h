#pragma once
namespace rh::engine
{
enum class DescriptorType
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
