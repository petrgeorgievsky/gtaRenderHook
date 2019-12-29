#include "D3D11RenderTargetCache.h"
#include "Engine/Common/types/index_ptr_pair.h"
#include "Engine/D3D11Impl/ImageBuffers/ImageBuffer.h"
#include <common.h>

rh::engine::D3D11RenderTargetCache::D3D11RenderTargetCache() noexcept
{
    for ( auto &m_aRenderTargetView : m_aRenderTargetViews )
        m_aRenderTargetView = nullptr;
    m_pDepthStencilView = nullptr;
}

void rh::engine::D3D11RenderTargetCache::SetRenderTargets(
    const std::vector<IndexPtrPair> &renderTargets, void *depthStencilView )
{
    SetRenderTargets( renderTargets );
    SetDepthStencilTarget( depthStencilView );
}

void rh::engine::D3D11RenderTargetCache::SetRenderTargets(
    const std::vector<IndexPtrPair> &renderTargets )
{
    for ( auto el : renderTargets ) {
        //if ( m_aRenderTargetViews[el.id] != el.ptr ) {
        m_aRenderTargetViews[el.id] = reinterpret_cast<D3D11BindableResource *>( el.ptr );
        //}
    }
    MakeDirty();
}

void rh::engine::D3D11RenderTargetCache::SetDepthStencilTarget( void *depthStencilView )
{
    if ( depthStencilView != m_pDepthStencilView ) {
        m_pDepthStencilView = reinterpret_cast<D3D11BindableResource *>( depthStencilView );
        MakeDirty();
    }
}

void rh::engine::D3D11RenderTargetCache::OnFlush( void *deviceObject )
{
    ID3D11RenderTargetView *rtvArray[8];
    auto *context = reinterpret_cast<ID3D11DeviceContext *>( deviceObject );

    if ( context ) {
        for ( unsigned int i = 0; i < 8; i++ )
            rtvArray[i] = m_aRenderTargetViews[i] ? m_aRenderTargetViews[i]->GetRenderTargetView()
                                                  : nullptr;

        context->OMSetRenderTargets( 8,
                                     rtvArray,
                                     m_pDepthStencilView
                                         ? m_pDepthStencilView->GetDepthStencilView()
                                         : nullptr );
    }
}
