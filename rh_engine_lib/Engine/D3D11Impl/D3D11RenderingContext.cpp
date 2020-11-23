#include "D3D11RenderingContext.h"
#include "Buffers/D3D11Buffer.h"
#include "D3D11InputLayout.h"
#include "Engine/Common/types/image_bind_type.h"
#include "Engine/Common/types/image_clear_type.h"
#include "Engine/Common/types/index_ptr_pair.h"
#include "Engine/Common/types/primitive_type.h"
#include "Engine/Common/types/shader_stage.h"
#include "Engine/Common/types/viewport.h"
#include "Shaders/D3D11Shader.h"
#include <array>
#include <d3d11.h>

using namespace rh::engine;

D3D11RenderingContext::~D3D11RenderingContext()
{
    if ( m_pContextImpl ) {
        m_pContextImpl->Release();
        m_pContextImpl = nullptr;
    }
}

void D3D11RenderingContext::Init( ID3D11DeviceContext *context, IGPUAllocator *allocator )
{
    m_pContextImpl = context;
    m_renderStateCache.Init( allocator );
}

void D3D11RenderingContext::BindViewPorts( const std::vector<ViewPort> &viewports )
{
    m_pContextImpl->RSSetViewports( static_cast<UINT>( viewports.size() ),
                                    reinterpret_cast<const D3D11_VIEWPORT *>( viewports.data() ) );
}

bool D3D11RenderingContext::BindImageBuffers( ImageBindType bindType,
                                              const std::vector<IndexPtrPair> &buffers )
{
    auto rt_cache = m_renderStateCache.GetRTCache();
    auto sr_cache = m_renderStateCache.GetSRCache();
    switch ( bindType ) {
    case ImageBindType::RenderTarget:
        rt_cache->SetRenderTargets( buffers );
        m_renderStateCache.FlushRenderTargets( m_pContextImpl );
        return true;
    case ImageBindType::DepthStencilTarget:
        rt_cache->SetDepthStencilTarget( buffers[0].ptr );
        m_renderStateCache.FlushRenderTargets( m_pContextImpl );
        return true;
    case ImageBindType::UnorderedAccessTarget:
        sr_cache->SetUnorderedAccessResources( buffers, ShaderStage::Compute );
        return true;
    case ImageBindType::CSResource:
        sr_cache->SetShaderResources( buffers, ShaderStage::Compute );
        return true;
    case ImageBindType::PSResource:
        sr_cache->SetShaderResources( buffers, ShaderStage::Pixel );
        return true;
    case ImageBindType::DSResource:
        sr_cache->SetShaderResources( buffers, ShaderStage::Domain );
        return true;
    case ImageBindType::HSResource:
        sr_cache->SetShaderResources( buffers, ShaderStage::Hull );
        return true;
    case ImageBindType::GSResource:
        sr_cache->SetShaderResources( buffers, ShaderStage::Geometry );
        return true;
    case ImageBindType::VSResource:
        sr_cache->SetShaderResources( buffers, ShaderStage::Vertex );
        return true;
    default:
        return false;
    }
}

bool rh::engine::D3D11RenderingContext::ClearImageBuffer( ImageClearType clearType,
                                                          void *buffer,
                                                          const std::array<float, 4> &clearColor )
{
    switch ( clearType ) {
    case ImageClearType::Color: {
        auto *rtv = reinterpret_cast<D3D11BindableResource *>( buffer );
        if ( rtv == nullptr )
            return false;
        auto rtv_impl = rtv->GetRenderTargetView();
        // if ( rtv_impl == nullptr )
        //    return false;
        m_pContextImpl->ClearRenderTargetView( rtv_impl, clearColor.data() );
        return true;
    }
    case ImageClearType::Depth: {
        auto *dsv = reinterpret_cast<D3D11BindableResource *>( buffer );
        if ( dsv == nullptr )
            return false;

        // TODO: REVISIT PARAMS
        m_pContextImpl->ClearDepthStencilView( dsv->GetDepthStencilView(),
                                               D3D11_CLEAR_DEPTH,
                                               1.0F,
                                               0 );
        return true;
    }
    default:
        return false;
    }
}

void rh::engine::D3D11RenderingContext::FlushCache()
{
    m_renderStateCache.Flush( m_pContextImpl );
}

void rh::engine::D3D11RenderingContext::RecordDrawCall( uint32_t vertexCount, uint32_t startVertex )
{
    m_pContextImpl->Draw( vertexCount, startVertex );
}

void rh::engine::D3D11RenderingContext::BindSamplers(
    const std::vector<IndexPtrPair> &sampler_states, ShaderStage stage )
{
    m_renderStateCache.GetSamplerStateCache()->SetSamplerState( sampler_states, stage );
}

void rh::engine::D3D11RenderingContext::SetPrimitiveTopology( PrimitiveType type )
{
    switch ( type ) {
    case PrimitiveType::Unknown:
        break;
    case PrimitiveType::LineList:
        m_pContextImpl->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
        break;
    case PrimitiveType::LineStrip:
        m_pContextImpl->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP );
        break;
    case PrimitiveType::TriangleFan:
    case PrimitiveType::TriangleList:
        m_pContextImpl->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        break;
    case PrimitiveType::TriangleStrip:
        m_pContextImpl->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
        break;
    case PrimitiveType::PointList:
        m_pContextImpl->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
        break;
    }
}

void rh::engine::D3D11RenderingContext::BindVertexBuffers( const std::vector<void *> &buffers,
                                                           uint32_t *strides,
                                                           uint32_t *offsets )
{
    std::vector<ID3D11Buffer *> internal_buffers;
    internal_buffers.reserve( buffers.size() );

    // Convert buffers to internal buffers
    for ( void *buffer_ptr : buffers ) {
        auto *d3d11buf = reinterpret_cast<D3D11BufferOld *>( buffer_ptr );
        internal_buffers.push_back( d3d11buf ? d3d11buf->GetBuffer() : nullptr );
    }

    m_pContextImpl->IASetVertexBuffers( 0,
                                        internal_buffers.size(),
                                        internal_buffers.data(),
                                        strides,
                                        offsets );
}

bool rh::engine::D3D11RenderingContext::BindIndexBuffer( void *buffer )
{
    auto *d3d11buf = reinterpret_cast<D3D11BufferOld *>( buffer );
    // TODO: add ability to use 32bit uint
    m_pContextImpl->IASetIndexBuffer( d3d11buf ? d3d11buf->GetBuffer() : nullptr,
                                      DXGI_FORMAT_R16_UINT,
                                      0 );
    return true;
}

bool rh::engine::D3D11RenderingContext::BindInputLayout( void *layout )
{
    auto *input_layout = reinterpret_cast<D3D11InputLayout *>( layout );
    if ( input_layout )
        input_layout->Set( m_pContextImpl );
    else
        m_pContextImpl->IASetInputLayout( nullptr );
    return true;
}

void rh::engine::D3D11RenderingContext::BindShader( void *shader )
{
    auto *shader_interface = reinterpret_cast<D3D11ShaderOld *>( shader );
    shader_interface->Set( m_pContextImpl );
}

void rh::engine::D3D11RenderingContext::UpdateBuffer( void *buffer, const void *data, int32_t size )
{
    auto *d3d11buf = reinterpret_cast<D3D11BufferOld *>( buffer );
    d3d11buf->Update( m_pContextImpl, data, size );
}

rh::engine::D3D11RenderStateCache *rh::engine::D3D11RenderingContext::GetStateCache()
{
    return &m_renderStateCache;
}

void rh::engine::D3D11RenderingContext::ResetShader( void *shader )
{
    auto *shader_interface = reinterpret_cast<D3D11ShaderOld *>( shader );
    shader_interface->ReSet( m_pContextImpl );
}

void rh::engine::D3D11RenderingContext::FinishCmdList( void *&cmdlist )
{
    ID3D11CommandList *res_ptr = nullptr;
    m_pContextImpl->FinishCommandList( false, &res_ptr );
    m_renderStateCache.Invalidate();
    cmdlist = res_ptr;
}

void rh::engine::D3D11RenderingContext::ReplayCmdList( void *cmdlist )
{
    auto *res_ptr = static_cast<ID3D11CommandList *>( cmdlist );
    m_pContextImpl->ExecuteCommandList( res_ptr, true );
    res_ptr->Release();
}

void rh::engine::D3D11RenderingContext::DispatchThreads( uint32_t x, uint32_t y, uint32_t z )
{
    m_pContextImpl->Dispatch( x, y, z );
}

void rh::engine::D3D11RenderingContext::ReleaseResources()
{
    m_pContextImpl->Flush();
}

bool rh::engine::D3D11RenderingContext::BindConstantBuffers(
    uint8_t bindStages, const std::vector<IndexPtrPair> &buffers )
{
    uint32_t min_slot = buffers[0].id;
    uint32_t size = buffers.size();
    std::vector<ID3D11Buffer *> internal_buffers;

    for ( const IndexPtrPair &pair : buffers ) {
        auto *d3d11buf = reinterpret_cast<D3D11BufferOld *>( pair.ptr );
        internal_buffers.push_back( d3d11buf->GetBuffer() );
    }
    if ( bindStages & ShaderStage::Compute )
        m_pContextImpl->CSSetConstantBuffers( min_slot, size, internal_buffers.data() );
    if ( bindStages & ShaderStage::Domain )
        m_pContextImpl->DSSetConstantBuffers( min_slot, size, internal_buffers.data() );
    if ( bindStages & ShaderStage::Geometry )
        m_pContextImpl->GSSetConstantBuffers( min_slot, size, internal_buffers.data() );
    if ( bindStages & ShaderStage::Hull )
        m_pContextImpl->HSSetConstantBuffers( min_slot, size, internal_buffers.data() );
    if ( bindStages & ShaderStage::Pixel )
        m_pContextImpl->PSSetConstantBuffers( min_slot, size, internal_buffers.data() );
    if ( bindStages & ShaderStage::Vertex )
        m_pContextImpl->VSSetConstantBuffers( min_slot, size, internal_buffers.data() );
    return false;
}

void rh::engine::D3D11RenderingContext::RecordDrawCallIndexed( uint32_t indexCount,
                                                               uint32_t startIndex,
                                                               uint32_t baseVertex )
{
    m_pContextImpl->DrawIndexed( indexCount, startIndex, static_cast<int>( baseVertex ) );
}

void rh::engine::D3D11RenderingContext::CopyImageBuffer( void *dest_buffer,
                                                         void *src_buffer,
                                                         uint32_t dest_subres_id,
                                                         uint32_t dest_pos_x,
                                                         uint32_t dest_pos_y,
                                                         uint32_t dest_pos_z )
{
    auto dest_res = static_cast<IGPUResource *>( dest_buffer );
    auto src_res = static_cast<IGPUResource *>( src_buffer );
    m_pContextImpl
        ->CopySubresourceRegion( static_cast<ID3D11Resource *>( dest_res->GetImplResource() ),
                                 dest_subres_id,
                                 dest_pos_x,
                                 dest_pos_y,
                                 dest_pos_z,
                                 static_cast<ID3D11Resource *>( src_res->GetImplResource() ),
                                 0,
                                 nullptr );
}
