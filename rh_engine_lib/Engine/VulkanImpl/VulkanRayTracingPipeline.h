//
// Created by peter on 02.05.2020.
//

#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <Engine/Common/IPipeline.h>
#include <common.h>

namespace rh::engine
{
enum class RTShaderGroupType
{
    General,
    TriangleHitGroup,
    ProceduralGeometry
};
struct RayTracingGroup
{
    RTShaderGroupType mType;
    uint32_t          mGeneralId      = ~0u;
    uint32_t          mAnyHitId       = ~0u;
    uint32_t          mClosestHitId   = ~0u;
    uint32_t          mIntersectionId = ~0u;
};

struct RayTracingPipelineCreateInfo
{
    IPipelineLayout *           mLayout;
    ArrayProxy<ShaderStageDesc> mShaderStages;
    ArrayProxy<RayTracingGroup> mShaderGroups;
};

struct VulkanRayTracingPipelineCreateInfo : RayTracingPipelineCreateInfo
{
    // Dependencies...
    vk::Device mDevice;
};

class VulkanRayTracingPipeline
{
  public:
    VulkanRayTracingPipeline(
        const VulkanRayTracingPipelineCreateInfo &create_info );
    ~VulkanRayTracingPipeline();
    std::vector<uint8_t> GetShaderBindingTable();

    operator vk::Pipeline() { return mPipelineImpl; }

  private:
    vk::Pipeline       mPipelineImpl;
    vk::PipelineLayout mPipelineLayout;
    vk::Device         mDevice;
    uint32_t           mGroupCount;
};
} // namespace rh::engine