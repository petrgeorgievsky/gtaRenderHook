#include "VulkanFrameBuffer.h"
#include "VulkanCommon.h"

using namespace rh::engine;

VulkanFrameBuffer::VulkanFrameBuffer( vk::Device                    device,
                                      vk::ArrayProxy<vk::ImageView> attachments,
                                      uint32_t w, uint32_t h,
                                      vk::RenderPass renderPass )
    : m_vkDevice( device )
{
    vk::FramebufferCreateInfo create_info{};
    create_info.pAttachments    = attachments.data();
    create_info.attachmentCount = attachments.size();
    create_info.layers          = 1;
    create_info.width           = w;
    create_info.height          = h;
    create_info.renderPass      = renderPass;
    auto result                 = m_vkDevice.createFramebuffer( create_info );
    if ( !CALL_VK_API( result.result,
                       TEXT( "Failed to create framebuffer!" ) ) )
        return;
    m_vkFrameBuffer = result.value;
    m_rhInfo        = { w, h };
}

VulkanFrameBuffer::~VulkanFrameBuffer()
{
    m_vkDevice.destroyFramebuffer( m_vkFrameBuffer );
}

const FrameBufferInfo &VulkanFrameBuffer::GetInfo() const { return m_rhInfo; }