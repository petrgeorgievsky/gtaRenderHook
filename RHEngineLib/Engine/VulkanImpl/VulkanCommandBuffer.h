#pragma once
#include <common.h>

#include "Engine/Common/ICommandBuffer.h"
#include "Engine/Common/IImageView.h"
#include "Engine/Common/IRenderPass.h"
#include "Engine/Common/ISyncPrimitive.h"

namespace rh::engine
{

class VulkanCommandBuffer : public ICommandBuffer
{
  public:
    VulkanCommandBuffer( vk::Device device, vk::CommandBuffer cmd_buffer );
    ~VulkanCommandBuffer() override { delete m_executionSyncPrim; }

    vk::CommandBuffer GetBuffer() { return m_vkCmdBuffer; }

    void BeginRecord() override;
    void EndRecord() override;
    void BeginRenderPass( const RenderPassBeginInfo &params ) override;
    void EndRenderPass() override;

    void SetViewports( uint32_t                     start_id,
                       const std::vector<ViewPort> &viewports ) override;
    void SetScissors( uint32_t                    start_id,
                      const std::vector<Scissor> &scissors ) override;

    void BindPipeline( IPipeline *pipeline ) override;

    void BindVertexBuffers(
        uint32_t                                start_id,
        const std::vector<VertexBufferBinding> &buffers ) override;
    void Draw( uint32_t vertex_count, uint32_t instance_count,
               uint32_t first_vertex, uint32_t first_instance ) override;

    ISyncPrimitive *ExecutionFinishedPrimitive() override;

  private:
    vk::CommandBuffer m_vkCmdBuffer;
    ISyncPrimitive *  m_executionSyncPrim = nullptr;
};
} // namespace rh::engine