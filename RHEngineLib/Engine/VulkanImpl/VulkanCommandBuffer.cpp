#include "VulkanCommandBuffer.h"
#include "SyncPrimitives/VulkanCPUSyncPrimitive.h"
#include "VulkanBuffer.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"

using namespace rh::engine;

VulkanCommandBuffer::VulkanCommandBuffer( vk::Device        device,
                                          vk::CommandBuffer cmd_buffer )
    : m_vkCmdBuffer( cmd_buffer ),
      m_executionSyncPrim( new VulkanCPUSyncPrimitive( device ) )
{
}

ISyncPrimitive *VulkanCommandBuffer::ExecutionFinishedPrimitive()
{
    return m_executionSyncPrim;
}

void VulkanCommandBuffer::BeginRecord()
{
    vk::CommandBufferBeginInfo begin_info{};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    m_vkCmdBuffer.begin( begin_info );
}

void VulkanCommandBuffer::EndRecord() { m_vkCmdBuffer.end(); }

// TODO: Pass Params!!!!
void VulkanCommandBuffer::BeginRenderPass( const RenderPassBeginInfo &params )
{
    vk::RenderPassBeginInfo begin_info{};
    begin_info.renderPass =
        *dynamic_cast<VulkanRenderPass *>( params.m_pRenderPass );
    begin_info.framebuffer =
        dynamic_cast<VulkanFrameBuffer *>( params.m_pFrameBuffer )->GetImpl();

    std::vector<vk::ClearValue> clear_values;
    clear_values.reserve( params.m_aClearValues.size() );

    // Convert clear params...
    std::transform(
        params.m_aClearValues.begin(), params.m_aClearValues.end(),
        std::back_inserter( clear_values ), []( const ClearValue &cv ) {
            vk::ClearValue cv_res;
            if ( cv.type == ClearValueType::Color )
            {
                cv_res.color = vk::ClearColorValue( std::array<float, 4>{
                    ( cv.color.r / 255.0f ), ( cv.color.g / 255.0f ),
                    ( cv.color.b / 255.0f ), ( cv.color.a / 255.0f )} );
            }
            else
            {
                cv_res.depthStencil = vk::ClearDepthStencilValue(
                    cv.depthStencil.depth, cv.depthStencil.stencil );
            }

            return cv_res;
        } );

    auto frame_buffer_info              = params.m_pFrameBuffer->GetInfo();
    begin_info.pClearValues             = clear_values.data();
    begin_info.clearValueCount          = clear_values.size();
    begin_info.renderArea.extent.width  = frame_buffer_info.width;
    begin_info.renderArea.extent.height = frame_buffer_info.height;
    m_vkCmdBuffer.beginRenderPass( begin_info, vk::SubpassContents::eInline );
}

void VulkanCommandBuffer::EndRenderPass() { m_vkCmdBuffer.endRenderPass(); }

void VulkanCommandBuffer::BindPipeline( IPipeline *pipeline )
{
    auto         vk_pipe      = dynamic_cast<VulkanPipeline *>( pipeline );
    vk::Pipeline vk_pipe_impl = *vk_pipe;
    m_vkCmdBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics,
                                vk_pipe_impl );
}

void VulkanCommandBuffer::BindVertexBuffers(
    uint32_t start_id, const std::vector<VertexBufferBinding> &buffers )
{
    std::vector<vk::Buffer>     vertex_buffers;
    std::vector<vk::DeviceSize> vertex_buffer_offsets;
    for ( auto buffer_binding : buffers )
    {
        vertex_buffers.push_back(
            *dynamic_cast<VulkanBuffer *>( buffer_binding.mBuffer ) );
        vertex_buffer_offsets.push_back( buffer_binding.mOffset );
    }

    m_vkCmdBuffer.bindVertexBuffers( start_id, vertex_buffers,
                                     vertex_buffer_offsets );
}

void VulkanCommandBuffer::Draw( uint32_t vertex_count, uint32_t instance_count,
                                uint32_t first_vertex, uint32_t first_instance )
{
    m_vkCmdBuffer.draw( vertex_count, instance_count, first_vertex,
                        first_instance );
}

void VulkanCommandBuffer::SetViewports( uint32_t                     start_id,
                                        const std::vector<ViewPort> &viewports )
{
    std::vector<vk::Viewport> vk_viewports;
    std::transform( viewports.begin(), viewports.end(),
                    std::back_inserter( vk_viewports ),
                    []( const ViewPort &viewport ) {
                        vk::Viewport vk_vp{};
                        vk_vp.x        = viewport.topLeftX;
                        vk_vp.y        = viewport.topLeftY;
                        vk_vp.width    = viewport.width;
                        vk_vp.height   = viewport.height;
                        vk_vp.maxDepth = viewport.maxDepth;
                        vk_vp.minDepth = viewport.minDepth;
                        return vk_vp;
                    } );

    m_vkCmdBuffer.setViewport( start_id, vk_viewports );
}
void VulkanCommandBuffer::SetScissors( uint32_t                    start_id,
                                       const std::vector<Scissor> &scissors )
{

    std::vector<vk::Rect2D> vk_scissors;
    std::transform( scissors.begin(), scissors.end(),
                    std::back_inserter( vk_scissors ),
                    []( const Scissor &scissor ) {
                        vk::Rect2D vk_sc{};
                        vk_sc.offset.x      = scissor.offset_x;
                        vk_sc.offset.y      = scissor.offset_y;
                        vk_sc.extent.width  = scissor.size_x;
                        vk_sc.extent.height = scissor.size_y;
                        return vk_sc;
                    } );

    m_vkCmdBuffer.setScissor( start_id, vk_scissors );
}