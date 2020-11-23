#pragma once
#include <common.h>

#include "Engine/Common/ICommandBuffer.h"
#include "Engine/Common/IImageView.h"
#include "Engine/Common/IRenderPass.h"
#include "Engine/Common/ISyncPrimitive.h"
#include "Engine/VulkanImpl/VulkanBottomLevelAccelerationStructure.h"
#include "Engine/VulkanImpl/VulkanRayTracingPipeline.h"
#include "Engine/VulkanImpl/VulkanTopLevelAccelerationStructure.h"
#include "VulkanComputePipeline.h"

namespace rh::engine
{

struct VulkanRayDispatch
{
    IBuffer *mRayGenBuffer;
    uint64_t mRayGenOffset;
    IBuffer *mMissBuffer;
    uint64_t mMissOffset;
    uint64_t mMissStride;
    IBuffer *mHitBuffer;
    uint64_t mHitOffset;
    uint64_t mHitStride;
    IBuffer *mCallableBuffer;
    uint64_t mCallableOffset;
    uint64_t mCallableStride;
    uint32_t mX;
    uint32_t mY;
    uint32_t mZ;
};

struct VulkanComputeDispatch
{
    uint32_t mX = 1;
    uint32_t mY = 1;
    uint32_t mZ = 1;
};
class VulkanCommandBuffer : public ICommandBuffer
{
  public:
    VulkanCommandBuffer( vk::Device device, vk::CommandPool pool,
                         vk::CommandBuffer cmd_buffer );
    ~VulkanCommandBuffer() override
    {
        mDevice.freeCommandBuffers( mPool, { m_vkCmdBuffer } );
        delete m_executionSyncPrim;
    }

    vk::CommandBuffer &GetBuffer() { return m_vkCmdBuffer; }

    void BeginRecord() override;
    void EndRecord() override;
    void BeginRenderPass( const RenderPassBeginInfo &params ) override;
    void EndRenderPass() override;

    void SetViewports( uint32_t                    start_id,
                       const ArrayProxy<ViewPort> &viewports ) override;
    void SetScissors( uint32_t                   start_id,
                      const ArrayProxy<Scissor> &scissors ) override;

    void BindPipeline( IPipeline *pipeline ) override;
    virtual void
    BindDescriptorSets( const DescriptorSetBindInfo &bind_info ) override;

    void BindVertexBuffers(
        uint32_t                               start_id,
        const ArrayProxy<VertexBufferBinding> &buffers ) override;
    void            BindIndexBuffer( uint32_t offset, IBuffer *buffer,
                                     IndexType type ) override;
    void            Draw( uint32_t vertex_count, uint32_t instance_count,
                          uint32_t first_vertex, uint32_t first_instance ) override;
    void            DrawIndexed( uint32_t index_count, uint32_t instance_count,
                                 uint32_t first_index, uint32_t first_vertex,
                                 uint32_t first_instance ) override;
    ISyncPrimitive *ExecutionFinishedPrimitive() override;
    virtual void
    CopyBufferToImage( const BufferToImageCopyInfo &copy_info ) override;
    virtual void PipelineBarrier( const PipelineBarrierInfo &info ) override;

    void BuildBLAS( VulkanBottomLevelAccelerationStructure *blas,
                    IBuffer *                               scratch_buffer );

    void BuildTLAS( VulkanTopLevelAccelerationStructure *blas,
                    IBuffer *scratch_buffer, IBuffer *instance_buffer );
    void BindRayTracingPipeline( VulkanRayTracingPipeline *pipeline );
    void BindComputePipeline( VulkanComputePipeline *pipeline );
    void DispatchRays( const VulkanRayDispatch &dispatch );
    void DispatchCompute( const VulkanComputeDispatch &dispatch );
    void CopyImageToImage( const ImageToImageCopyInfo &copy_info ) override;

  private:
    vk::CommandBuffer m_vkCmdBuffer;
    vk::CommandPool   mPool;
    vk::Device        mDevice;

    ISyncPrimitive *m_executionSyncPrim = nullptr;
    friend class VulkanDebugUtils;
};
} // namespace rh::engine
