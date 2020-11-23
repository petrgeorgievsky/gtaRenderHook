#pragma once
#include "Engine/Common/ISampler.h"
#include <common.h>

namespace rh::engine
{

struct VulkanSamplerDesc : SamplerDesc
{
    // Dependencies...
    vk::Device mDevice;
};

class VulkanSampler : public ISampler
{
  public:
    VulkanSampler( const VulkanSamplerDesc &desc );
    ~VulkanSampler() override;

    operator vk::Sampler() { return mSamplerImpl; }

  private:
    vk::Sampler mSamplerImpl;
    vk::Device  mDevice;
};

} // namespace rh::engine
