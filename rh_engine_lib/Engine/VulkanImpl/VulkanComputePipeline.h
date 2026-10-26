//
// Created by peter on 12.05.2020.
//
#pragma once
#include <Engine/Common/IPipeline.h>
#include <common.h>

namespace rh::engine
{

struct ComputePipelineCreateParams
{
    IPipelineLayout *mLayout;
    ShaderStageDesc  mShaderStage;
};

struct VulkanComputePipelineCreateInfo : ComputePipelineCreateParams
{
    // Dependencies...
    vk::Device mDevice;
};

class VulkanComputePipeline
{
  public:
    explicit VulkanComputePipeline( const VulkanComputePipelineCreateInfo &create_info );
    ~VulkanComputePipeline();
    vk::Pipeline GetImpl() const { return mPipelineImpl; }

  private:
    vk::Pipeline mPipelineImpl;
    vk::Device   mDevice;
};

} // namespace rh::engine