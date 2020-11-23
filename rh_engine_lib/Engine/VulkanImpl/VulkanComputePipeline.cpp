//
// Created by peter on 12.05.2020.
//

#include "VulkanComputePipeline.h"
#include "VulkanCommon.h"
#include "VulkanConvert.h"
#include "VulkanPipelineLayout.h"
#include "VulkanShader.h"

namespace rh::engine
{
VulkanComputePipeline::VulkanComputePipeline(
    const VulkanComputePipelineCreateInfo &create_info )
    : mDevice( create_info.mDevice )
{
    vk::ComputePipelineCreateInfo vk_ci{};

    vk_ci.layout = *static_cast<VulkanPipelineLayout *>( create_info.mLayout );
    vk::PipelineShaderStageCreateInfo vk_desc{};
    vk_desc.stage = Convert( create_info.mShaderStage.mStage );
    vk_desc.pName = create_info.mShaderStage.mEntryPoint.c_str();
    vk_desc.module =
        *dynamic_cast<VulkanShader *>( create_info.mShaderStage.mShader );
    vk_ci.stage = vk_desc;

    mPipelineImpl = mDevice.createComputePipeline( nullptr, vk_ci );
}
VulkanComputePipeline::~VulkanComputePipeline()
{
    mDevice.destroyPipeline( mPipelineImpl );
}
} // namespace rh::engine