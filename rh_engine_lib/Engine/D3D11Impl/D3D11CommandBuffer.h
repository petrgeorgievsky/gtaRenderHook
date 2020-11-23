#pragma once
#include "Engine/Common/ICommandBuffer.h"

// d3d11 struct forwards:
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11CommandList;

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

    void SetViewports( uint32_t                    start_id,
                       const ArrayProxy<ViewPort> &viewports ) override;
    void SetScissors( uint32_t                   start_id,
                      const ArrayProxy<Scissor> &scissors ) override;
    void BindPipeline( IPipeline *pipeline ) override;
    void BindVertexBuffers(
        uint32_t                               start_id,
        const ArrayProxy<VertexBufferBinding> &buffers ) override;
    void BindIndexBuffer( uint32_t offset, IBuffer *buffer,
                          IndexType type ) override;
    void Draw( uint32_t vertex_count, uint32_t instance_count,
               uint32_t first_vertex, uint32_t first_instance ) override;
    void DrawIndexed( uint32_t index_count, uint32_t instance_count,
                      uint32_t first_index, uint32_t first_vertex,
                      uint32_t first_instance ) override;
    void CopyImageToImage( const ImageToImageCopyInfo &copy_info ) override;

  private:
    ID3D11DeviceContext *mContext = nullptr;
    ID3D11CommandList *  mCmdList = nullptr;

    // Унаследовано через ICommandBuffer
    virtual void
    BindDescriptorSets( const DescriptorSetBindInfo &bind_info ) override;

    // Унаследовано через ICommandBuffer
    virtual void
    CopyBufferToImage( const BufferToImageCopyInfo &copy_info ) override;

    // Унаследовано через ICommandBuffer
    virtual void PipelineBarrier( const PipelineBarrierInfo &info ) override;
};
} // namespace rh::engine
