#include "D3D11CommandBuffer.h"
#include "D3D11Buffer.h"
#include "D3D11DescriptorSet.h"
#include "D3D11Framebuffer.h"
#include "D3D11ImageBuffer.h"
#include "D3D11ImageView.h"
#include "D3D11Pipeline.h"
#include "D3D11RenderPass.h"
#include <algorithm>
#include <d3d11.h>
#include <ranges>

using namespace rh::engine;

D3D11CommandBuffer::D3D11CommandBuffer(
    const D3D11CommandBufferCreateParams &create_params )
{
    create_params.mDevice->CreateDeferredContext( 0, &mContext );
}

D3D11CommandBuffer::~D3D11CommandBuffer()
{
    if ( mCmdList )
    {
        mCmdList->Release();
        mCmdList = nullptr;
    }
    if ( mContext )
    {
        mContext->Release();
        mContext = nullptr;
    }
}

void D3D11CommandBuffer::BeginRecord()
{
    if ( mCmdList )
    {
        mCmdList->Release();
        mCmdList = nullptr;
    }
}
void D3D11CommandBuffer::EndRecord()
{
    mCmdList = nullptr;
    mContext->FinishCommandList( false, &mCmdList );
}
void D3D11CommandBuffer::BeginRenderPass( const RenderPassBeginInfo &params )
{
    auto *d3d_fb = dynamic_cast<D3D11Framebuffer *>( params.m_pFrameBuffer );
    auto *d3d_rp = dynamic_cast<D3D11RenderPass *>( params.m_pRenderPass );
    auto  attachments = d3d_rp->Info().mAttachments;
    auto  subpass     = d3d_rp->Info().mSubpasses[0];
    // TODO: add more compatibility to vulkan
    std::vector<ID3D11RenderTargetView *> d3d_rtv_vec;
    for ( const auto &color_attachment : subpass.mColorAttachments )
    {
        int   attachment_id   = color_attachment.mAttachmentId;
        auto  attachment_info = attachments[attachment_id];
        auto *d3d_img_view    = dynamic_cast<D3D11ImageView *>(
            d3d_fb->GetImageView( static_cast<uint8_t>( attachment_id ) ) );
        ID3D11View *img_view = ( d3d_img_view )->GetRTV();
        d3d_rtv_vec.push_back(
            reinterpret_cast<ID3D11RenderTargetView *>( img_view ) );
        if ( attachment_info.mLoadOp == LoadOp::Clear )
        {
            auto                 cv = params.m_aClearValues[attachment_id];
            std::array<float, 4> clear_value{
                ( float( cv.color.r ) / 255.0f ),
                ( float( cv.color.g ) / 255.0f ),
                ( float( cv.color.b ) / 255.0f ),
                ( float( cv.color.a ) / 255.0f ) };

            mContext->ClearRenderTargetView(
                reinterpret_cast<ID3D11RenderTargetView *>( img_view ),
                clear_value.data() );
        }
    }

    ID3D11DepthStencilView *d3d_dsv = nullptr;
    if ( subpass.mDepthStencilAttachment )
    {
        int  attachment_id   = subpass.mDepthStencilAttachment->mAttachmentId;
        auto attachment_info = attachments[attachment_id];
        auto d3d_img_view    = dynamic_cast<D3D11ImageView *>(
            d3d_fb->GetImageView( static_cast<uint8_t>( attachment_id ) ) );
        ID3D11View *img_view = ( d3d_img_view )->GetDSV();
        d3d_dsv = reinterpret_cast<ID3D11DepthStencilView *>( img_view );
        if ( attachment_info.mLoadOp == LoadOp::Clear ||
             attachment_info.mStencilLoadOp == LoadOp::Clear )
        {
            const auto &cv = params.m_aClearValues[attachment_id];

            UINT clear_flags = 0;
            if ( attachment_info.mLoadOp == LoadOp::Clear )
                clear_flags |= uint32_t( D3D11_CLEAR_DEPTH );

            if ( attachment_info.mStencilLoadOp == LoadOp::Clear )
                clear_flags |= uint32_t( D3D11_CLEAR_STENCIL );

            mContext->ClearDepthStencilView( d3d_dsv, clear_flags,
                                             cv.depthStencil.depth,
                                             cv.depthStencil.stencil );
        }
    }
    // Bind Views to OM PipelineState...
    mContext->OMSetRenderTargets( static_cast<UINT>( d3d_rtv_vec.size() ),
                                  d3d_rtv_vec.data(), d3d_dsv );
}

void D3D11CommandBuffer::BindPipeline( IPipeline *pipeline )
{
    dynamic_cast<D3D11Pipeline *>( pipeline )->BindToContext( mContext );
}

void D3D11CommandBuffer::EndRenderPass() {}
void D3D11CommandBuffer::BindVertexBuffers(
    uint32_t start_id, const ArrayProxy<VertexBufferBinding> &buffers )
{
    std::vector<ID3D11Buffer *> vertex_buffers;
    std::vector<uint32_t>       vertex_offsets;
    std::vector<uint32_t>       vertex_strides;

    std::ranges::for_each( buffers, [&]( const VertexBufferBinding &binding ) {
        auto buffer = dynamic_cast<D3D11Buffer *>( binding.mBuffer )->GetImpl();
        vertex_buffers.push_back( buffer );
        vertex_strides.push_back( binding.mStride );
        vertex_offsets.push_back( binding.mOffset );
    } );

    mContext->IASetVertexBuffers( start_id, static_cast<UINT>( buffers.Size() ),
                                  vertex_buffers.data(), vertex_strides.data(),
                                  vertex_offsets.data() );
}

void D3D11CommandBuffer::BindIndexBuffer( uint32_t offset, IBuffer *buffer,
                                          IndexType type )
{
    mContext->IASetIndexBuffer(
        dynamic_cast<D3D11Buffer *>( buffer )->GetImpl(),
        type == IndexType::i16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
        offset );
}

ISyncPrimitive *D3D11CommandBuffer::ExecutionFinishedPrimitive()
{
    return nullptr;
}
void D3D11CommandBuffer::Draw( uint32_t vertex_count, uint32_t instance_count,
                               uint32_t first_vertex, uint32_t first_instance )
{
    if ( instance_count > 1 )
        mContext->DrawInstanced( vertex_count, instance_count, first_vertex,
                                 first_instance );
    else
        mContext->Draw( vertex_count, first_vertex );
}

void D3D11CommandBuffer::DrawIndexed( uint32_t index_count,
                                      uint32_t instance_count,
                                      uint32_t first_index,
                                      uint32_t first_vertex,
                                      uint32_t first_instance )
{
    if ( instance_count > 1 )
        mContext->DrawIndexedInstanced( index_count, instance_count,
                                        first_index, first_vertex,
                                        first_instance );
    else
        mContext->DrawIndexed( index_count, first_index, first_vertex );
}

void rh::engine::D3D11CommandBuffer::BindDescriptorSets(
    const DescriptorSetBindInfo &bind_info )
{
    for ( auto desc_set : bind_info.mDescriptorSets )
    {
        dynamic_cast<D3D11DescriptorSet *>( desc_set )
            ->BindToContext( mContext );
    }
}

void rh::engine::D3D11CommandBuffer::CopyBufferToImage(
    const BufferToImageCopyInfo & /*copy_info*/ )
{
    // mContext->CopySubresourceRegion()
}

void rh::engine::D3D11CommandBuffer::PipelineBarrier(
    const PipelineBarrierInfo & /*info*/ )
{
}

void D3D11CommandBuffer::SetViewports( uint32_t                    start_id,
                                       const ArrayProxy<ViewPort> &viewports )
{
    if ( start_id == 0 )
    {
        std::vector<D3D11_VIEWPORT> vp;

        std::ranges::transform( viewports, std::back_inserter( vp ),
                                []( const ViewPort &vport ) {
                                    D3D11_VIEWPORT view_port{};
                                    view_port.Width    = vport.width;
                                    view_port.Height   = vport.height;
                                    view_port.TopLeftX = vport.topLeftX;
                                    view_port.TopLeftY = vport.topLeftY;
                                    view_port.MaxDepth = vport.maxDepth;
                                    view_port.MinDepth = vport.minDepth;
                                    return view_port;
                                } );

        mContext->RSSetViewports( static_cast<UINT>( vp.size() ), vp.data() );
    }
}

void D3D11CommandBuffer::SetScissors( uint32_t, const ArrayProxy<Scissor> & ) {}
void D3D11CommandBuffer::CopyImageToImage(
    const ImageToImageCopyInfo &copy_info )
{
    auto src_img  = dynamic_cast<D3D11ImageBuffer *>( copy_info.mSrc );
    auto dest_img = dynamic_cast<D3D11ImageBuffer *>( copy_info.mDst );

    // TODO: Implement?
    mContext->CopySubresourceRegion(
        dest_img->GetImpl(), 0, copy_info.mRegions[0].mDest.mOffsetX,
        copy_info.mRegions[0].mDest.mOffsetY,
        copy_info.mRegions[0].mDest.mOffsetZ, src_img->GetImpl(), 0, nullptr );
}
void D3D11CommandBuffer::CopyImageToBuffer(
    const ImageToBufferCopyInfo & /*copy_info*/ )
{
    // TODO: Implement
}
