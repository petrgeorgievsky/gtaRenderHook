#include "D3D11ImageView.h"
using namespace rh::engine;

D3D11ImageView::D3D11ImageView(
    const D3D11ImageViewCreateParams &create_params )
{
    // TODO: ADD PARAMS
    switch ( create_params.mViewType )
    {
    case ImageViewType::RenderTargetView:
        create_params.mDevice->CreateRenderTargetView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11RenderTargetView **>( &mView ) );
        break;
    case ImageViewType::DepthStencilView:
        create_params.mDevice->CreateDepthStencilView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11DepthStencilView **>( &mView ) );
        break;
    case ImageViewType::UnorderedAccessView:
        create_params.mDevice->CreateUnorderedAccessView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11UnorderedAccessView **>( &mView ) );
        break;
    case ImageViewType::ShaderView:
        create_params.mDevice->CreateShaderResourceView(
            create_params.mResource, nullptr,
            reinterpret_cast<ID3D11ShaderResourceView **>( &mView ) );
        break;

    default: break;
    }
}

D3D11ImageView::~D3D11ImageView()
{
    if ( mView )
    {
        mView->Release();
        mView = nullptr;
    }
}