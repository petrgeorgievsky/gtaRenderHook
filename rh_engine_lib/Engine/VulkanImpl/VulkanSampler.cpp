#include "VulkanSampler.h"

#include "Engine/Common/types/sampler_addressing.h"
#include "Engine/Common/types/sampler_filter.h"
#include <DebugUtils/DebugLogger.h>

using namespace rh::engine;

VulkanSampler::VulkanSampler( const VulkanSamplerDesc &desc )
    : mDevice( desc.mDevice )
{
    vk::SamplerCreateInfo create_info{};
    create_info.anisotropyEnable = false;
    switch ( desc.mInfo.filtering )
    {
    case SamplerFilter::Unknown:
    case SamplerFilter::Point:
        create_info.magFilter = vk::Filter::eNearest;
        create_info.minFilter = vk::Filter::eNearest;
        break;
    case SamplerFilter::Linear:
        create_info.magFilter = vk::Filter::eLinear;
        create_info.minFilter = vk::Filter::eLinear;
        break;
    case SamplerFilter::Anisotropic:
        create_info.magFilter        = vk::Filter::eLinear;
        create_info.minFilter        = vk::Filter::eLinear;
        create_info.anisotropyEnable = true;
        break;
    }
    create_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
    switch ( desc.mInfo.adressU )
    {
    case SamplerAddressing::Unknown:
    case SamplerAddressing::Wrap:
        create_info.addressModeU = vk::SamplerAddressMode::eRepeat;
        break;
    case SamplerAddressing::Clamp:
        create_info.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        break;
    case SamplerAddressing::Mirror:
        create_info.addressModeU = vk::SamplerAddressMode::eMirroredRepeat;
        break;
    case SamplerAddressing::Border:
        create_info.addressModeU = vk::SamplerAddressMode::eClampToBorder;
        break;
    }
    switch ( desc.mInfo.adressV )
    {
    case SamplerAddressing::Unknown:
    case SamplerAddressing::Wrap:
        create_info.addressModeV = vk::SamplerAddressMode::eRepeat;
        break;
    case SamplerAddressing::Clamp:
        create_info.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        break;
    case SamplerAddressing::Mirror:
        create_info.addressModeV = vk::SamplerAddressMode::eMirroredRepeat;
        break;
    case SamplerAddressing::Border:
        create_info.addressModeV = vk::SamplerAddressMode::eClampToBorder;
        break;
    }
    create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
    create_info.unnormalizedCoordinates = false;
    create_info.compareEnable           = false;
    create_info.maxAnisotropy           = 16.0f;
    create_info.maxLod                  = 1000.0f;
    auto result                         = mDevice.createSampler( create_info );
    if ( result.result != vk::Result::eSuccess )
    {
        debug::DebugLogger::ErrorFmt( "Failed to create sampler:%s",
                                      vk::to_string( result.result ).c_str() );
    }
    else
        mSamplerImpl = result.value;
}

VulkanSampler::~VulkanSampler() { mDevice.destroySampler( mSamplerImpl ); }
