#include "D3D11DepthStencilStateCache.h"
#include <Engine/Common/IGPUAllocator.h>
#include <Engine/Common/types/comparison_func.h>
#include <Engine/Common/types/stencil_op.h>
#include <d3d11_3.h>

rh::engine::D3D11DepthStencilStateCache::D3D11DepthStencilStateCache() noexcept {}

rh::engine::D3D11DepthStencilStateCache::~D3D11DepthStencilStateCache() noexcept
{
    for ( auto [key, ptr] : m_aInternalStateData ) {
        if ( ptr )
            reinterpret_cast<ID3D11DepthStencilState *>( ptr )->Release();
    }
    m_aInternalStateData.clear();
}

void rh::engine::D3D11DepthStencilStateCache::SetDepthEnable( bool enable )
{
    if ( m_depthStencilDescription.enableDepthBuffer != enable ) {
        m_depthStencilDescription.enableDepthBuffer = enable;
        MakeDirty();
    }
}

void rh::engine::D3D11DepthStencilStateCache::SetDepthWriteEnable( bool enable )
{
    if ( m_depthStencilDescription.enableDepthWrite != enable ) {
        m_depthStencilDescription.enableDepthWrite = enable;
        MakeDirty();
    }
}

void rh::engine::D3D11DepthStencilStateCache::Init( IGPUAllocator *allocator )
{
    m_pAllocator = allocator;
    m_depthStencilDescription = {};
    m_depthStencilDescription.enableDepthBuffer = true;
    m_depthStencilDescription.enableDepthWrite = true;
    m_depthStencilDescription.enableStencilBuffer = true;
    m_depthStencilDescription.depthComparisonFunc = ComparisonFunc::LessEqual;
    m_depthStencilDescription.stencilReadMask = 0xFF;
    m_depthStencilDescription.stencilWriteMask = 0xFF;

    m_depthStencilDescription.frontFaceStencilOp.stencilFailOp = StencilOp::Keep;
    m_depthStencilDescription.frontFaceStencilOp.stencilDepthFailOp = StencilOp::Incr;
    m_depthStencilDescription.frontFaceStencilOp.stencilPassOp = StencilOp::Keep;
    m_depthStencilDescription.frontFaceStencilOp.stencilFunc = ComparisonFunc::Always;

    m_depthStencilDescription.backFaceStencilOp.stencilFailOp = StencilOp::Keep;
    m_depthStencilDescription.backFaceStencilOp.stencilDepthFailOp = StencilOp::Decr;
    m_depthStencilDescription.backFaceStencilOp.stencilPassOp = StencilOp::Keep;
    m_depthStencilDescription.backFaceStencilOp.stencilFunc = ComparisonFunc::Always;
}

void rh::engine::D3D11DepthStencilStateCache::OnFlush( void *deviceObject )
{
    auto *context = reinterpret_cast<ID3D11DeviceContext *>( deviceObject );
    void *&depthStencilState = m_aInternalStateData[m_depthStencilDescription];
    if ( depthStencilState == nullptr )
        m_pAllocator->AllocateDepthStencilState( m_depthStencilDescription, depthStencilState );
    if ( context )
        context->OMSetDepthStencilState( reinterpret_cast<ID3D11DepthStencilState *>(
                                             depthStencilState ),
                                         0 );
}
