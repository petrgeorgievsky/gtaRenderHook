#include "CDebug.h"
#include "VulkanDevice.h"

CVulkanDevice::CVulkanDevice(VkPhysicalDevice gpu)
{
	m_extensions.push_back("VK_KHR_swapchain");
	FindGraphicsQueueFamily(gpu);

	float queuePriorities[] { 1.0f };

	VkDeviceQueueCreateInfo queueInfo { };
	queueInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueCount			= 1;
	queueInfo.pQueuePriorities		= queuePriorities;
	queueInfo.queueFamilyIndex		= m_QueueFamilyID;

	VkDeviceCreateInfo deviceInfo { };
	deviceInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount		= 1;
	deviceInfo.pQueueCreateInfos		= &queueInfo;
	deviceInfo.enabledLayerCount		= m_layers.size();
	deviceInfo.ppEnabledLayerNames		= m_layers.data();
	deviceInfo.enabledExtensionCount	= m_extensions.size();
	deviceInfo.ppEnabledExtensionNames	= m_extensions.data();

	VkPhysicalDeviceFeatures features { };
	features.shaderClipDistance		= VK_TRUE;
	deviceInfo.pEnabledFeatures		= &features;

	if( vkCreateDevice(gpu, &deviceInfo, NULL, &m_Device) != VK_SUCCESS )
		g_pDebug->printError("Failed to create logical device!");

	vkGetDeviceQueue(m_Device, m_QueueFamilyID, 0, &m_GraphicsQueue);
}


CVulkanDevice::~CVulkanDevice()
{
	vkDestroyDevice(m_Device, nullptr);
	m_Device = VK_NULL_HANDLE;
}

void CVulkanDevice::FindGraphicsQueueFamily(VkPhysicalDevice gpu)
{
	{
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, nullptr);
		vector<VkQueueFamilyProperties> familyPropertyList{ familyCount };
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, familyPropertyList.data());
		m_QueueFamilyID = -1;
		for (uint32_t i = 0; (i < familyCount) && m_QueueFamilyID<0; ++i)
			if (familyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				m_QueueFamilyID = i;
		if (m_QueueFamilyID < 0)
			g_pDebug->printError("Couldn't find graphics queue family");
	}
}
