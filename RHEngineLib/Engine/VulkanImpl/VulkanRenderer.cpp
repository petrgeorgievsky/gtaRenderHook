#include "stdafx.h"
#include "VulkanRenderer.h"
#include "ImageBuffers\VulkanBackBuffer.h"
#include "..\..\DebugUtils\DebugLogger.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

	RHDebug::DebugLogger::Log(ToRHString(msg));
	return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

RHEngine::VulkanRenderer::VulkanRenderer(HWND window, HINSTANCE inst) : 
	IRenderer(window, inst)
{
	m_aExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	m_aExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	m_aExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	
	// This extension is crutial to display selection, but for some reason, 
	// no implementation exists for windows, guess we'll have to live without it :)
	//m_aExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
    //m_aLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	m_aLayers.push_back("VK_LAYER_LUNARG_monitor");
	
    // App info
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Render Hook App";
    app_info.pEngineName = "Render Hook Engine";

    // Instance info
	VkInstanceCreateInfo inst_info{};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = m_aExtensions.size();
	inst_info.ppEnabledExtensionNames = m_aExtensions.data();
	inst_info.enabledLayerCount = m_aLayers.size();
	inst_info.ppEnabledLayerNames = m_aLayers.data();

    // Create vulkan instance
	if (!CALL_VK_API(vkCreateInstance(&inst_info, nullptr, &m_vkInstance), 
        L"VulkanRenderer failed to initialize: Failed to initialize Vulkan Instance!"))
		return;

	// Enumerate GPUS
	uint32_t deviceCount;
	if (!CALL_VK_API(vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr),
		L"VulkanRenderer failed to initialize: Failed to retrieve adapter count!"))
		return;
    
    if (deviceCount <= 0)
    {
        RHDebug::DebugLogger::Error(L"VulkanRenderer failed to initialize: GPU count <= 0.");
        return;
    }

	m_aAdapters.resize(deviceCount);
	if (!CALL_VK_API(vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, m_aAdapters.data()),
		L"VulkanRenderer failed to initialize: Failed to get adapter list!"))
		return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
	createInfo.pfnCallback = debugCallback;
	if (!CALL_VK_API(CreateDebugReportCallbackEXT(m_vkInstance, &createInfo, nullptr, &m_debugCallback),
		L"VulkanRenderer failed to initialize: Failed to create debug callback!"))
		return;

    SetCurrentAdapter(0);
    SetCurrentOutput(0);
}

RHEngine::VulkanRenderer::~VulkanRenderer()
{
	//vkDestroyDebugReportCallbackEXT(m_vkInstance, m_debugCallback, nullptr);
	vkDestroySurfaceKHR(m_vkInstance, m_vkWindowSurface, nullptr);
	vkDestroyInstance(m_vkInstance, nullptr);
}

bool RHEngine::VulkanRenderer::InitDevice()
{
    float queuePriority[] = { 0.5f,0.5f };
    const char* devExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = m_aGraphicsQueueFamilies[0];
	queueCreateInfo.queueCount = 2;
	queueCreateInfo.pQueuePriorities = queuePriority;

	VkDeviceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.pQueueCreateInfos = &queueCreateInfo;
	info.queueCreateInfoCount = 1;
	info.enabledExtensionCount = 1;
	info.ppEnabledExtensionNames = devExtensions;

	if (!CALL_VK_API(vkCreateDevice(m_aAdapters[m_uiCurrentAdapter], &info, nullptr, &m_vkDevice),
		L"Device Initialization fail: Failed to create logical device!"))
		return false;
    // Retrieve queue handles
    vkGetDeviceQueue(m_vkDevice, m_aGraphicsQueueFamilies[0], 0, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkDevice, m_aGraphicsQueueFamilies[0], 1, &m_vkPresentQueue);

    uint32_t backBufferImgCount = m_vkWindowSurfaceCap.minImageCount;
    m_vkMainCommandBufferList.resize(backBufferImgCount);

	// Initialize command pool and main command buffer
	VkCommandPoolCreateInfo cmdPoolInfo{};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = m_aGraphicsQueueFamilies[0];
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (!CALL_VK_API(vkCreateCommandPool(m_vkDevice, &cmdPoolInfo, nullptr, &m_vkGraphicsQueueCommandPool),
        L"Failed to create command pool!"))
    {
        vkDestroyDevice(m_vkDevice, nullptr);
        return false;
    }

	VkCommandBufferAllocateInfo mainCmdBufferInfo{};
	mainCmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	mainCmdBufferInfo.commandBufferCount = backBufferImgCount;
	mainCmdBufferInfo.commandPool = m_vkGraphicsQueueCommandPool;
	mainCmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
    if (!CALL_VK_API(vkAllocateCommandBuffers(m_vkDevice, &mainCmdBufferInfo, m_vkMainCommandBufferList.data()),
        L"Failed to allocate main command buffer!"))
    {
        vkDestroyCommandPool(m_vkDevice, m_vkGraphicsQueueCommandPool, nullptr);
        vkDestroyDevice(m_vkDevice, nullptr);
        return false;
    }

	VkSemaphoreCreateInfo imageAquireSemaphoreInfo{};
	imageAquireSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (!CALL_VK_API(vkCreateSemaphore(m_vkDevice, &imageAquireSemaphoreInfo, nullptr, &m_vkSwapChainImageSemaphore),
        L"Failed to create swap-chain image wait semaphore!"))
    {
        vkFreeCommandBuffers(m_vkDevice, m_vkGraphicsQueueCommandPool, m_vkMainCommandBufferList.size(), m_vkMainCommandBufferList.data());
        vkDestroyCommandPool(m_vkDevice, m_vkGraphicsQueueCommandPool, nullptr);
        vkDestroyDevice(m_vkDevice, nullptr);
        return false;
    }

	if (!CALL_VK_API(vkCreateSemaphore(m_vkDevice, &imageAquireSemaphoreInfo, nullptr, &m_vkMainCmdBufferSemaphore),
		L"Failed to create command buffer execution wait semaphore!"))
    {
        vkDestroySemaphore(m_vkDevice, m_vkSwapChainImageSemaphore, nullptr);
        vkFreeCommandBuffers(m_vkDevice, m_vkGraphicsQueueCommandPool, m_vkMainCommandBufferList.size(), m_vkMainCommandBufferList.data());
        vkDestroyCommandPool(m_vkDevice, m_vkGraphicsQueueCommandPool, nullptr);
        vkDestroyDevice(m_vkDevice, nullptr);
        return false;
    }
    m_vkMainCmdBufferFences.resize(backBufferImgCount);

	VkFenceCreateInfo renderFenceInfo{};
	renderFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    renderFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < m_vkMainCmdBufferFences.size(); i++)
    {
        if (!CALL_VK_API(vkCreateFence(m_vkDevice, &renderFenceInfo, nullptr, &m_vkMainCmdBufferFences[i]),
            L"Failed to create wait fence!")) 
        {
            for (int j = i; j >= 0; j--)
                vkDestroyFence(m_vkDevice, m_vkMainCmdBufferFences[i], nullptr);
            vkDestroySemaphore(m_vkDevice, m_vkMainCmdBufferSemaphore, nullptr);
            vkDestroySemaphore(m_vkDevice, m_vkSwapChainImageSemaphore, nullptr);
            vkFreeCommandBuffers(m_vkDevice, m_vkGraphicsQueueCommandPool, m_vkMainCommandBufferList.size(), m_vkMainCommandBufferList.data());
            vkDestroyCommandPool(m_vkDevice, m_vkGraphicsQueueCommandPool, nullptr);
            vkDestroyDevice(m_vkDevice, nullptr);
            return false;
        }
    }
	
	VkSwapchainCreateInfoKHR swapChainInfo = {};
	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.surface = m_vkWindowSurface;
	// HARDCODED: replace
	VkExtent2D extent;
	extent.width = 640;
	extent.height = 480;
	swapChainInfo.minImageCount = backBufferImgCount;
	swapChainInfo.imageFormat = m_aWindowSurfaceFormats[0].format;
	swapChainInfo.imageColorSpace = m_aWindowSurfaceFormats[0].colorSpace;
	swapChainInfo.imageExtent = m_vkWindowSurfaceCap.currentExtent;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (m_vkWindowSurfaceCap.supportedUsageFlags&VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    swapChainInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;//m_aWindowSurfacePresentModes[2];
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;//m_vkWindowSurfaceCap.currentTransform;

	if (!CALL_VK_API(vkCreateSwapchainKHR(m_vkDevice, &swapChainInfo, nullptr, &m_vkSwapChain),
		L"Failed to create swap chain!")) 
    {
        for (size_t i = 0; i < m_vkMainCmdBufferFences.size(); i++)
            vkDestroyFence(m_vkDevice, m_vkMainCmdBufferFences[i], nullptr);
        vkDestroySemaphore(m_vkDevice, m_vkMainCmdBufferSemaphore, nullptr);
        vkDestroySemaphore(m_vkDevice, m_vkSwapChainImageSemaphore, nullptr);
        vkFreeCommandBuffers(m_vkDevice, m_vkGraphicsQueueCommandPool, m_vkMainCommandBufferList.size(), m_vkMainCommandBufferList.data());
        vkDestroyCommandPool(m_vkDevice, m_vkGraphicsQueueCommandPool, nullptr);
        vkDestroyDevice(m_vkDevice, nullptr);
        return false;
    }
	 
	return true;
}

bool RHEngine::VulkanRenderer::ShutdownDevice()
{
    vkDeviceWaitIdle(m_vkDevice);
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, nullptr);
    for (size_t i = 0; i < m_vkMainCmdBufferFences.size(); i++)
        vkDestroyFence(m_vkDevice, m_vkMainCmdBufferFences[i], nullptr);
    vkDestroySemaphore(m_vkDevice, m_vkMainCmdBufferSemaphore, nullptr);
    vkDestroySemaphore(m_vkDevice, m_vkSwapChainImageSemaphore, nullptr);
    vkFreeCommandBuffers(m_vkDevice, m_vkGraphicsQueueCommandPool, m_vkMainCommandBufferList.size(), m_vkMainCommandBufferList.data());
    vkDestroyCommandPool(m_vkDevice, m_vkGraphicsQueueCommandPool, nullptr);
    vkDestroyDevice(m_vkDevice, nullptr);
    return true;
}

bool RHEngine::VulkanRenderer::GetAdaptersCount(int & n)
{
	n = m_aAdapters.size();
	return true;
}

bool RHEngine::VulkanRenderer::GetAdapterInfo(unsigned int n, std::wstring &info)
{
	if (n >= m_aAdapters.size())
		return false;
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_aAdapters[n], &props);
	info = ToRHString(props.deviceName);
	return true;
}

bool RHEngine::VulkanRenderer::SetCurrentAdapter(unsigned int n)
{
	if(n >= m_aAdapters.size())
		return false;
	m_uiCurrentAdapter = n;
	// Retrieve Queue Families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_aAdapters[m_uiCurrentAdapter], &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_aAdapters[m_uiCurrentAdapter], &queueFamilyCount, queueFamilies.data());
	int q = 0;
	// Initialize queue families
	for (auto fam : queueFamilies)
	{
		if (fam.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			m_aGraphicsQueueFamilies.push_back(q);
		if (fam.queueFlags & VK_QUEUE_COMPUTE_BIT)
			m_aComputeQueueFamilies.push_back(q);
		if (fam.queueFlags & VK_QUEUE_TRANSFER_BIT)
			m_aTransferQueueFamilies.push_back(q);
		q++;
	}

	if (m_aGraphicsQueueFamilies.size() <= 0)
		return false;
	
	// Retrieve window surface
	VkWin32SurfaceCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hinstance = m_hInst;
	info.hwnd = m_hWnd;
	if (!CALL_VK_API(vkCreateWin32SurfaceKHR(m_vkInstance, &info, nullptr, &m_vkWindowSurface),
		L"Failed to create window surface!"))
		return false;
	VkBool32 surfaceSupported;
	if (!CALL_VK_API(vkGetPhysicalDeviceSurfaceSupportKHR(m_aAdapters[n], m_aGraphicsQueueFamilies[0], m_vkWindowSurface, &surfaceSupported),
		L"Failed to get surface support!"))
		return false;
	if (!surfaceSupported)
		return false;
	if (!CALL_VK_API(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_aAdapters[n], m_vkWindowSurface, &m_vkWindowSurfaceCap),
		L"Failed to get surface capabilities!"))
		return false;
	uint32_t surfaceFormatCount;
	if (!CALL_VK_API(vkGetPhysicalDeviceSurfaceFormatsKHR(m_aAdapters[n], m_vkWindowSurface, &surfaceFormatCount, nullptr),
		L"Failed to get physical device surface format count!"))
		return false;
	m_aWindowSurfaceFormats.resize(surfaceFormatCount);
	if (!CALL_VK_API(vkGetPhysicalDeviceSurfaceFormatsKHR(m_aAdapters[n], m_vkWindowSurface, &surfaceFormatCount, m_aWindowSurfaceFormats.data()),
		L"Failed to get physical device surface formats!"))
		return false;
	uint32_t presentModeCount;
	if (!CALL_VK_API(vkGetPhysicalDeviceSurfacePresentModesKHR(m_aAdapters[n], m_vkWindowSurface, &presentModeCount, nullptr),
		L"Failed to get physical device surface present mode count!"))
		return false;
	m_aWindowSurfacePresentModes.resize(presentModeCount);
	if (!CALL_VK_API(vkGetPhysicalDeviceSurfacePresentModesKHR(m_aAdapters[n], m_vkWindowSurface, &presentModeCount, m_aWindowSurfacePresentModes.data()),
		L"Failed to get physical device surface present modes!"))
		return false;
	return true;
}

bool RHEngine::VulkanRenderer::GetOutputCount(unsigned int adapterId, int & c)
{
	if (adapterId >= m_aAdapters.size())
		return false;
	// Enumerate outputs(displays)
	uint32_t displayCount=1;
	/*if (!CALL_VK_API(vkGetPhysicalDeviceDisplayPropertiesKHR(m_aAdapters[n], &displayCount, nullptr),
		L"Failed to get output count!"))
		return false;
	m_aOutputProperties.resize(displayCount);
	if (!CALL_VK_API(vkGetPhysicalDeviceDisplayPropertiesKHR(m_aAdapters[n], &displayCount, m_aOutputProperties.data()),
		L"Failed to get output count!"))
		return false;*/
	c = displayCount;
	return true;
}

bool RHEngine::VulkanRenderer::GetOutputInfo(unsigned int n, std::wstring &info)
{
	if (n >= m_aOutputProperties.size())
		return false;
	info = L"DefaultDisplay";
	return true;
}

bool RHEngine::VulkanRenderer::SetCurrentOutput(unsigned int id)
{
	return true;
}

bool RHEngine::VulkanRenderer::GetDisplayModeCount(unsigned int outputId, int &c)
{
	// Vulkan can't do such computations, so we fallback for now
	c = 1;
	return true;
}

bool RHEngine::VulkanRenderer::GetDisplayModeInfo(unsigned int id, DisplayModeInfo &info)
{
	info.width = 640;
	info.height = 480;
	info.refreshRate = 60;
	return true;
}

bool RHEngine::VulkanRenderer::SetCurrentDisplayMode(unsigned int id)
{
	return true;
}

bool RHEngine::VulkanRenderer::GetCurrentAdapter(int & n)
{
	n = 0;
	return true;
}

bool RHEngine::VulkanRenderer::GetCurrentOutput(int & n)
{
	n = 0;
	return true;
}

bool RHEngine::VulkanRenderer::GetCurrentDisplayMode(int & n)
{
	n = 0;
	return true;
}

bool RHEngine::VulkanRenderer::Present(void* image)
{
	VulkanBackBuffer* backBuffer = reinterpret_cast<VulkanBackBuffer*>(image);
	if (backBuffer == nullptr) return false;
	uint32_t currentBackBufferId = backBuffer->GetBackBufferID();

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pSwapchains = &m_vkSwapChain;
	presentInfo.swapchainCount = 1;	
	presentInfo.pImageIndices = &currentBackBufferId;
	presentInfo.pWaitSemaphores = &m_vkMainCmdBufferSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	vkQueuePresentKHR(m_vkPresentQueue, &presentInfo);
	return true;
}

void * RHEngine::VulkanRenderer::AllocateImageBuffer(unsigned int width, unsigned int height, RHImageBufferType type)
{
	switch (type)
	{
	case RHEngine::RHImageBufferType::Unknown:
		break;
	case RHEngine::RHImageBufferType::BackBuffer:
		return new VulkanBackBuffer(m_vkDevice, m_vkSwapChain);
	case RHEngine::RHImageBufferType::TextureBuffer:
		break;
	case RHEngine::RHImageBufferType::DepthBuffer:
		break;
	case RHEngine::RHImageBufferType::RenderTargetBuffer:
		break;
	default:
		break;
	}
	return nullptr;
}

bool RHEngine::VulkanRenderer::FreeImageBuffer(void * buffer, RHImageBufferType type)
{
    switch (type)
    {
    case RHEngine::RHImageBufferType::Unknown:
        break;
    case RHEngine::RHImageBufferType::BackBuffer:
        delete static_cast<VulkanBackBuffer*>(buffer);
        break;
    case RHEngine::RHImageBufferType::TextureBuffer:
        break;
    case RHEngine::RHImageBufferType::DepthBuffer:
        break;
    case RHEngine::RHImageBufferType::RenderTargetBuffer:
        break;
    default:
        break;
    }
	return true;
}

bool RHEngine::VulkanRenderer::BindImageBuffers(RHImageBindType bindType, const std::unordered_map<int, void*>& buffers)
{
	VulkanBackBuffer * backBuffer = reinterpret_cast<VulkanBackBuffer*>(buffers.at(0));
	if (backBuffer == nullptr) return false;
    VkImageSubresourceRange subresRange{};
    subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresRange.levelCount = 1;
    subresRange.layerCount = 1;

	// A barrier to convert back-buffer image from read-only to write-only and from undefined layout to transfer layout
	VkImageMemoryBarrier cmdlistBarrier{};
	cmdlistBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	cmdlistBarrier.image = backBuffer->GetImage();
	cmdlistBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	cmdlistBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	cmdlistBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	cmdlistBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	cmdlistBarrier.srcQueueFamilyIndex = m_aGraphicsQueueFamilies[0];
	cmdlistBarrier.dstQueueFamilyIndex = m_aGraphicsQueueFamilies[0];
	cmdlistBarrier.subresourceRange = subresRange;
	vkCmdPipelineBarrier(m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer],
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &cmdlistBarrier);

	return true;
}

bool RHEngine::VulkanRenderer::ClearImageBuffer(RHImageClearType clearType, void* buffer, const float clearColor[4])
{
	VulkanBackBuffer * backBuffer = reinterpret_cast<VulkanBackBuffer*>(buffer);
	if (backBuffer == nullptr) return false;
	VkClearColorValue vkClearColor{};
	VkImageSubresourceRange subresRange{};
	switch (clearType)
	{
	case RHEngine::RHImageClearType::Color:
		
		
		subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresRange.levelCount = 1;
		subresRange.layerCount = 1;
		
		vkClearColor.float32[0] = clearColor[0];
		vkClearColor.float32[1] = clearColor[1];
		vkClearColor.float32[2] = clearColor[2];
		vkClearColor.float32[3] = clearColor[3];
		vkCmdClearColorImage(m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer],
			backBuffer->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vkClearColor, 1, &subresRange);
		return true;
	case RHEngine::RHImageClearType::Depth:
		break;
	case RHEngine::RHImageClearType::Stencil:
		break;
	case RHEngine::RHImageClearType::DepthStencil:
		break;
	default:
		break;
	}
	return false;
}

bool RHEngine::VulkanRenderer::BeginCommandList(void * cmdList)
{
	if (cmdList == nullptr)
	{
        
		vkResetCommandBuffer(m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer], 0);
		VkCommandBufferBeginInfo mainCmdBufferBeginInfo{};
		mainCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (!CALL_VK_API(vkBeginCommandBuffer(m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer], &mainCmdBufferBeginInfo),
			L"Failed to begin command list writing!"))
			return false;

		

		return true;
	}
	return false;
}

bool RHEngine::VulkanRenderer::EndCommandList(void * cmdList)
{
	if (cmdList == nullptr)
	{
		if (!CALL_VK_API(vkEndCommandBuffer(m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer]),
			L"Failed to end command list writing!"))
			return false;
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_vkSwapChainImageSemaphore;
		submitInfo.pSignalSemaphores = &m_vkMainCmdBufferSemaphore;
		submitInfo.pCommandBuffers = &m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer];
		VkPipelineStageFlags flags = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		submitInfo.pWaitDstStageMask = &flags;
		vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, m_vkMainCmdBufferFences[m_nCurrentMainCmdBuffer]);
		return true;
	}
	return false;
}

bool RHEngine::VulkanRenderer::RequestSwapChainImage(void * frameBuffer)
{
	VulkanBackBuffer* backBuffer = reinterpret_cast<VulkanBackBuffer*>(frameBuffer);
	if (backBuffer == nullptr) return false;

    vkWaitForFences(m_vkDevice, 1, &m_vkMainCmdBufferFences[m_nCurrentMainCmdBuffer], true, UINT64_MAX);
    vkResetFences(m_vkDevice, 1, &m_vkMainCmdBufferFences[m_nCurrentMainCmdBuffer]);

	uint32_t currentBackBufferId;

	vkAcquireNextImageKHR(m_vkDevice, m_vkSwapChain, UINT64_MAX, m_vkSwapChainImageSemaphore, VK_NULL_HANDLE, &currentBackBufferId);

	m_nCurrentMainCmdBuffer = currentBackBufferId;
	backBuffer->SetBackBufferID(currentBackBufferId);
	
	return true;
}

bool RHEngine::VulkanRenderer::PresentSwapChainImage(void * frameBuffer)
{
	VulkanBackBuffer* backBuffer = reinterpret_cast<VulkanBackBuffer*>(frameBuffer);
	if (backBuffer == nullptr) return false;

	auto backBufferImg = backBuffer->GetImage();

	VkImageMemoryBarrier presentationBarrier{};
	presentationBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	presentationBarrier.image = backBufferImg;
	presentationBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	presentationBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	presentationBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	presentationBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	presentationBarrier.srcQueueFamilyIndex = m_aGraphicsQueueFamilies[0];
	presentationBarrier.dstQueueFamilyIndex = m_aGraphicsQueueFamilies[0];

	VkImageSubresourceRange subresRange{};
	subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresRange.levelCount = 1;
	subresRange.layerCount = 1;
	presentationBarrier.subresourceRange = subresRange;

	// transform image to required layout
	vkCmdPipelineBarrier(m_vkMainCommandBufferList[m_nCurrentMainCmdBuffer], 
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentationBarrier);

	return true;
}
