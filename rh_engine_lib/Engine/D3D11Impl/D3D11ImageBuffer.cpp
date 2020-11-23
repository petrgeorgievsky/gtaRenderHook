#include "D3D11ImageBuffer.h"
#include "D3D11Convert.h"
#include <d3d11.h>
using namespace rh::engine;

D3D11ImageBuffer::D3D11ImageBuffer(
    const D3D11ImageBufferCreateParams &create_params )
{

    std::vector<D3D11_SUBRESOURCE_DATA> subresources;
    subresources.reserve( create_params.mPreinitData.Size() );
    for ( std::size_t i = 0; i < create_params.mPreinitData.Size(); i++ )
    {
        D3D11_SUBRESOURCE_DATA res{};
        res.pSysMem          = create_params.mPreinitData[i].mData;
        res.SysMemPitch      = create_params.mPreinitData[i].mStride;
        res.SysMemSlicePitch = create_params.mPreinitData[i].mSize;
        subresources.push_back( res );
    }

    switch ( create_params.mDimension )
    {
    case ImageDimensions::d1D:
    {
        D3D11_TEXTURE1D_DESC desc{};
        desc.Width     = create_params.mWidth;
        desc.Format    = GetDXGIResourceFormat( create_params.mFormat );
        desc.ArraySize = create_params.mArrayLayers;
        desc.Usage     = D3D11_USAGE_IMMUTABLE;
        desc.MipLevels = create_params.mMipLevels;
        create_params.mDevice->CreateTexture1D(
            &desc, subresources.size() > 0 ? subresources.data() : nullptr,
            reinterpret_cast<ID3D11Texture1D **>( &mImage ) );
        break;
    }
    case ImageDimensions::d2D:
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width            = create_params.mWidth;
        desc.Height           = create_params.mHeight;
        desc.Format           = GetDXGIResourceFormat( create_params.mFormat );
        desc.ArraySize        = create_params.mArrayLayers;
        desc.Usage            = D3D11_USAGE_IMMUTABLE;
        desc.MipLevels        = create_params.mMipLevels;
        desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        create_params.mDevice->CreateTexture2D(
            &desc, subresources.size() > 0 ? subresources.data() : nullptr,
            reinterpret_cast<ID3D11Texture2D **>( &mImage ) );
        break;
    }
    case ImageDimensions::d3D:
    {
        D3D11_TEXTURE3D_DESC desc{};
        desc.Width     = create_params.mWidth;
        desc.Height    = create_params.mHeight;
        desc.Depth     = create_params.mDepth;
        desc.Format    = GetDXGIResourceFormat( create_params.mFormat );
        desc.Usage     = D3D11_USAGE_IMMUTABLE;
        desc.MipLevels = create_params.mMipLevels;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        create_params.mDevice->CreateTexture3D(
            &desc, subresources.size() > 0 ? subresources.data() : nullptr,
            reinterpret_cast<ID3D11Texture3D **>( &mImage ) );
        break;
    }
    }
}

D3D11ImageBuffer::~D3D11ImageBuffer()
{
    if ( mImage )
    {
        mImage->Release();
        mImage = nullptr;
    }
}
