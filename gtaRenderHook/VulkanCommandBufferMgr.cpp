#include "stdafx.h"
#include "VulkanCommandBufferMgr.h"
#include "VulkanDevice.h"
#include "CDebug.h"
CVulkanCommandBufferMgr::CVulkanCommandBufferMgr(CVulkanDevice* pDevice): m_pDevice{ pDevice }
{
	VkCommandPoolCreateInfo		poolInfo {};
	poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex	= m_pDevice->getGraphicsQueueFamilyID();
	poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	vkCreateCommandPool(m_pDevice->getDevice(), &poolInfo, nullptr, &m_pool);

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool		= m_pool;
	allocateInfo.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(m_pDevice->getDevice(), &allocateInfo, &m_renderBuffer);
}


CVulkanCommandBufferMgr::~CVulkanCommandBufferMgr()
{
	vkFreeCommandBuffers(m_pDevice->getDevice(), m_pool, 1, &m_renderBuffer);
	vkDestroyCommandPool(m_pDevice->getDevice(), m_pool, nullptr);
}

void CVulkanCommandBufferMgr::BeginRenderBuffer(VkImage& image)
{
	vkResetCommandBuffer(m_renderBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	VkImageSubresourceRange image_subresource_range = {
		VK_IMAGE_ASPECT_COLOR_BIT,                    // VkImageAspectFlags                     aspectMask
		0,                                            // uint32_t                               baseMipLevel
		1,                                            // uint32_t                               levelCount
		0,                                            // uint32_t                               baseArrayLayer
		1                                             // uint32_t                               layerCount
	};
	VkCommandBufferBeginInfo cmd_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // VkStructureType                        sType
		nullptr,                                      // const void                            *pNext
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // VkCommandBufferUsageFlags              flags
		nullptr                                       // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
	};
	VkImageMemoryBarrier barrier_from_present_to_clear = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
		nullptr,                                    // const void                            *pNext
		(m_firstTimeInit<0)? (VkAccessFlags)0 : VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          srcAccessMask
		VK_ACCESS_MEMORY_READ_BIT,               // VkAccessFlags                          dstAccessMask
		(m_firstTimeInit<0)? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                          oldLayout
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,       // VkImageLayout                          newLayout
		m_pDevice->getGraphicsQueueFamilyID(),      // uint32_t                               srcQueueFamilyIndex
		m_pDevice->getGraphicsQueueFamilyID(),      // uint32_t                               dstQueueFamilyIndex
		image,										// VkImage                                image
		image_subresource_range                     // VkImageSubresourceRange                subresourceRange
	};
	if (m_firstTimeInit<0)
		m_firstTimeInit++;
	VkResult res=vkBeginCommandBuffer(m_renderBuffer, &cmd_buffer_begin_info);
	if (res != VK_SUCCESS)
		g_pDebug->printError("fail to begin cmd buffer");
	vkCmdPipelineBarrier(m_renderBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);
}

void CVulkanCommandBufferMgr::EndRenderBuffer(VkImage& image)
{
	VkImageSubresourceRange image_subresource_range = {
		VK_IMAGE_ASPECT_COLOR_BIT,                    // VkImageAspectFlags                     aspectMask
		0,                                            // uint32_t                               baseMipLevel
		1,                                            // uint32_t                               levelCount
		0,                                            // uint32_t                               baseArrayLayer
		1                                             // uint32_t                               layerCount
	};
	VkImageMemoryBarrier barrier_from_clear_to_present = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
		nullptr,                                    // const void                            *pNext
		VK_ACCESS_MEMORY_READ_BIT,               // VkAccessFlags                          srcAccessMask
		VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          dstAccessMask
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,       // VkImageLayout                          oldLayout
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                          newLayout
		m_pDevice->getGraphicsQueueFamilyID(),      // uint32_t                               srcQueueFamilyIndex
		m_pDevice->getGraphicsQueueFamilyID(),      // uint32_t                               dstQueueFamilyIndex
		image,								        // VkImage                                image
		image_subresource_range                     // VkImageSubresourceRange                subresourceRange
	};


	vkCmdPipelineBarrier(m_renderBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);
	VkResult res = vkEndCommandBuffer(m_renderBuffer);

}
