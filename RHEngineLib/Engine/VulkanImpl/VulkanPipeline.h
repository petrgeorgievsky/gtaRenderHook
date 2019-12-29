#pragma once
#include "Engine/Common/IPipeline.h"
#include <common.h>

namespace rh::engine
{

struct VulkanPipelineCreateInfo
{
    // Dependencies...
    vk::Device     mDevice;
    vk::RenderPass mRenderPass;
    // Params

    std::vector<rh::engine::ShaderStageDesc> mShaderStages;
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