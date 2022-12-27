//
// Created by peter on 02.05.2020.
//

#include "VulkanRayTracingPipeline.h"
#include "VulkanConvert.h"
#include "VulkanPipelineLayout.h"
#include "VulkanShader.h"
namespace rh::engine
{

VulkanRayTracingPipeline::VulkanRayTracingPipeline(
    const VulkanRayTracingPipelineCreateInfo &create_info )
    : mDevice( create_info.mDevice ), mGPUInfo( create_info.mGPUInfo )
{
    vk::RayTracingPipelineCreateInfoKHR createInfo{};
    // TODO: allow to change
    createInfo.maxPipelineRayRecursionDepth =
        ( std::max )( 10u, create_info.mGPUInfo.mMaxRecursionDepth );
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{};
    shader_stages.reserve( create_info.mShaderStages.Size() );
    std::ranges::transform(
        create_info.mShaderStages, std::back_inserter( shader_stages ),
        []( const ShaderStageDesc &stage_desc )
        {
            vk::PipelineShaderStageCreateInfo vk_desc{};
            vk_desc.stage = Convert( stage_desc.mStage );
            vk_desc.pName = stage_desc.mEntryPoint.c_str();
            vk_desc.module =
                *dynamic_cast<VulkanShader *>( stage_desc.mShader );
            return vk_desc;
        } );

    std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shader_groups{};
    shader_groups.reserve( create_info.mShaderGroups.Size() );

    std::ranges::transform(
        create_info.mShaderGroups, std::back_inserter( shader_groups ),
        []( const RayTracingGroup &group )
        {
            vk::RayTracingShaderGroupCreateInfoKHR vk_desc{};
            // TODO: BIX
            vk_desc.type =
                group.mType == RTShaderGroupType::General
                    ? vk::RayTracingShaderGroupTypeKHR::eGeneral
                    : vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
            vk_desc.generalShader      = group.mGeneralId;
            vk_desc.anyHitShader       = group.mAnyHitId;
            vk_desc.closestHitShader   = group.mClosestHitId;
            vk_desc.intersectionShader = group.mIntersectionId;

            return vk_desc;
        } );

    createInfo.pStages    = shader_stages.data();
    createInfo.stageCount = static_cast<uint32_t>( shader_stages.size() );
    createInfo.pGroups    = shader_groups.data();
    createInfo.groupCount = static_cast<uint32_t>( shader_groups.size() );
    mGroupCount           = static_cast<uint32_t>( shader_groups.size() );
    createInfo.layout =
        *static_cast<VulkanPipelineLayout *>( create_info.mLayout );
    mPipelineLayout = createInfo.layout;
    mPipelineImpl =
        mDevice.createRayTracingPipelineKHR( nullptr, nullptr, createInfo )
            .value;
}

VulkanRayTracingPipeline::~VulkanRayTracingPipeline()
{
    mDevice.destroyPipeline( mPipelineImpl );
}

std::vector<uint8_t> VulkanRayTracingPipeline::GetShaderBindingTable()
{
    std::vector<uint8_t> data{};
    auto aligned_handle_size = mGPUInfo.GetAlignedSGHandleSize();
    data.resize( mGroupCount * aligned_handle_size );

    (void)VULKAN_HPP_DEFAULT_DISPATCHER.vkGetRayTracingShaderGroupHandlesKHR(
        mDevice, mPipelineImpl, 0, mGroupCount, (uint32_t)data.size(),
        (void *)data.data() );
    return data;
}
uint32_t VulkanRayTracingPipeline::GetSBTHandleSize()
{
    return mGPUInfo.GetAlignedSGHandleSize();
}
uint32_t VulkanRayTracingPipeline::GetSBTHandleSizeUnalign() const
{
    return mGPUInfo.mShaderGroupHandleSize;
}
} // namespace rh::engine