#pragma once
#include "Engine/Common/IPipelineLayout.h"
#include "VulkanDescriptorSetLayout.h"
#include <common.h>

namespace rh::engine
{

struct VulkanPipelineLayoutCreateInfo
{
    // Dependencies...
    vk::Device     mDevice;
    // Params
    ArrayProxy<IDescriptorSetLayout *> mDescriptorSetLayouts;
};

class VulkanPipelineLayout: public IPipelineLayout
{
  private:
    vk::PipelineLayout mPipelineLayoutImpl;
    vk::Device         mDevice;
  public:
    VulkanPipelineLayout( const VulkanPipelineLayoutCreateInfo &create_info );
    ~VulkanPipelineLayout();

    operator vk::PipelineLayout() { return mPipelineLayoutImpl; }
};
} // namespace rh::engine