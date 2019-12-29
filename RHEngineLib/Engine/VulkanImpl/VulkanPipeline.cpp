#include "VulkanPipeline.h"
#include "VulkanShader.h"
using namespace rh::engine;

constexpr vk::ShaderStageFlagBits Convert( ShaderStage stage )
{
    switch ( stage )
    {
    case Compute: return vk::ShaderStageFlagBits::eCompute;
    case Geometry: return vk::ShaderStageFlagBits::eGeometry;
    case Pixel: return vk::ShaderStageFlagBits::eFragment;
    case Vertex: return vk::ShaderStageFlagBits::eVertex;
    }
}

VulkanPipeline::VulkanPipeline( const VulkanPipelineCreateInfo &create_info )
    : mDevice( create_info.mDevice )
{
    vk::GraphicsPipelineCreateInfo vk_create_info{};
    vk_create_info.renderPass = create_info.mRenderPass;
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{};
    shader_stages.reserve( create_info.mShaderStages.size() );
    std::transform( create_info.mShaderStages.begin(),
                    create_info.mShaderStages.end(),
                    std::back_inserter( shader_stages ),
                    []( const ShaderStageDesc &stage_desc ) {
                        vk::PipelineShaderStageCreateInfo vk_desc{};
                        vk_desc.stage = Convert( stage_desc.mStage );
                        vk_desc.pName = stage_desc.mEntryPoint.c_str();
                        vk_desc.module =
                            *dynamic_cast<VulkanShader *>( stage_desc.mShader );
                        return vk_desc;
                    } );

    // --------------------------------TODO TERRITORY
    vk::PipelineLayoutCreateInfo      pipe_layout_info{};
    vk::DescriptorSetLayoutCreateInfo desc_set_layout{};
    vk::DescriptorSetLayoutBinding    binding{};
    binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    mDevice.createDescriptorSetLayout( desc_set_layout );
    // pipe_layout_info.pSetLayouts = &desc_set_layout;
    mPipelineLayout       = mDevice.createPipelineLayout( pipe_layout_info );
    vk_create_info.layout = mPipelineLayout;

    vk::PipelineRasterizationStateCreateInfo raster_state{};
    raster_state.polygonMode           = vk::PolygonMode::eFill;
    raster_state.frontFace             = vk::FrontFace::eClockwise;
    raster_state.lineWidth             = 1.0f;
    raster_state.depthClampEnable      = false;
    vk_create_info.pRasterizationState = &raster_state;

    vk::PipelineInputAssemblyStateCreateInfo ia_state{};
    ia_state.topology                  = vk::PrimitiveTopology::eTriangleList;
    vk_create_info.pInputAssemblyState = &ia_state;

    vk::PipelineMultisampleStateCreateInfo ms_state{};
    ms_state.rasterizationSamples    = vk::SampleCountFlagBits::e1;
    vk_create_info.pMultisampleState = &ms_state;

    vk::PipelineColorBlendStateCreateInfo blend_state{};
    vk::PipelineColorBlendAttachmentState attachment_blend_state{};
    attachment_blend_state.blendEnable = false;
    attachment_blend_state.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    blend_state.attachmentCount     = 1;
    blend_state.pAttachments        = &attachment_blend_state;
    blend_state.blendConstants[0]   = 1.0f;
    blend_state.blendConstants[1]   = 1.0f;
    blend_state.blendConstants[2]   = 1.0f;
    blend_state.blendConstants[3]   = 1.0f;
    vk_create_info.pColorBlendState = &blend_state;

    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.viewportCount  = 1;
    viewport_state.scissorCount   = 1;
    vk_create_info.pViewportState = &viewport_state;

    vk::PipelineVertexInputStateCreateInfo           vertex_state{};
    std::vector<vk::VertexInputAttributeDescription> input_attributes;
    vk::VertexInputBindingDescription                binding_desc{};
    binding_desc.binding                       = 0;
    binding_desc.stride                        = 28;
    binding_desc.inputRate                     = vk::VertexInputRate::eVertex;
    vertex_state.pVertexBindingDescriptions    = &binding_desc;
    vertex_state.vertexBindingDescriptionCount = 1;

    vk::VertexInputAttributeDescription input_attr{};
    input_attr.binding  = 0;
    input_attr.location = 0;
    input_attr.format   = vk::Format::eR32G32B32A32Sfloat;
    input_attr.offset   = 0;
    input_attributes.push_back( input_attr );
    input_attr.binding  = 0;
    input_attr.location = 1;
    input_attr.format   = vk::Format::eR8G8B8A8Unorm;
    input_attr.offset   = 16;
    input_attributes.push_back( input_attr );
    input_attr.binding  = 0;
    input_attr.location = 2;
    input_attr.format   = vk::Format::eR32G32Sfloat;
    input_attr.offset   = 20;
    input_attributes.push_back( input_attr );
    vertex_state.pVertexAttributeDescriptions    = input_attributes.data();
    vertex_state.vertexAttributeDescriptionCount = input_attributes.size();

    vk_create_info.pVertexInputState = &vertex_state;

    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    std::array<vk::DynamicState, 2>    dynamic_states = {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    dynamic_state.pDynamicStates    = dynamic_states.data();
    dynamic_state.dynamicStateCount = dynamic_states.size();
    vk_create_info.pDynamicState    = &dynamic_state;

    // --------------------------------TODO TERRITORY
    vk_create_info.pStages    = shader_stages.data();
    vk_create_info.stageCount = shader_stages.size();
    mPipelineImpl = mDevice.createGraphicsPipeline( nullptr, vk_create_info );
}
VulkanPipeline::~VulkanPipeline()
{
    if ( mPipelineLayout )
        mDevice.destroyPipelineLayout( mPipelineLayout );
    if ( mPipelineImpl )
        mDevice.destroyPipeline( mPipelineImpl );
}