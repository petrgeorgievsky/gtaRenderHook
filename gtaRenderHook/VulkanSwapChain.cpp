#include "stdafx.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"

CVulkanSwapChain::CVulkanSwapChain(CVulkanRenderer* pRenderer): m_pRenderer { pRenderer }
{
	VkPresentModeKHR presentMode = m_findPresentMode(m_pRenderer->getCurrentGPU(), m_pRenderer->getWindowSurface());
	m_surfaceFormat = m_findSurfaceFormat();

	VkSwapchainCreateInfoKHR swapchaininfo{};
	{
		swapchaininfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchaininfo.surface = m_pRenderer->getWindowSurface();
		swapchaininfo.minImageCount = m_swapChainImageCount;
		swapchaininfo.imageFormat = m_surfaceFormat.format;
		swapchaininfo.imageColorSpace = m_surfaceFormat.colorSpace;
		swapchaininfo.imageExtent.width = m_pRenderer->getWindowWidth();
		swapchaininfo.imageExtent.height = m_pRenderer->getWindowHeight();
		swapchaininfo.imageArrayLayers = 1;
		swapchaininfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchaininfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchaininfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchaininfo.presentMode = presentMode;
		swapchaininfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchaininfo.clipped = VK_TRUE;
	}

	vkCreateSwapchainKHR(m_pRenderer->getDevice()->getDevice(), &swapchaininfo, nullptr, &m_swapChain);
}

CVulkanSwapChain::~CVulkanSwapChain()
{
	for (uint32_t i = 0;i < m_swapChainImageCount;i++) {
		vkDestroyFramebuffer(m_pRenderer->getDevice()->getDevice(), m_frameBuffers[i], nullptr);
		vkDestroyImageView(m_pRenderer->getDevice()->getDevice(), m_swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_pRenderer->getDevice()->getDevice(), m_swapChain, nullptr);
	m_swapChain = VK_NULL_HANDLE;
}

void CVulkanSwapChain::CreateFrameBuffers()
{
	vkGetSwapchainImagesKHR(m_pRenderer->getDevice()->getDevice(), m_swapChain, &m_swapChainImageCount, nullptr);
	m_swapChainImages.resize(m_swapChainImageCount);
	m_swapChainImageViews.resize(m_swapChainImageCount);
	m_frameBuffers.resize(m_swapChainImageCount);
	vkGetSwapchainImagesKHR(m_pRenderer->getDevice()->getDevice(), m_swapChain, &m_swapChainImageCount, m_swapChainImages.data());

	for (uint32_t i = 0;i < m_swapChainImageCount;i++) {
		VkImageViewCreateInfo imageViewInfo{};
		{
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.image = m_swapChainImages[i];
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewInfo.format = m_surfaceFormat.format;
			imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
		}

		vkCreateImageView(m_pRenderer->getDevice()->getDevice(), &imageViewInfo, nullptr, &m_swapChainImageViews[i]);

		VkFramebufferCreateInfo framebuffer_create_info = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // VkStructureType                sType
			nullptr,                                    // const void                    *pNext
			0,                                          // VkFramebufferCreateFlags       flags
			m_pRenderer->getRenderPass(),               // VkRenderPass                   renderPass
			1,                                          // uint32_t                       attachmentCount
			&m_swapChainImageViews[i],    // const VkImageView             *pAttachments
			m_pRenderer->getWindowWidth(),                                        // uint32_t                       width
			m_pRenderer->getWindowHeight(),                                        // uint32_t                       height
			1                                           // uint32_t                       layers
		};

		vkCreateFramebuffer(m_pRenderer->getDevice()->getDevice(), &framebuffer_create_info, nullptr, &m_frameBuffers[i]);
	}
}

VkPresentModeKHR CVulkanSwapChain::m_findPresentMode(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	// get present mode count
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr);
	// get present mode list
	std::vector<VkPresentModeKHR> presentModeList{ presentModeCount };
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModeList.data());

	// iterate through present mode list and find if mailbox mode is supported
	for (uint32_t i = 0; i < presentModeCount; i++)
		if (presentModeList[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	
	return presentMode;
}

VkSurfaceFormatKHR CVulkanSwapChain::m_findSurfaceFormat()
{
	VkSurfaceFormatKHR surface_format;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pRenderer->getCurrentGPU(), m_pRenderer->getWindowSurface(), &m_surfaceCaps);

	VkBool32 surfaceSupported = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(m_pRenderer->getCurrentGPU(), m_pRenderer->getDevice()->getGraphicsQueueFamilyID(), m_pRenderer->getWindowSurface(), &surfaceSupported);
	// get surface format count
	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_pRenderer->getCurrentGPU(), m_pRenderer->getWindowSurface(), &surfaceFormatCount, nullptr);
	// get surface format list
	std::vector<VkSurfaceFormatKHR> surfaceFormatList{ surfaceFormatCount };
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_pRenderer->getCurrentGPU(), m_pRenderer->getWindowSurface(), &surfaceFormatCount, surfaceFormatList.data());

	if (surfaceFormatList[0].format == VK_FORMAT_UNDEFINED)
	{
		surface_format.format = VK_FORMAT_R8G8B8A8_UNORM;
		surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	else
		surface_format = surfaceFormatList[0];
	return surface_format;
}

