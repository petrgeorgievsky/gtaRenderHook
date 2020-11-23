#include "D3D11Swapchain.h"
#include "D3D11Common.h"
#include "D3D11ImageView.h"
#include <algorithm>
#include <cassert>
#include <d3d11.h>

using namespace rh::engine;

D3D11Swapchain::D3D11Swapchain(
    const D3D11SwapchainCreateParams &create_params )
{
    assert( create_params.mDXGIFactory != nullptr );
    assert( create_params.mDevice != nullptr );

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount =
        std::max<uint32_t>( create_params.mPresentParams.mBufferCount, 1 );
    sd.BufferDesc.Width  = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // sd.BufferDesc.RefreshRate = create_params.mDisplayMode.RefreshRate;
    sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow       = create_params.mWindowHandle;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed           = create_params.mPresentParams.mWindowed;
    sd.Flags              = create_params.mPresentParams.mWindowed
                   ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
                   : 0;

    debug::DebugLogger::Log(
        "Swap-Chain params:"
        "\n\tBufferDesc.Width:" +
        std::to_string( sd.BufferDesc.Width ) +
        "\n\tBufferDesc.Height:" + std::to_string( sd.BufferDesc.Height ) +
        "\n\tBufferDesc.Format:" + std::to_string( sd.BufferDesc.Format ) +
        "\n\tBufferDesc.RefreshRate.Numerator:" +
        std::to_string( sd.BufferDesc.RefreshRate.Numerator ) +
        "\n\tBufferDesc.RefreshRate.Denominator:" +
        std::to_string( sd.BufferDesc.RefreshRate.Denominator ) +
        "\n\tBufferDesc.OutputWindow:" +
        std::to_string( reinterpret_cast<uint64_t>( sd.OutputWindow ) ) +
        "\n\tBufferDesc.Windowed:" +
        std::to_string( create_params.mPresentParams.mWindowed ) );

    if ( !CALL_D3D_API( create_params.mDXGIFactory->CreateSwapChain(
                            create_params.mDevice, &sd, &mSwapchain ),
                        "Failed to create swapchain using DX11 API" ) )
        return;

    CALL_D3D_API( mSwapchain->GetDesc( &sd ),
                  "Failed to retrieve swapchain description" );
    mWidth  = sd.BufferDesc.Width;
    mHeight = sd.BufferDesc.Height;

    CALL_D3D_API(
        mSwapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ),
                               reinterpret_cast<void **>( &mBackbufer ) ),
        "Failed to get backbuffer texture." );

    D3D11ImageViewCreateParams iv_create_params{};
    iv_create_params.mDevice   = create_params.mDevice;
    iv_create_params.mResource = mBackbufer;
    iv_create_params.mUsage    = ImageViewUsage::RenderTarget;
    mBackbufferView            = new D3D11ImageView( iv_create_params );
}

D3D11Swapchain::~D3D11Swapchain()
{
    delete mBackbufferView;
    mBackbufferView = nullptr;
    if ( mBackbufer )
    {
        mBackbufer->Release();
        mBackbufer = nullptr;
    }
    if ( mSwapchain )
    {
        mSwapchain->Release();
        mSwapchain = nullptr;
    }
}

SwapchainFrame D3D11Swapchain::GetAvaliableFrame( ISyncPrimitive * )
{
    SwapchainFrame result{};
    result.mImageView = mBackbufferView;
    result.mWidth     = mWidth;
    result.mHeight    = mHeight;

    return result;
}

bool D3D11Swapchain::Present( uint32_t, ISyncPrimitive * )
{
    HRESULT result = mSwapchain->Present( 0, 0 );
    return SUCCEEDED( result );
}
