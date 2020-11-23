#include "VulkanSampler.h"
#include "Engine/Common/types/sampler_filter.h"

using namespace rh::engine;

VulkanSampler::VulkanSampler( const VulkanSamplerDesc &desc )
    : mDevice( desc.mDevice )
{
    vk::SamplerCreateInfo create_info{};
    create_info.anisotropyEnable = false;
    switch ( desc.mInfo.filtering )
    {
    case SamplerFilter::Unknown:
        create_info.magFilter = vk::Filter::eNearest;
        create_info.minFilter = vk::Filter::eNearest;
        break;
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
    create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
    create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
    create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
    create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
    create_info.unnormalizedCoordinates = false;
    create_info.compareEnable           = false;
    create_info.maxAnisotropy           = 16.0f;
    create_info.maxLod                  = 1000.0f;
    mSamplerImpl                        = mDevice.createSampler( create_info );
}

VulkanSampler::~VulkanSampler() { mDevice.destroySampler( mSamplerImpl ); }
