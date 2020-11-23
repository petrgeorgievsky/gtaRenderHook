#include "D3D11BlendStateCache.h"
#include "Engine/Common/IGPUAllocator.h"
#include <Engine/Common/types/blend_op.h>
#include <d3d11_3.h>
using namespace rh::engine;

D3D11BlendStateCache::D3D11BlendStateCache() noexcept {}

D3D11BlendStateCache::~D3D11BlendStateCache() noexcept
{
    for ( auto [key, ptr] : m_aInternalStateData )
    {
        if ( ptr )
            reinterpret_cast<ID3D11BlendState *>( ptr )->Release();
    }
    m_aInternalStateData.clear();
}

void D3D11BlendStateCache::Init( IGPUAllocator *allocator )
{
    m_pAllocator = allocator;
    m_blendStateDescription.renderTargetBlendState[0].srcBlend =
        BlendOp::SrcAlpha;
    m_blendStateDescription.renderTargetBlendState[0].destBlend =
        BlendOp::InvSrcAlpha;
    m_blendStateDescription.renderTargetBlendState[0].blendCombineOp =
        BlendCombineOp::Add;
    m_blendStateDescription.renderTargetBlendState[0].srcBlendAlpha =
        BlendOp::One;
    m_blendStateDescription.renderTargetBlendState[0].destBlendAlpha =
        BlendOp::Zero;
    m_blendStateDescription.renderTargetBlendState[0].blendAlphaCombineOp =
        BlendCombineOp::Add;
    m_blendStateDescription.renderTargetBlendState[0].enableBlending = true;
}

void D3D11BlendStateCache::SetAlphaTestEnable( bool enable )
{
    if ( alphaTestEnabled != enable )
    {
        alphaTestEnabled = enable;
        MakeDirty();
    }
}

void D3D11BlendStateCache::SetBlendEnable( bool     enable,
                                           uint32_t renderTargetId )
{
    if ( m_blendStateDescription.renderTargetBlendState[renderTargetId]
             .enableBlending != enable )
    {
        m_blendStateDescription.renderTargetBlendState[renderTargetId]
            .enableBlending = enable;
        MakeDirty();
    }
}

void D3D11BlendStateCache::SetSrcBlendOp( BlendOp  blendOp,
                                          uint32_t renderTargetId )
{
    if ( m_blendStateDescription.renderTargetBlendState[renderTargetId]
             .srcBlend != blendOp )
    {
        m_blendStateDescription.renderTargetBlendState[renderTargetId]
            .srcBlend = blendOp;
        MakeDirty();
    }
}

void D3D11BlendStateCache::SetDestBlendOp( BlendOp  blendOp,
                                           uint32_t renderTargetId )
{
    if ( m_blendStateDescription.renderTargetBlendState[renderTargetId]
             .destBlend != blendOp )
    {
        m_blendStateDescription.renderTargetBlendState[renderTargetId]
            .destBlend = blendOp;
        MakeDirty();
    }
}

void D3D11BlendStateCache::OnFlush( void *deviceObject )
{
    auto *context = reinterpret_cast<ID3D11DeviceContext *>( deviceObject );
    if ( alphaTestEnabled )
        m_blendStateDescription.renderTargetBlendState[0].enableBlending = true;
    void *&blendState = m_aInternalStateData[m_blendStateDescription];
    if ( blendState == nullptr )
        m_pAllocator->AllocateBlendState( m_blendStateDescription, blendState );
    if ( context )
        context->OMSetBlendState(
            reinterpret_cast<ID3D11BlendState *>( blendState ), nullptr,
            0xFFFFFFFF );
}
