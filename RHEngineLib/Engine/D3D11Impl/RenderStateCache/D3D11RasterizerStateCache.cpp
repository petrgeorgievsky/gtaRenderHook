#include "D3D11RasterizerStateCache.h"
#include <Engine/Common/IGPUAllocator.h>
#include <Engine/Common/types/cull_mode.h>

#include <d3d11_3.h>

rh::engine::D3D11RasterizerStateCache::D3D11RasterizerStateCache() noexcept {}

rh::engine::D3D11RasterizerStateCache::~D3D11RasterizerStateCache() noexcept
{
    for ( auto [key, ptr] : m_aInternalStateData ) {
        if ( ptr )
            reinterpret_cast<ID3D11RasterizerState *>( ptr )->Release();
    }
    m_aInternalStateData.clear();
}

void rh::engine::D3D11RasterizerStateCache::Init( IGPUAllocator *allocator )
{
    m_pAllocator = allocator;
    m_rasterizerDescription = {};
    m_rasterizerDescription.cullMode = CullMode::None;
}

void rh::engine::D3D11RasterizerStateCache::SetCullMode( CullMode mode )
{
    if ( m_rasterizerDescription.cullMode != mode ) {
        m_rasterizerDescription.cullMode = mode;
        MakeDirty();
    }
}

void rh::engine::D3D11RasterizerStateCache::OnFlush( void *deviceObject )
{
    auto *context = reinterpret_cast<ID3D11DeviceContext *>( deviceObject );
    void *&rasterizerState = m_aInternalStateData[m_rasterizerDescription];
    if ( rasterizerState == nullptr )
        m_pAllocator->AllocateRasterizerState( m_rasterizerDescription, rasterizerState );
    if ( context )
        context->RSSetState( reinterpret_cast<ID3D11RasterizerState *>( rasterizerState ) );
}
