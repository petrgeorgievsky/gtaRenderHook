#pragma once
#include "Engine/Common/ICommandBuffer.h"
#include "Engine/Common/IDeviceOutputView.h"
#include "Engine/Common/IDeviceState.h"
#include "Engine/Common/IFrameBuffer.h"
#include "Engine/Common/IImageView.h"
#include "Engine/Common/IRenderPass.h"
#include "Engine/Common/ISyncPrimitive.h"
#include "Engine/Common/IWindow.h"
#include "Engine/VulkanImpl/SyncPrimitives/VulkanCPUSyncPrimitive.h"

#include <common.h>

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
    IDeviceOutputView *
    CreateDeviceOutputView( HWND window, const OutputInfo &info ) override;

    IWindow *CreateDeviceWindow( HWND window, const OutputInfo &info ) override;

    ISyncPrimitive *CreateSyncPrimitive( SyncPrimitiveType type ) override;

    // TODODO: ADD PARAMS!!!!
    IRenderPass *
    CreateRenderPass( const RenderPassCreateParams &params ) override;

    IFrameBuffer *
    CreateFrameBuffer( const FrameBufferCreateParams &params ) override;

    IPipeline *CreatePipeline( const PipelineCreateParams &params ) override;

    ICommandBuffer *GetMainCommandBuffer() override;
    IShader *       CreateShader( const ShaderDesc &params ) override;

    IBuffer *CreateBuffer( const BufferCreateInfo &params ) override;

    // Executes the command buffer on GPU, waits for waitFor sync primitive and
    // signals to signal sync primitive after execution
    void ExecuteCommandBuffer( ICommandBuffer *buffer, ISyncPrimitive *waitFor,
                               ISyncPrimitive *signal ) override;

    void Wait( const std::vector<ISyncPrimitive *> &primitiveList ) override;

  private:
    // Vulkan renderer instance - used to work with platform-specific stuff
    vk::Instance m_vkInstance;

    // Avaliable physical devices
    std::vector<vk::PhysicalDevice> m_aAdapters;

    // Vulkan debug callback
    VkDebugReportCallbackEXT m_debugCallback{};

    // Instance extension list
    std::vector<const char *> m_aExtensions;

    // Instance layer list
    std::vector<const char *> m_aLayers;

    vk::Device m_vkDevice = nullptr;

    vk::Queue m_vkMainQueue = nullptr;

    vk::CommandPool m_vkCommandPool = nullptr;

    std::vector<DisplayInfo> m_aDisplayInfos;

    uint32_t m_uiCurrentAdapter = 0, m_uiCurrentOutput = 0,
             m_uiCurrentAdapterMode = 0;

    uint32_t m_iGraphicsQueueFamilyIdx = 0;

    ICommandBuffer *mMainCmdBuffer = nullptr;
};
} // namespace rh::engine
