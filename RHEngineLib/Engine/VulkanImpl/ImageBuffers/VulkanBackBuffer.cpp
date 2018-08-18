#include "stdafx.h"
#include "VulkanBackBuffer.h"
#include "../../../DebugUtils/DebugLogger.h"

RHEngine::VulkanBackBuffer::VulkanBackBuffer(const VkDevice &device, const VkSwapchainKHR &swapChain)
{
	uint32_t imageCount;
	if (!CALL_VK_API(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr),
		TEXT("Failed to create logical device!")))
		return;
	m_vkBackBufferImages.resize(imageCount);
	if (!CALL_VK_API(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, m_vkBackBufferImages.data()),
        TEXT("Failed to create logical device!")))
		return;
}

RHEngine::VulkanBackBuffer::~VulkanBackBuffer()
{
}

VkImageView RHEngine::VulkanBackBuffer::GetImageView() const
{
	return VkImageView();
}

VkImage RHEngine::VulkanBackBuffer::GetImage() const
{
	return m_vkBackBufferImages[m_uiBackBufferID];
}

uint32_t RHEngine::VulkanBackBuffer::GetBackBufferID()
{
	return m_uiBackBufferID;
}

void RHEngine::VulkanBackBuffer::SetBackBufferID(uint32_t id)
{
	m_uiBackBufferID = id;
}
