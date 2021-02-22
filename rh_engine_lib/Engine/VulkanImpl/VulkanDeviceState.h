#pragma once
#include "Engine/Common/ICommandBuffer.h"
#include "Engine/Common/IDeviceState.h"
#include "Engine/Common/IFrameBuffer.h"
#include "Engine/Common/IImageView.h"
#include "Engine/Common/IRenderPass.h"
#include "Engine/Common/ISyncPrimitive.h"
#include "Engine/Common/IWindow.h"
#include "Engine/VulkanImpl/SyncPrimitives/VulkanCPUSyncPrimitive.h"
#include "Engine/VulkanImpl/VulkanBottomLevelAccelerationStructure.h"
#include "Engine/VulkanImpl/VulkanMemoryAllocator.h"
#include "Engine/VulkanImpl/VulkanRayTracingPipeline.h"
#include "Engine/VulkanImpl/VulkanTopLevelAccelerationStructure.h"
#include "VulkanComputePipeline.h"
#include "VulkanGPUInfo.h"
#include "VulkanImGUI.h"

#include <common.h>

#include <vk_mem_alloc.h>

namespace rh::engine
{
struct DisplayInfo
{
    String                       m_sDisplayName;
    std::vector<DisplayModeInfo> m_aDisplayModes;
};

class VulkanDeviceState : public IDeviceState
{
    // IDeviceState interface
  public:
    VulkanDeviceState();
    ~VulkanDeviceState() override;
    bool Init() override;
    bool Shutdown() override;
    bool GetAdaptersCount( unsigned int &count ) override;
    bool GetAdapterInfo( unsigned int id, String &info ) override;
    bool GetCurrentAdapter( unsigned int &id ) override;
    bool SetCurrentAdapter( unsigned int id ) override;
    bool GetOutputCount( unsigned int adapterId, unsigned int &count ) override;
    bool GetOutputInfo( unsigned int id, String &info ) override;
    bool GetCurrentOutput( unsigned int &id ) override;
    bool SetCurrentOutput( unsigned int id ) override;
    bool GetDisplayModeCount( unsigned int  outputId,
                              unsigned int &count ) override;
    bool GetDisplayModeInfo( unsigned int id, DisplayModeInfo &info ) override;
    bool GetCurrentDisplayMode( unsigned int &id ) override;
    bool SetCurrentDisplayMode( unsigned int id ) override;

    IDescriptorSetLayout *CreateDescriptorSetLayout(
        const DescriptorSetLayoutCreateParams &params ) override;
    IDescriptorSetAllocator *CreateDescriptorSetAllocator(
        const DescriptorSetAllocatorCreateParams &params ) override;
    IPipelineLayout *
    CreatePipelineLayout( const PipelineLayoutCreateParams &params ) override;

    IWindow *CreateDeviceWindow( HWND window, const OutputInfo &info ) override;

    ISyncPrimitive *CreateSyncPrimitive( SyncPrimitiveType type ) override;

    // TODODO: ADD PARAMS!!!!
    IRenderPass *
    CreateRenderPass( const RenderPassCreateParams &params ) override;

    IFrameBuffer *
    CreateFrameBuffer( const FrameBufferCreateParams &params ) override;

    IPipeline *
    CreateRasterPipeline( const RasterPipelineCreateParams &params ) override;

    ICommandBuffer *GetMainCommandBuffer() override;
    ICommandBuffer *CreateCommandBuffer() override;
    IShader *       CreateShader( const ShaderDesc &params ) override;

    IBuffer *CreateBuffer( const BufferCreateInfo &params ) override;
    IImageBuffer *
    CreateImageBuffer( const ImageBufferCreateParams &params ) override;
    virtual ISampler *CreateSampler( const SamplerDesc &params ) override;
    virtual IImageView *
    CreateImageView( const ImageViewCreateInfo &params ) override;

    virtual void
    UpdateDescriptorSets( const DescriptorSetUpdateInfo &params ) override;

    // Executes the command buffer on GPU, waits for waitFor sync primitive and
    // signals to signal sync primitive after execution
    void ExecuteCommandBuffer( ICommandBuffer *buffer, ISyncPrimitive *waitFor,
                               ISyncPrimitive *signal ) override;
    virtual void DispatchToGPU(
        const ArrayProxy<CommandBufferSubmitInfo> &buffers ) override;

    void WaitForGPU() override;
    void Wait( const ArrayProxy<ISyncPrimitive *> &primitiveList ) override;

    VulkanBottomLevelAccelerationStructure *
    CreateBLAS( const AccelerationStructureCreateInfo &create_info );
    VulkanTopLevelAccelerationStructure *
    CreateTLAS( const TLASCreateInfo &create_info );
    VulkanRayTracingPipeline *
    CreateRayTracingPipeline( const RayTracingPipelineCreateInfo &params );
    VulkanComputePipeline *
    CreateComputePipeline( const ComputePipelineCreateParams &params );
    VulkanImGUI *           CreateImGUI( IWindow *wnd );
    const DeviceLimitsInfo &GetLimits() override;

  private:
    // Vulkan renderer instance - used to work with platform-specific stuff
    vk::Instance m_vkInstance;

    // Avaliable physical devices
    std::vector<vk::PhysicalDevice> m_aAdapters;

    // Physical devices info
    std::vector<VulkanGPUInfo> m_aAdaptersInfo;

    // Vulkan debug callback
    [[maybe_unused]] VkDebugUtilsMessengerEXT m_debugCallback{};

    // Instance extension list
    std::vector<const char *> m_aExtensions;

    // Instance layer list
    std::vector<const char *> m_aLayers;

    vk::Device m_vkDevice = nullptr;

    vk::Queue                  m_vkMainQueue = nullptr;
    [[maybe_unused]] vk::Queue m_vkCopyQueue = nullptr;

    vk::CommandPool m_vkCommandPool = nullptr;

    std::vector<DisplayInfo> m_aDisplayInfos;

    uint32_t m_uiCurrentAdapter = 0, m_uiCurrentOutput = 0,
             m_uiCurrentAdapterMode = 0;

    uint32_t                  m_iGraphicsQueueFamilyIdx = 0;
    [[maybe_unused]] uint32_t m_iCopyQueueFamilyIdx     = 0;

    ICommandBuffer *       mMainCmdBuffer    = nullptr;
    VulkanMemoryAllocator *mDefaultAllocator = nullptr;
    vk::DynamicLoader      dl;
};
} // namespace rh::engine
