#include "D3D11ImageView.h"
#include "D3D11ImageBuffer.h"
#include <d3d11.h>

using namespace rh::engine;

D3D11ImageView::D3D11ImageView(
    const D3D11ImageViewCreateParams &create_params )
{
    // TODO: ADD PARAMS
    if ( create_params.mUsage & rh::engine::ImageViewUsage::RenderTarget )
    {
        create_params.mDevice->CreateRenderTargetView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11RenderTargetView **>( &mRTView ) );
    }
    if ( create_params.mUsage & rh::engine::ImageViewUsage::ShaderResource )
    {
        create_params.mDevice->CreateShaderResourceView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11ShaderResourceView **>( &mSRView ) );
    }
    if ( create_params.mUsage & rh::engine::ImageViewUsage::DepthStencilTarget )
    {
        create_params.mDevice->CreateDepthStencilView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11DepthStencilView **>( &mDSView ) );
    }
    if ( create_params.mUsage & rh::engine::ImageViewUsage::RWTexture )
    {
        create_params.mDevice->CreateUnorderedAccessView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11UnorderedAccessView **>( &mUAView ) );
    }
}

D3D11ImageView::D3D11ImageView( const D3D11ImageViewCreateInfo &create_params )
{
    auto resource =
        static_cast<D3D11ImageBuffer *>( create_params.mBuffer )->GetImpl();
    if ( create_params.mUsage & rh::engine::ImageViewUsage::RenderTarget )
    {
        create_params.mDevice->CreateRenderTargetView(
            resource, nullptr,
            reinterpret_cast<ID3D11RenderTargetView **>( &mRTView ) );
    }
    if ( create_params.mUsage & rh::engine::ImageViewUsage::ShaderResource )
    {
        create_params.mDevice->CreateShaderResourceView(
            resource, nullptr,
            reinterpret_cast<ID3D11ShaderResourceView **>( &mSRView ) );
    }
    if ( create_params.mUsage & rh::engine::ImageViewUsage::DepthStencilTarget )
    {
        create_params.mDevice->CreateDepthStencilView(
            resource, nullptr,
            reinterpret_cast<ID3D11DepthStencilView **>( &mDSView ) );
    }
    if ( create_params.mUsage & rh::engine::ImageViewUsage::RWTexture )
    {
        create_params.mDevice->CreateUnorderedAccessView(
            resource, nullptr,
            reinterpret_cast<ID3D11UnorderedAccessView **>( &mUAView ) );
    }
}

D3D11ImageView::~D3D11ImageView()
{
    if ( mRTView )
    {
        mRTView->Release();
        mRTView = nullptr;
    }
    if ( mUAView )
    {
        mUAView->Release();
        mUAView = nullptr;
    }
    if ( mDSView )
    {
        mDSView->Release();
        mDSView = nullptr;
    }
    if ( mSRView )
    {
        mSRView->Release();
        mSRView = nullptr;
    }
}
