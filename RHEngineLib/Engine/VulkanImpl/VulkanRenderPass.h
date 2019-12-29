#pragma once
#include "Engine/Common/IRenderPass.h"
#include <common.h>

namespace rh::engine
{

struct VulkanRenderPassCreateInfo
{
    // Dependencies...
    vk::Device mDevice;
    // Params...
    RenderPassCreateParams mDesc;
};

class VulkanRenderPass : public IRenderPass
{
  public:
    VulkanRenderPass( const VulkanRenderPassCreateInfo &create_info );
    ~VulkanRenderPass() override;

    operator vk::RenderPass();

  private:
    vk::RenderPass m_vkRenderPass;
    vk::Device     m_vkDevice;
};

} // namespace rh::engine
