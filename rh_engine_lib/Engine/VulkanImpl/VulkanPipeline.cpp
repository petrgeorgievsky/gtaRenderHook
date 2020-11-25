#include "VulkanPipeline.h"
#include "VulkanConvert.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include <Engine/Common/types/blend_state.h>
#include <Engine/Common/types/comparison_func.h>
#include <Engine/Common/types/depth_stencil_state.h>
#include <memory_resource>
#include <span>

using namespace rh::engine;
namespace rh::engine
{
vk::PipelineColorBlendAttachmentState
Convert( const AttachmentBlendState &state )
{
    vk::PipelineColorBlendAttachmentState res{};
    res.blendEnable         = state.enableBlending;
    res.srcColorBlendFactor = Convert( state.srcBlend );
    res.dstColorBlendFactor = Convert( state.destBlend );
    res.srcAlphaBlendFactor = Convert( state.srcBlendAlpha );
    res.dstAlphaBlendFactor = Convert( state.destBlendAlpha );

    res.colorBlendOp = vk::BlendOp::eAdd;
    res.alphaBlendOp = vk::BlendOp::eAdd;
    res.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    return res;
}

vk::PipelineColorBlendStateCreateInfo
Convert( const BlendState &                                  state,
         std::vector<vk::PipelineColorBlendAttachmentState> &state_store )
{
    vk::PipelineColorBlendStateCreateInfo res{};
    // TODO: maybe allow such stuff, but it's available only on vulkan
    // afaik(nope, d3d11 too, but it's weird there)
    res.logicOpEnable = false;
    for ( int i = 0; i < 4; i++ )
        res.blendConstants[i] = state.blendConstants[i];
    for ( auto s : state.renderTargetBlendState )
        state_store.push_back( Convert( s ) );
    res.pAttachments    = state_store.data();
    res.attachmentCount = static_cast<uint32_t>( state_store.size() );

    return res;
}
constexpr vk::CompareOp Convert( ComparisonFunc func )
{
    switch ( func )
    {
    case ComparisonFunc::Unknown: return vk::CompareOp::eNever;
    case ComparisonFunc::Never: return vk::CompareOp::eNever;
    case ComparisonFunc::Less: return vk::CompareOp::eLess;
    case ComparisonFunc::Equal: return vk::CompareOp::eEqual;
    case ComparisonFunc::LessEqual: return vk::CompareOp::eLessOrEqual;
    case ComparisonFunc::Greater: return vk::CompareOp::eGreater;
    case ComparisonFunc::NotEqual: return vk::CompareOp::eNotEqual;
    case ComparisonFunc::GreaterEqual: return vk::CompareOp::eGreaterOrEqual;
    case ComparisonFunc::Always: return vk::CompareOp::eAlways;
    }
    return vk::CompareOp::eNever;
}
constexpr vk::PipelineDepthStencilStateCreateInfo
Convert( const DepthStencilState &state )
{
    vk::PipelineDepthStencilStateCreateInfo res{};
    res.depthTestEnable   = state.enableDepthBuffer;
    res.depthWriteEnable  = state.enableDepthWrite;
    res.stencilTestEnable = state.enableStencilBuffer;
    // TODO: Fix
    res.minDepthBounds = 0.0f;
    res.maxDepthBounds = 1.0f;
    res.depthCompareOp = Convert( state.depthComparisonFunc );
    return res;
}
} // namespace rh::engine

VulkanPipeline::VulkanPipeline( const VulkanPipelineCreateInfo &create_info )
    : mDevice( create_info.mDevice )
{
    using namespace std;
    array<uint8_t, 1024 * 8>       stack_buffer{};
    pmr::monotonic_buffer_resource fast_storage( stack_buffer.data(),
                                                 stack_buffer.size() );

    vk::GraphicsPipelineCreateInfo vk_create_info{};
    vk_create_info.renderPass =
        *dynamic_cast<VulkanRenderPass *>( create_info.mRenderPass );

    pmr::vector<vk::PipelineShaderStageCreateInfo> shader_stages(
        &fast_storage );
    shader_stages.reserve( create_info.mShaderStages.size() );

    ranges::transform(
        create_info.mShaderStages, back_inserter( shader_stages ),
        []( const ShaderStageDesc &stage_desc ) {
            vk::PipelineShaderStageCreateInfo vk_desc{};
            vk_desc.stage = Convert( stage_desc.mStage );
            vk_desc.pName = stage_desc.mEntryPoint.c_str();
            vk_desc.module =
                *dynamic_cast<VulkanShader *>( stage_desc.mShader );
            return vk_desc;
        } );

    vk_create_info.pStages    = shader_stages.data();
    vk_create_info.stageCount = static_cast<uint32_t>( shader_stages.size() );

    // Vertex Input State initialization

    pmr::vector<vk::VertexInputAttributeDescription> input_attributes(
        &fast_storage );
    input_attributes.reserve(
        create_info.mVertexInputStateDesc.mVertexInputLayout.Size() );
    ranges::transform( create_info.mVertexInputStateDesc.mVertexInputLayout,
                       back_inserter( input_attributes ),
                       []( const VertexInputElementDesc &element_desc ) {
                           vk::VertexInputAttributeDescription vk_desc{};
                           vk_desc.binding  = element_desc.mBinding;
                           vk_desc.location = element_desc.mLocation;
                           vk_desc.format   = Convert( element_desc.mFormat );
                           vk_desc.offset   = element_desc.mOffset;
                           return vk_desc;
                       } );

    pmr::vector<vk::VertexInputBindingDescription> binding_desc(
        &fast_storage );
    binding_desc.reserve(
        create_info.mVertexInputStateDesc.mVertexBindingLayout.Size() );
    ranges::transform( create_info.mVertexInputStateDesc.mVertexBindingLayout,
                       back_inserter( binding_desc ),
                       []( const VertexBindingDesc &element_desc ) {
                           vk::VertexInputBindingDescription vk_desc{};
                           vk_desc.binding = element_desc.mBinding;
                           vk_desc.inputRate =
                               Convert( element_desc.mInputRate );
                           vk_desc.stride = element_desc.mStride;
                           return vk_desc;
                       } );

    vk::PipelineVertexInputStateCreateInfo vertex_state{};

    vertex_state.pVertexBindingDescriptions = binding_desc.data();
    vertex_state.vertexBindingDescriptionCount =
        static_cast<uint32_t>( binding_desc.size() );

    vertex_state.pVertexAttributeDescriptions = input_attributes.data();
    vertex_state.vertexAttributeDescriptionCount =
        static_cast<uint32_t>( input_attributes.size() );

    vk_create_info.pVertexInputState = &vertex_state;

    vk_create_info.layout =
        *dynamic_cast<VulkanPipelineLayout *>( create_info.mLayout );

    vector<vk::PipelineColorBlendAttachmentState> blend_states{};
    vk::PipelineColorBlendStateCreateInfo         blend_state =
        Convert( create_info.mBlendState, blend_states );

    vk_create_info.pColorBlendState = &blend_state;

    vk::PipelineDepthStencilStateCreateInfo ds_state =
        Convert( create_info.mDepthStencilState );
    vk_create_info.pDepthStencilState = &ds_state;

    // --------------------------------TODO TERRITORY

    vk::PipelineRasterizationStateCreateInfo raster_state{};
    raster_state.polygonMode           = vk::PolygonMode::eFill;
    raster_state.frontFace             = vk::FrontFace::eCounterClockwise;
    raster_state.lineWidth             = 1.0f;
    raster_state.depthClampEnable      = false;
    raster_state.cullMode              = vk::CullModeFlagBits::eNone;
    vk_create_info.pRasterizationState = &raster_state;

    vk::PipelineInputAssemblyStateCreateInfo ia_state{};
    ia_state.topology                  = Convert( create_info.mTopology );
    vk_create_info.pInputAssemblyState = &ia_state;

    vk::PipelineMultisampleStateCreateInfo ms_state{};
    ms_state.rasterizationSamples    = vk::SampleCountFlagBits::e1;
    vk_create_info.pMultisampleState = &ms_state;

    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.viewportCount  = 1;
    viewport_state.scissorCount   = 1;
    vk_create_info.pViewportState = &viewport_state;

    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    array dynamic_states         = { vk::DynamicState::eViewport,
                             vk::DynamicState::eScissor };
    dynamic_state.pDynamicStates = dynamic_states.data();
    dynamic_state.dynamicStateCount =
        static_cast<uint32_t>( dynamic_states.size() );
    vk_create_info.pDynamicState = &dynamic_state;

    // --------------------------------TODO TERRITORY
    mPipelineImpl = mDevice.createGraphicsPipeline( nullptr, vk_create_info );
}
VulkanPipeline::~VulkanPipeline()
{
    if ( mPipelineImpl )
        mDevice.destroyPipeline( mPipelineImpl );
}