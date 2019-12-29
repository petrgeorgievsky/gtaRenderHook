#include "D3D11CommandBuffer.h"
#include "D3D11Framebuffer.h"
#include "D3D11ImageView.h"
#include "D3D11RenderPass.h"

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
    D3D11Framebuffer *d3d_fb =
        dynamic_cast<D3D11Framebuffer *>( params.m_pFrameBuffer );
    D3D11RenderPass *d3d_rp =
        dynamic_cast<D3D11RenderPass *>( params.m_pRenderPass );
    auto attachments = d3d_rp->Info().mAttachments;
    auto subpass     = d3d_rp->Info().mSubpasses[0];
    // TODO: add more compatibility to vulkan
    std::vector<ID3D11RenderTargetView *> d3d_rtv_vec;
    for ( auto color_attachment : subpass.mColorAttachments )
    {
        int             attachment_id   = color_attachment.mAttachmentId;
        auto            attachment_info = attachments[attachment_id];
        D3D11ImageView *d3d_img_view    = dynamic_cast<D3D11ImageView *>(
            d3d_fb->GetImageView( attachment_id ) );
        ID3D11View *img_view = ( *d3d_img_view );
        d3d_rtv_vec.push_back(
            reinterpret_cast<ID3D11RenderTargetView *>( img_view ) );
        if ( attachment_info.mLoadOp == LoadOp::Clear )
        {
            auto                 cv = params.m_aClearValues[attachment_id];
            std::array<float, 4> clear_value{
                ( cv.color.r / 255.0f ), ( cv.color.g / 255.0f ),
                ( cv.color.b / 255.0f ), ( cv.color.a / 255.0f )};

            mContext->ClearRenderTargetView(
                reinterpret_cast<ID3D11RenderTargetView *>( img_view ),
                clear_value.data() );
        }
    }

    ID3D11DepthStencilView *d3d_dsv = nullptr;
    if ( subpass.mDepthStencilAttachment.has_value() )
    {
        int  attachment_id   = subpass.mDepthStencilAttachment->mAttachmentId;
        auto attachment_info = attachments[attachment_id];
        D3D11ImageView *d3d_img_view = dynamic_cast<D3D11ImageView *>(
            d3d_fb->GetImageView( attachment_id ) );
        ID3D11View *img_view = ( *d3d_img_view );
        d3d_dsv = reinterpret_cast<ID3D11DepthStencilView *>( img_view );
        if ( attachment_info.mLoadOp == LoadOp::Clear ||
             attachment_info.mStencilLoadOp == LoadOp::Clear )
        {
            auto cv = params.m_aClearValues[attachment_id];

            UINT clear_flags = 0;
            if ( attachment_info.mLoadOp == LoadOp::Clear )
                clear_flags |= D3D11_CLEAR_DEPTH;

            if ( attachment_info.mStencilLoadOp == LoadOp::Clear )
                clear_flags |= D3D11_CLEAR_STENCIL;

            mContext->ClearDepthStencilView( d3d_dsv, clear_flags,
                                             cv.depthStencil.depth,
                                             cv.depthStencil.stencil );
        }
    }
    mContext->OMSetRenderTargets( d3d_rtv_vec.size(), d3d_rtv_vec.data(),
                                  d3d_dsv );
    // Bind Views to OM RenderState...
    // mContext->OMSetRenderTargets()
}

void D3D11CommandBuffer::BindPipeline( IPipeline *pipeline ) {}

void D3D11CommandBuffer::EndRenderPass() {}
void D3D11CommandBuffer::BindVertexBuffers(
    uint32_t /*start_id*/,
    const std::vector<VertexBufferBinding> & /*buffers*/ )
{
}
ISyncPrimitive *D3D11CommandBuffer::ExecutionFinishedPrimitive()
{
    return nullptr;
}
void D3D11CommandBuffer::Draw( uint32_t, uint32_t, uint32_t, uint32_t ) {}

void D3D11CommandBuffer::SetViewports( uint32_t, const std::vector<ViewPort> & )
{
}
void D3D11CommandBuffer::SetScissors( uint32_t, const std::vector<Scissor> & )
{
}