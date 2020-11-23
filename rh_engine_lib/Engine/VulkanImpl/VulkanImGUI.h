//
// Created by peter on 12.07.2020.
//
#pragma once
#include <Engine/Common/ICommandBuffer.h>
#include <Engine/Common/IRenderPass.h>
#include <common.h>

namespace rh::engine
{
struct VulkanImGUIStartParams
{
    HWND             mWindow;
    VkInstance       Instance;
    VkPhysicalDevice PhysicalDevice;
    VkDevice         Device;
    uint32_t         QueueFamily;
    VkQueue          Queue;
};
struct VulkanImGUIInitParams
{
    IRenderPass *mRenderPass;
};
class VulkanImGUI
{
  public:
    VulkanImGUI( const VulkanImGUIStartParams &params );
    ~VulkanImGUI();
    void Init( const VulkanImGUIInitParams &init );
    bool UploadFonts( ICommandBuffer *cmd_buff );
    void BeginFrame();
    void DrawGui( ICommandBuffer *cmd_buff );

  private:
    VkInstance       mInstance;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice         mDevice;
    uint32_t         mQueueFamily;
    VkQueue          mQueue;
    VkDescriptorPool mDescriptorPool;
    bool             mFontsUploaded = false;
};

} // namespace rh::engine