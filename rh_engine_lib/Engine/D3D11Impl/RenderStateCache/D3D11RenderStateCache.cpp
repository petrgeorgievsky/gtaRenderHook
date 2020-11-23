#include "D3D11RenderStateCache.h"


void rh::engine::D3D11RenderStateCache::FlushRenderTargets( ID3D11DeviceContext* context )
{
    m_rtCache.Flush( context );
}

void rh::engine::D3D11RenderStateCache::Flush( ID3D11DeviceContext* context )
{
    FlushRenderTargets( context );
    m_shaderResourceCache.Flush( context );
    m_samplerStateCache.Flush( context );
    m_depthStencilStateCache.Flush( context );
    m_blendStateCache.Flush( context );
    m_rasterizerStateCache.Flush( context );
}

void rh::engine::D3D11RenderStateCache::Init( IGPUAllocator * allocator )
{
    m_depthStencilStateCache.Init( allocator );
    m_blendStateCache.Init( allocator );
    m_rasterizerStateCache.Init( allocator );
}

void rh::engine::D3D11RenderStateCache::Invalidate()
{
    m_rtCache.Invalidate();
    m_shaderResourceCache.Invalidate();
    m_samplerStateCache.Invalidate();
    m_depthStencilStateCache.Invalidate();
    m_blendStateCache.Invalidate();
    m_rasterizerStateCache.Invalidate();
}
