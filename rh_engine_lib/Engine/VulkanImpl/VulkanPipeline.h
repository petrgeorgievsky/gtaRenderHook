#pragma once
#include "Engine/Common/IPipeline.h"
#include <common.h>

namespace rh::engine
{

struct VulkanPipelineCreateInfo : RasterPipelineCreateParams
{
    // Dependencies...
    vk::Device     mDevice;
};

class VulkanPipeline : public IPipeline
{
  public:
    VulkanPipeline( const VulkanPipelineCreateInfo &create_info );
    ~VulkanPipeline() override;

    operator vk::Pipeline() { return mPipelineImpl; }

  private:
    vk::Pipeline       mPipelineImpl;
    vk::PipelineLayout mPipelineLayout;
    vk::Device         mDevice;
};
} // namespace rh::engine