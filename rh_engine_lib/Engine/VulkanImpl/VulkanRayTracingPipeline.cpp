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
    vk::RayTracingPipelineCreateInfoNV createInfoNv{};
    // TODO: allow to change
    createInfoNv.maxRecursionDepth =
        ( std::max )( 10u, create_info.mGPUInfo.mMaxRecursionDepth );
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{};
    shader_stages.reserve( create_info.mShaderStages.Size() );
    std::ranges::transform(
        create_info.mShaderStages, std::back_inserter( shader_stages ),
        []( const ShaderStageDesc &stage_desc ) {
            vk::PipelineShaderStageCreateInfo vk_desc{};
            vk_desc.stage = Convert( stage_desc.mStage );
            vk_desc.pName = stage_desc.mEntryPoint.c_str();
            vk_desc.module =
                *dynamic_cast<VulkanShader *>( stage_desc.mShader );
            return vk_desc;
        } );

    std::vector<vk::RayTracingShaderGroupCreateInfoNV> shader_groups{};
    shader_groups.reserve( create_info.mShaderGroups.Size() );

    std::ranges::transform(
        create_info.mShaderGroups, std::back_inserter( shader_groups ),
        []( const RayTracingGroup &group ) {
            vk::RayTracingShaderGroupCreateInfoNV vk_desc{};
            // TODO: BIX
            vk_desc.type =
                group.mType == RTShaderGroupType::General
                    ? vk::RayTracingShaderGroupTypeNV::eGeneral
                    : vk::RayTracingShaderGroupTypeNV::eTrianglesHitGroup;
            vk_desc.generalShader      = group.mGeneralId;
            vk_desc.anyHitShader       = group.mAnyHitId;
            vk_desc.closestHitShader   = group.mClosestHitId;
            vk_desc.intersectionShader = group.mIntersectionId;

            return vk_desc;
        } );

    createInfoNv.pStages    = shader_stages.data();
    createInfoNv.stageCount = static_cast<uint32_t>( shader_stages.size() );
    createInfoNv.pGroups    = shader_groups.data();
    createInfoNv.groupCount = static_cast<uint32_t>( shader_groups.size() );
    mGroupCount             = static_cast<uint32_t>( shader_groups.size() );
    createInfoNv.layout =
        *static_cast<VulkanPipelineLayout *>( create_info.mLayout );
    mPipelineLayout = createInfoNv.layout;
    mPipelineImpl = mDevice.createRayTracingPipelineNV( nullptr, createInfoNv );
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
    mDevice.getRayTracingShaderGroupHandlesNV( mPipelineImpl, 0, mGroupCount,
                                               data.size(), data.data() );
    return data;
}
uint32_t VulkanRayTracingPipeline::GetSBTHandleSize()
{
    return mGPUInfo.GetAlignedSGHandleSize();
}
uint32_t VulkanRayTracingPipeline::GetSBTHandleSizeUnalign()
{
    return mGPUInfo.mShaderGroupHandleSize;
}
} // namespace rh::engine