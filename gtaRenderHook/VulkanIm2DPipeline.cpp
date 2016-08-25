#include "stdafx.h"
#include "VulkanIm2DPipeline.h"
#include "VulkanPipeline.h"
#include "VulkanRenderer.h"
#include "VulkanCommandBufferMgr.h"
#include "VulkanDevice.h"
#include "CDebug.h"
uint32_t CVulkanIm2DPipeline::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(m_pRenderer->getGPUlist()[m_pRenderer->GPU_ID()], &memory_properties);
	// Iterate over all memory types available for the device used in this example
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}

	throw "Could not find a suitable memory type!";
}

CVulkanIm2DPipeline::CVulkanIm2DPipeline(CVulkanRenderer* pRenderer)
{
	m_pRenderer = pRenderer;
	std::vector<VkVertexInputBindingDescription> inputBinding = {
		{
			0,                                                          // uint32_t                                       binding
			sizeof(RwIm2DVertex),                                         // uint32_t                                       stride
			VK_VERTEX_INPUT_RATE_VERTEX                                 // VkVertexInputRate                              inputRate
		} 
	};

	std::vector<VkVertexInputAttributeDescription> vertexAttributeBinding = { 
		{
			0,                                                      // uint32_t                                       location
			inputBinding[0].binding,								// uint32_t                                       binding
			VK_FORMAT_R32G32B32A32_SFLOAT,                          // VkFormat                                       format
			offsetof(RwIm2DVertex, x)								// uint32_t                                       offset
		},
		{
			1,                                                      // uint32_t                                       location
			inputBinding[0].binding,								// uint32_t                                       binding
			VK_FORMAT_R32G32_SFLOAT,								// VkFormat                                       format
			offsetof(RwIm2DVertex, u)								// uint32_t                                       offset
		},
		{
			2,                                                      // uint32_t                                       location
			inputBinding[0].binding,								// uint32_t                                       binding
			VK_FORMAT_B8G8R8A8_UNORM,								// VkFormat                                       format
			offsetof(RwIm2DVertex, emissiveColor)								// uint32_t                                       offset
		}
	};

	const VkDescriptorSetLayoutBinding myDescriptorSetLayoutBinding[] =
	{
		// binding to a single image descriptor
		{
			0,                                      // binding
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,       // descriptorType
			1,                                      // descriptorCount
			VK_SHADER_STAGE_FRAGMENT_BIT,           // stageFlags
			NULL                                    // pImmutableSamplers
		},
		// binding to a single image descriptor
		{
			1,                                      // binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       // descriptorType
			1,                                      // descriptorCount
			VK_SHADER_STAGE_VERTEX_BIT,           // stageFlags
			NULL                                    // pImmutableSamplers
		}
	};

	const VkDescriptorSetLayoutCreateInfo myDescriptorSetLayoutCreateInfo[] =
	{
		// Create info for first descriptor set with two descriptor bindings
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,    // sType
			NULL,                                                   // pNext
			0,                                                      // flags
			2,                                                      // bindingCount
			myDescriptorSetLayoutBinding                        // pBindings
		}
	};

	//
	// Create first descriptor set layout
	//
	vkCreateDescriptorSetLayout(m_pRenderer->getDevice()->getDevice(), &myDescriptorSetLayoutCreateInfo[0], NULL, &m_descriptorSetLayout);

	VkDescriptorPoolSize samplerPoolSize[2] = {};
	samplerPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize[0].descriptorCount = 1;
	samplerPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	samplerPoolSize[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = 1;
	poolCreateInfo.poolSizeCount = 2;
	poolCreateInfo.pPoolSizes = samplerPoolSize;

	vkCreateDescriptorPool(m_pRenderer->getDevice()->getDevice(), &poolCreateInfo, NULL, &m_descriptorPool);
	VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
	descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorAllocateInfo.descriptorPool = m_descriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 1;
	descriptorAllocateInfo.pSetLayouts = &m_descriptorSetLayout;

	vkAllocateDescriptorSets(m_pRenderer->getDevice()->getDevice(), &descriptorAllocateInfo, &m_descriptorSet);

	m_pBasePipeline_Fan		= new CVulkanPipeline(m_pRenderer->getDevice(), m_pRenderer->getRenderPass(),"im2d",inputBinding,vertexAttributeBinding,VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, &m_descriptorSetLayout);
	m_pBasePipeline_List	= new CVulkanPipeline(m_pRenderer->getDevice(), m_pRenderer->getRenderPass(), "im2d", inputBinding, vertexAttributeBinding,VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, &m_descriptorSetLayout);

	// VertexBuffer creation
	VkBufferCreateInfo vertexBufferInfo{};
	vertexBufferInfo.sType				= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size				= 0x40000*4;
	vertexBufferInfo.usage				= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.sharingMode		= VK_SHARING_MODE_EXCLUSIVE;

	vkCreateBuffer(m_pRenderer->getDevice()->getDevice(), &vertexBufferInfo, nullptr, &m_dynamicVertexBuffer);

	// VertexBuffer allocation
	{
		VkMemoryRequirements buffer_memory_requirements;
		vkGetBufferMemoryRequirements(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBuffer, &buffer_memory_requirements);
	
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(m_pRenderer->getGPUlist()[m_pRenderer->GPU_ID()], &memory_properties);
	
		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
			if ((buffer_memory_requirements.memoryTypeBits & (1 << i)) &&
				(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
	
				VkMemoryAllocateInfo memory_allocate_info = {
					VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // VkStructureType                        sType
					nullptr,                                    // const void                            *pNext
					buffer_memory_requirements.size,            // VkDeviceSize                           allocationSize
					i                                           // uint32_t                               memoryTypeIndex
				};
	
				vkAllocateMemory(m_pRenderer->getDevice()->getDevice(), &memory_allocate_info, nullptr, &m_dynamicVertexBufferMemory);
				break;
			}
		}
	}

	vkBindBufferMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBuffer, m_dynamicVertexBufferMemory, 0);

	// IndexBuffer creation
	VkBufferCreateInfo indexBufferInfo{};
	indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferInfo.size = 20000;
	indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateBuffer(m_pRenderer->getDevice()->getDevice(), &indexBufferInfo, nullptr, &m_IndexBuffer);

	// IndexBuffer allocation
	{
		VkMemoryRequirements buffer_memory_requirements;
		vkGetBufferMemoryRequirements(m_pRenderer->getDevice()->getDevice(), m_IndexBuffer, &buffer_memory_requirements);

		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(m_pRenderer->getGPUlist()[m_pRenderer->GPU_ID()], &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
			if ((buffer_memory_requirements.memoryTypeBits & (1 << i)) &&
				(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {

				VkMemoryAllocateInfo memory_allocate_info = {
					VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // VkStructureType                        sType
					nullptr,                                    // const void                            *pNext
					buffer_memory_requirements.size,            // VkDeviceSize                           allocationSize
					i                                           // uint32_t                               memoryTypeIndex
				};

				vkAllocateMemory(m_pRenderer->getDevice()->getDevice(), &memory_allocate_info, nullptr, &m_IndexBufferMemory);
				break;
			}
		}
	}

	vkBindBufferMemory(m_pRenderer->getDevice()->getDevice(), m_IndexBuffer, m_IndexBufferMemory, 0);

	// Prepare and initialize a uniform buffer block containing shader uniforms
	// In Vulkan there are no more single uniforms like in GL
	// All shader uniforms are passed as uniform buffer blocks 
	VkMemoryRequirements memReqs;

	// Vertex shader uniform buffer block
	VkBufferCreateInfo bufferInfo = {};
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(uboVS);
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	// Create a new buffer
	vkCreateBuffer(m_pRenderer->getDevice()->getDevice(), &bufferInfo, nullptr, &uniformDataVS.buffer);
	// Get memory requirements including size, alignment and memory type 
	vkGetBufferMemoryRequirements(m_pRenderer->getDevice()->getDevice(), uniformDataVS.buffer, &memReqs);
	allocInfo.allocationSize = memReqs.size;
	// Get the memory type index that supports host visible memory access
	// Most implementations offer multiple memory types and selecting the 
	// correct one to allocate memory from is important
	// We also want the buffer to be host coherent so we don't have to flush 
	// after every update. 
	// Note that this may affect performance so you might not want to do this 
	// in a real world application that updates buffers on a regular base
	allocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	// Allocate memory for the uniform buffer
	(vkAllocateMemory(m_pRenderer->getDevice()->getDevice(), &allocInfo, nullptr, &(uniformDataVS.memory)));
	// Bind memory to buffer
	(vkBindBufferMemory(m_pRenderer->getDevice()->getDevice(), uniformDataVS.buffer, uniformDataVS.memory, 0));

	// Store information in the uniform's descriptor
	uniformDataVS.descriptor.buffer = uniformDataVS.buffer;
	uniformDataVS.descriptor.offset = 0;
	uniformDataVS.descriptor.range = sizeof(uboVS);
}


CVulkanIm2DPipeline::~CVulkanIm2DPipeline()
{
	vkDestroyBuffer(m_pRenderer->getDevice()->getDevice(), uniformDataVS.buffer, nullptr);
	vkFreeMemory(m_pRenderer->getDevice()->getDevice(), uniformDataVS.memory, nullptr);
	vkDestroyDescriptorPool(m_pRenderer->getDevice()->getDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_pRenderer->getDevice()->getDevice(), m_descriptorSetLayout, nullptr);
	vkDestroyBuffer(m_pRenderer->getDevice()->getDevice(), m_IndexBuffer, nullptr);
	m_IndexBuffer = VK_NULL_HANDLE;
	vkFreeMemory(m_pRenderer->getDevice()->getDevice(), m_IndexBufferMemory, nullptr);
	m_IndexBufferMemory = VK_NULL_HANDLE;
	vkDestroyBuffer(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBuffer, nullptr);
	m_dynamicVertexBuffer = VK_NULL_HANDLE;
	vkFreeMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBufferMemory, nullptr);
	m_dynamicVertexBufferMemory = VK_NULL_HANDLE;
	delete m_pBasePipeline_Fan;
	delete m_pBasePipeline_List;

}

void CVulkanIm2DPipeline::Draw(RwPrimitiveType prim, RwIm2DVertex* verticles, RwUInt32 vertexCount)
{
	CVulkanPipeline* currentPipeline = nullptr;
	if (prim == rwPRIMTYPETRIFAN)
		currentPipeline = m_pBasePipeline_Fan;
	else if (prim == rwPRIMTYPETRILIST)
		currentPipeline = m_pBasePipeline_List;
	else
		g_pDebug->printMsg("undefined primitive type:" + std::to_string(prim));
	// Update VB
	{
		RwIm2DVertex *vertex_buffer_memory_pointer;
		m_dynVB_offset = m_dynVB_offset + vertexCount * sizeof(RwIm2DVertex) < 0x40000*4 ? m_dynVB_offset : 0;
		vkMapMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBufferMemory, m_dynVB_offset, vertexCount * sizeof(RwIm2DVertex), 0, (void**)&vertex_buffer_memory_pointer);

		memcpy(vertex_buffer_memory_pointer, verticles, vertexCount * sizeof(RwIm2DVertex));

		VkMappedMemoryRange flush_range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,            // VkStructureType        sType
			nullptr,                                          // const void            *pNext
			m_dynamicVertexBufferMemory,                       // VkDeviceMemory         memory
			m_dynVB_offset,                                                // VkDeviceSize           offset
			vertexCount * sizeof(RwIm2DVertex)                                     // VkDeviceSize           size
		};
		vkFlushMappedMemoryRanges(m_pRenderer->getDevice()->getDevice(), 1, &flush_range);

		vkUnmapMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBufferMemory);
	}

	vkCmdBindPipeline(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getPipeline());

	vkCmdBindDescriptorSets(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getPipelineLayout(), 0, 1, &m_descriptorSet, 0, NULL);

	VkDeviceSize offset = m_dynVB_offset;
	vkCmdBindVertexBuffers(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), 0, 1, &m_dynamicVertexBuffer, &offset);

	vkCmdDraw(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), vertexCount, 1, 0, 0);
	m_dynVB_offset += vertexCount * sizeof(RwIm2DVertex);
}

void CVulkanIm2DPipeline::DrawIndexed(RwPrimitiveType prim, RwIm2DVertex* verticles, RwUInt32 vertexCount, RwImVertexIndex *indices, RwUInt32 numIndices)
{
	CVulkanPipeline* currentPipeline = nullptr;
	int numInd = numIndices;
	if (prim == rwPRIMTYPETRIFAN) {
		currentPipeline = m_pBasePipeline_Fan;
		numInd = numInd - 2;
	}
	else if (prim == rwPRIMTYPETRILIST)
	{
		currentPipeline = m_pBasePipeline_List;
		numInd = numInd - 1;
	}
	else
		g_pDebug->printMsg("undefined primitive type:" + std::to_string(prim));
	if (numIndices <= vertexCount) {
		// Update VB
		{
			RwIm2DVertex *vertex_buffer_memory_pointer;
			m_dynVB_offset = m_dynVB_offset + numIndices * sizeof(RwIm2DVertex) < 0x40000 * 4 ? m_dynVB_offset : 0;
			vkMapMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBufferMemory, m_dynVB_offset, numIndices * sizeof(RwIm2DVertex), 0, (void**)&vertex_buffer_memory_pointer);

			for (RwUInt32 i = 0; i < numIndices; i++) {
				memcpy(&vertex_buffer_memory_pointer[i], &verticles[indices[i]], sizeof(RwIm2DVertex));
			}
			//memcpy(vertex_buffer_memory_pointer, verticles, numIndices * sizeof(RwIm2DVertex));

			VkMappedMemoryRange flush_range = {
				VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,            // VkStructureType        sType
				nullptr,                                          // const void            *pNext
				m_dynamicVertexBufferMemory,                       // VkDeviceMemory         memory
				m_dynVB_offset,                                                // VkDeviceSize           offset
				numIndices * sizeof(RwIm2DVertex)                                     // VkDeviceSize           size
			};
			vkFlushMappedMemoryRanges(m_pRenderer->getDevice()->getDevice(), 1, &flush_range);

			vkUnmapMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBufferMemory);
		}

		vkCmdBindPipeline(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getPipeline());

		vkCmdBindDescriptorSets(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getPipelineLayout(), 0, 1, &m_descriptorSet, 0, NULL);

		VkDeviceSize offset = m_dynVB_offset;
		vkCmdBindVertexBuffers(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), 0, 1, &m_dynamicVertexBuffer, &offset);

		vkCmdDraw(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), numIndices, 1, 0, 0);
		m_dynVB_offset += numIndices * sizeof(RwIm2DVertex);
		return;
	}
	
	// Update VB
	{
		RwIm2DVertex *vertex_buffer_memory_pointer;
		m_dynVB_offset = m_dynVB_offset + vertexCount * sizeof(RwIm2DVertex) < 0x40000*4 ? m_dynVB_offset : 0;
		vkMapMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBufferMemory, m_dynVB_offset, vertexCount * sizeof(RwIm2DVertex), 0, (void**)&vertex_buffer_memory_pointer);
		
		//for (RwUInt32 i = 0; i < vertexCount; i++)
		//	vertex_buffer_memory_pointer[i] = RwIm2DVertex{ (verticles[i].x - m_pRenderer->getWindowWidth() / 2) / m_pRenderer->getWindowWidth()*2,
		//	(verticles[i].y - m_pRenderer->getWindowHeight() / 2) / m_pRenderer->getWindowHeight()*2,
		//	verticles[i].z,verticles[i].rhw,verticles[i].emissiveColor,(float)verticles[i].u,(float)verticles[i].v };
		memcpy(vertex_buffer_memory_pointer, verticles, vertexCount * sizeof(RwIm2DVertex));

		VkMappedMemoryRange flush_range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,            // VkStructureType        sType
			nullptr,                                          // const void            *pNext
			m_dynamicVertexBufferMemory,                       // VkDeviceMemory         memory
			m_dynVB_offset,                                                // VkDeviceSize           offset
			vertexCount * sizeof(RwIm2DVertex)                                     // VkDeviceSize           size
		};
		vkFlushMappedMemoryRanges(m_pRenderer->getDevice()->getDevice(), 1, &flush_range);

		vkUnmapMemory(m_pRenderer->getDevice()->getDevice(), m_dynamicVertexBufferMemory);
	}
	// Update IB
	{
		RwImVertexIndex *index_buffer_memory_pointer;
		m_dynIB_offset = m_dynIB_offset + numIndices * sizeof(RwImVertexIndex) < 20000 ? m_dynIB_offset : 0;
		vkMapMemory(m_pRenderer->getDevice()->getDevice(), m_IndexBufferMemory, m_dynIB_offset, numIndices * sizeof(RwImVertexIndex), 0, (void**)&index_buffer_memory_pointer);
		
		//for (RwUInt32 i = 0; i < numIndices; i++)
		//	index_buffer_memory_pointer[i] = indices[i];
		memcpy(index_buffer_memory_pointer, indices, numIndices * sizeof(RwImVertexIndex));

		VkMappedMemoryRange flush_range = {
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,            // VkStructureType        sType
			nullptr,                                          // const void            *pNext
			m_IndexBufferMemory,                       // VkDeviceMemory         memory
			m_dynIB_offset,                                                // VkDeviceSize           offset
			numIndices * sizeof(RwImVertexIndex)                                     // VkDeviceSize           size
		};
		vkFlushMappedMemoryRanges(m_pRenderer->getDevice()->getDevice(), 1, &flush_range);

		vkUnmapMemory(m_pRenderer->getDevice()->getDevice(), m_IndexBufferMemory);
	}

	vkCmdBindPipeline(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getPipeline());
	
	vkCmdBindDescriptorSets(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getPipelineLayout(), 0, 1, &m_descriptorSet, 0, NULL);
	VkDeviceSize offset = m_dynVB_offset;
	vkCmdBindVertexBuffers(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), 0, 1, &m_dynamicVertexBuffer, &offset);
	vkCmdBindIndexBuffer(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), m_IndexBuffer, m_dynIB_offset, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(),numIndices,1,0, m_dynVB_offset,0);

	m_dynIB_offset += numIndices * sizeof(RwImVertexIndex);
	m_dynVB_offset += vertexCount * sizeof(RwIm2DVertex);
}

void CVulkanIm2DPipeline::EndScene()
{
	m_dynVB_offset = 0;
	m_dynIB_offset = 0;
}

VkPipelineLayout	& CVulkanIm2DPipeline::getFanPipeLayout()
{
	return m_pBasePipeline_Fan->getPipelineLayout();
}

VkPipelineLayout	& CVulkanIm2DPipeline::getListPipeLayout()
{
	return m_pBasePipeline_List->getPipelineLayout();
}

void CVulkanIm2DPipeline::UpdateViewProj(RwMatrix& view, RwMatrix& proj)
{
	uboVS.projectionMatrix = proj ;

	uboVS.viewMatrix = view;

	// Map uniform buffer and update it
	uint8_t *pData;
	(vkMapMemory(m_pRenderer->getDevice()->getDevice(), uniformDataVS.memory, 0, sizeof(uboVS), 0, (void **)&pData));
	memcpy(pData, &uboVS, sizeof(uboVS));
	vkUnmapMemory(m_pRenderer->getDevice()->getDevice(), uniformDataVS.memory);

	VkWriteDescriptorSet writeDescriptorSet = {};

	// Binding 0 : Uniform buffer
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = m_descriptorSet;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pBufferInfo = &uniformDataVS.descriptor;
	// Binds this uniform buffer to binding point 0
	writeDescriptorSet.dstBinding = 1;

	vkUpdateDescriptorSets(m_pRenderer->getDevice()->getDevice(), 1, &writeDescriptorSet, 0, NULL);
}
