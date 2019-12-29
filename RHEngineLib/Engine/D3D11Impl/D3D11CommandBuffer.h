#pragma once
#include "Engine/Common/ICommandBuffer.h"
#include <common.h>
namespace rh::engine
{
struct D3D11CommandBufferCreateParams
{
    // Dependencies...
    ID3D11Device *mDevice;
    // Params
};
class D3D11CommandBuffer : public ICommandBuffer
{
  public:
    D3D11CommandBuffer( const D3D11CommandBufferCreateParams &create_params );
    ~D3D11CommandBuffer() override;
    void BeginRecord() override;
    void EndRecord() override;
    void BeginRenderPass( const RenderPassBeginInfo &params ) override;
    void EndRenderPass() override;
    ISyncPrimitive *   ExecutionFinishedPrimitive() override;
    ID3D11CommandList *GetCmdList() const { return mCmdList; }

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

  private:
    ID3D11DeviceContext *mContext = nullptr;
    ID3D11CommandList *  mCmdList = nullptr;
};
} // namespace rh::engine