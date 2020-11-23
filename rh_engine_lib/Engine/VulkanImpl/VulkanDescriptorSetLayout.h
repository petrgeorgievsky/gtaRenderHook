#pragma once
#include "Engine/Common/IDescriptorSetLayout.h"
#include "Engine/Common/ArrayProxy.h"
#include <common.h>

namespace rh::engine
{


struct VulkanDescriptorSetLayoutCreateInfo
{
    // Dependencies...
    vk::Device mDevice;
    // Params
    ArrayProxy<DescriptorBinding> mBindingList;
};

class VulkanDescriptorSetLayout : public IDescriptorSetLayout
{
  public:
    VulkanDescriptorSetLayout(
        const VulkanDescriptorSetLayoutCreateInfo &create_info );
    ~VulkanDescriptorSetLayout() override;

    operator vk::DescriptorSetLayout() { return mDescSetLayoutImpl; }
    vk::DescriptorSetLayout GetDescSet() const { return mDescSetLayoutImpl; }

  private:
    vk::DescriptorSetLayout mDescSetLayoutImpl;
    vk::Device         mDevice;
};
} // namespace rh::engine