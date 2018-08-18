#pragma once
#include "../../../stdafx.h"
namespace RHEngine {
	class VulkanBackBuffer
	{
	public:
		VulkanBackBuffer(const VkDevice &device, const VkSwapchainKHR& swapChain);
		~VulkanBackBuffer();
		VkImageView GetImageView() const;
		VkImage GetImage() const;
		/* 
			Sets current back-buffer(where we will be rendering) id
		*/
		void SetBackBufferID(uint32_t id);
		/*
			Returns current back-buffer id
		*/
		uint32_t GetBackBufferID();
	private:
		// Represe
		std::vector<VkImage> m_vkBackBufferImages;
		VkImageView m_vkImageView = VK_NULL_HANDLE;
		uint32_t m_uiBackBufferID = 0;
	};
};