#include "D3D11Sampler.h"
#include <d3d11.h>
using namespace rh::engine;

D3D11Sampler::D3D11Sampler( const D3D11SamplerCreateParams &create_params )
{
    D3D11_SAMPLER_DESC desc{};
    desc.Filter        = D3D11_FILTER_ANISOTROPIC;
    desc.AddressU      = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV      = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW      = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MaxAnisotropy = 16;
    create_params.mDevice->CreateSamplerState( &desc, &mSampler );
}

D3D11Sampler::~D3D11Sampler()
{
    if ( mSampler )
        mSampler->Release();
}
