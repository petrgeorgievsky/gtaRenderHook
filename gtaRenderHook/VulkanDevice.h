#ifndef VulkanDevice_h__
#define VulkanDevice_h__
class CVulkanDevice
{
public:
	CVulkanDevice(VkPhysicalDevice gpu);
	~CVulkanDevice();

	void FindGraphicsQueueFamily(VkPhysicalDevice gpu);

	VkDevice	&getDevice()				{ return m_Device; };
	VkQueue		&getGraphicsQueue()			{ return m_GraphicsQueue; };
	uint32_t	getGraphicsQueueFamilyID()	{ return static_cast<uint32_t>(m_QueueFamilyID); };
private:
	VkDevice	m_Device		= VK_NULL_HANDLE;
	VkQueue		m_GraphicsQueue = VK_NULL_HANDLE;
	int32_t	m_QueueFamilyID = 0;

	std::vector<const char *>		m_layers		= {};
	std::vector<const char *>		m_extensions	= {};
};
#endif // VulkanDevice_h__