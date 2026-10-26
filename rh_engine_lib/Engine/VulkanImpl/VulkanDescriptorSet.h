#pragma once
#include "Engine/Common/IDescriptorSet.h"
#include <common.h>

namespace rh::engine
{
struct VulkanDescriptorSetCreateParams
{
    // Dependencies...
    vk::Device         mDevice;
    vk::DescriptorPool mPool;
    // Params
    vk::DescriptorSet mDescSet;
};

class VulkanDescriptorSet final : public IDescriptorSet
{
  public:
    explicit VulkanDescriptorSet(
        const VulkanDescriptorSetCreateParams &create_params );
    ~VulkanDescriptorSet() override;
    DescriptorType GetType( uint32_t binding_id ) override;
    explicit       operator vk::DescriptorSet() const { return mDescSet; }

  private:
    vk::Device         mDevice;
    vk::DescriptorPool mPool;
    vk::DescriptorSet  mDescSet;
};
} // namespace rh::engine