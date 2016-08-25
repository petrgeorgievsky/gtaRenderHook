#include "stdafx.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanDevice.h"

CVulkanPipeline::CVulkanPipeline(CVulkanDevice* pDevice, VkRenderPass& renderPass, std::string name, 
	std::vector<VkVertexInputBindingDescription> inputBindigDesc, std::vector<VkVertexInputAttributeDescription> vertexAtrributeBinding, 
	VkPrimitiveTopology topology, VkDescriptorSetLayout* descSets)
{
	m_pDevice = pDevice;
	m_pVS = new CVulkanShader(m_pDevice, "shaders/" + name + "/vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT);
	m_pPS = new CVulkanShader(m_pDevice, "shaders/" + name + "/frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,    // VkStructureType                                sType
		nullptr,                                                      // const void                                    *pNext
		0,                                                            // VkPipelineVertexInputStateCreateFlags          flags;
		inputBindigDesc.size(),                                                            // uint32_t                                       vertexBindingDescriptionCount
		inputBindigDesc.data(),                                                      // const VkVertexInputBindingDescription         *pVertexBindingDescriptions
		vertexAtrributeBinding.size(),                                                            // uint32_t                                       vertexAttributeDescriptionCount
		vertexAtrributeBinding.data()                                                       // const VkVertexInputAttributeDescription       *pVertexAttributeDescriptions
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                                sType
		nullptr,                                                      // const void                                    *pNext
		0,                                                            // VkPipelineInputAssemblyStateCreateFlags        flags
		topology,                          // VkPrimitiveTopology                            topology
		VK_FALSE                                                      // VkBool32                                       primitiveRestartEnable
	};
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos = {
		// Vertex shader
		m_pVS->getStageCreateInfo(),
		// Fragment shader
		m_pPS->getStageCreateInfo()
	};

	VkViewport viewport = {
		0.0f,                                                         // float                                          x
		0.0f,                                                         // float                                          y
		640.0f,                                                       // float                                          width
		480.0f,                                                       // float                                          height
		0.0f,                                                         // float                                          minDepth
		1.0f                                                          // float                                          maxDepth
	};

	VkRect2D scissor = {
		{                                                             // VkOffset2D                                     offset
			0,                                                            // int32_t                                        x
			0                                                             // int32_t                                        y
		},
		{                                                             // VkExtent2D                                     extent
			640,                                                          // int32_t                                        width
			480                                                           // int32_t                                        height
		}
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,        // VkStructureType                                sType
		nullptr,                                                      // const void                                    *pNext
		0,                                                            // VkPipelineViewportStateCreateFlags             flags
		1,                                                            // uint32_t                                       viewportCount
		&viewport,                                                    // const VkViewport                              *pViewports
		1,                                                            // uint32_t                                       scissorCount
		&scissor                                                      // const VkRect2D                                *pScissors
	};
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,   // VkStructureType                                sType
		nullptr,                                                      // const void                                    *pNext
		0,                                                            // VkPipelineRasterizationStateCreateFlags        flags
		VK_FALSE,                                                     // VkBool32                                       depthClampEnable
		VK_FALSE,                                                     // VkBool32                                       rasterizerDiscardEnable
		VK_POLYGON_MODE_FILL,                                         // VkPolygonMode                                  polygonMode
		VK_CULL_MODE_NONE,                                        // VkCullModeFlags                                cullMode
		VK_FRONT_FACE_CLOCKWISE,                              // VkFrontFace                                    frontFace
		VK_FALSE,                                                     // VkBool32                                       depthBiasEnable
		0.0f,                                                         // float                                          depthBiasConstantFactor
		0.0f,                                                         // float                                          depthBiasClamp
		0.0f,                                                         // float                                          depthBiasSlopeFactor
		1.0f                                                          // float                                          lineWidth
	};

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,     // VkStructureType                                sType
		nullptr,                                                      // const void                                    *pNext
		0,                                                            // VkPipelineMultisampleStateCreateFlags          flags
		VK_SAMPLE_COUNT_1_BIT,                                        // VkSampleCountFlagBits                          rasterizationSamples
		VK_FALSE,                                                     // VkBool32                                       sampleShadingEnable
		1.0f,                                                         // float                                          minSampleShading
		nullptr,                                                      // const VkSampleMask                            *pSampleMask
		VK_FALSE,                                                     // VkBool32                                       alphaToCoverageEnable
		VK_FALSE                                                      // VkBool32                                       alphaToOneEnable
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		VK_TRUE,                                                     // VkBool32                                       blendEnable
		VK_BLEND_FACTOR_SRC_ALPHA,                                          // VkBlendFactor                                  srcColorBlendFactor
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,                                         // VkBlendFactor                                  dstColorBlendFactor
		VK_BLEND_OP_ADD,                                              // VkBlendOp                                      colorBlendOp
		VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcAlphaBlendFactor
		VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstAlphaBlendFactor
		VK_BLEND_OP_ADD,                                              // VkBlendOp                                      alphaBlendOp
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |         // VkColorComponentFlags                          colorWriteMask
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,     // VkStructureType                                sType
		nullptr,                                                      // const void                                    *pNext
		0,                                                            // VkPipelineColorBlendStateCreateFlags           flags
		VK_FALSE,                                                     // VkBool32                                       logicOpEnable
		VK_LOGIC_OP_COPY,                                             // VkLogicOp                                      logicOp
		1,                                                            // uint32_t                                       attachmentCount
		&color_blend_attachment_state,                                // const VkPipelineColorBlendAttachmentState     *pAttachments
		{ 0.0f, 0.0f, 0.0f, 0.0f }                                    // float                                          blendConstants[4]
	};

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = 4;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkPipelineLayoutCreateInfo layout_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // VkStructureType                sType
		nullptr,                                        // const void                    *pNext
		0,                                              // VkPipelineLayoutCreateFlags    flags
		1,                                              // uint32_t                       setLayoutCount
		descSets,                                        // const VkDescriptorSetLayout   *pSetLayouts
		1,                                              // uint32_t                       pushConstantRangeCount
		&pushConstantRange                                         // const VkPushConstantRange     *pPushConstantRanges
	};

	vkCreatePipelineLayout(m_pDevice->getDevice(), &layout_create_info, nullptr, &m_pipeline_layout);

	VkGraphicsPipelineCreateInfo pipeline_create_info = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,              // VkStructureType                                sType
		nullptr,                                                      // const void                                    *pNext
		0,                                                            // VkPipelineCreateFlags                          flags
		static_cast<uint32_t>(shader_stage_create_infos.size()),      // uint32_t                                       stageCount
		&shader_stage_create_infos[0],                                // const VkPipelineShaderStageCreateInfo         *pStages
		&vertex_input_state_create_info,                              // const VkPipelineVertexInputStateCreateInfo    *pVertexInputState;
		&input_assembly_state_create_info,                            // const VkPipelineInputAssemblyStateCreateInfo  *pInputAssemblyState
		nullptr,                                                      // const VkPipelineTessellationStateCreateInfo   *pTessellationState
		&viewport_state_create_info,                                  // const VkPipelineViewportStateCreateInfo       *pViewportState
		&rasterization_state_create_info,                             // const VkPipelineRasterizationStateCreateInfo  *pRasterizationState
		&multisample_state_create_info,                               // const VkPipelineMultisampleStateCreateInfo    *pMultisampleState
		nullptr,                                                      // const VkPipelineDepthStencilStateCreateInfo   *pDepthStencilState
		&color_blend_state_create_info,                               // const VkPipelineColorBlendStateCreateInfo     *pColorBlendState
		nullptr,                                                      // const VkPipelineDynamicStateCreateInfo        *pDynamicState
		m_pipeline_layout,                                        // VkPipelineLayout                               layout
		renderPass,                                            // VkRenderPass                                   renderPass
		0,                                                            // uint32_t                                       subpass
		VK_NULL_HANDLE,                                               // VkPipeline                                     basePipelineHandle
		-1                                                            // int32_t                                        basePipelineIndex
	};

	vkCreateGraphicsPipelines(m_pDevice->getDevice(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &m_pipeline);
}


CVulkanPipeline::~CVulkanPipeline()
{
	vkDestroyPipeline(m_pDevice->getDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_pDevice->getDevice(), m_pipeline_layout, nullptr);
	delete m_pPS;
	delete m_pVS;
}