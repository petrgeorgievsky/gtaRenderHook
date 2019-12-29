#pragma once
#include "Engine/Common/IFrameBuffer.h"
#include <common.h>

namespace rh::engine
{
class VulkanFrameBuffer : public IFrameBuffer
{
  public:
    VulkanFrameBuffer( vk::Device                    device,
                       vk::ArrayProxy<vk::ImageView> attachments, uint32_t w,
                       uint32_t h, vk::RenderPass renderPass );
    ~VulkanFrameBuffer() override;
    const FrameBufferInfo &GetInfo() const override;
    vk::Framebuffer        GetImpl() { return m_vkFrameBuffer; }

  private:
    vk::Framebuffer m_vkFrameBuffer;
    vk::Device      m_vkDevice;
    FrameBufferInfo m_rhInfo;
};
} // namespace rh::engine