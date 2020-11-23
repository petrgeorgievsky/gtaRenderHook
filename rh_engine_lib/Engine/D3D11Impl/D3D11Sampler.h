#pragma once
#include "Engine/Common/ISampler.h"

struct ID3D11Device;
struct ID3D11SamplerState;

namespace rh::engine
{
struct D3D11SamplerCreateParams : SamplerDesc
{
    // Dependencies..
    ID3D11Device *  mDevice;
};

class D3D11Sampler : public ISampler
{

  public:
    D3D11Sampler( const D3D11SamplerCreateParams &create_params );
    ~D3D11Sampler() override;
    ID3D11SamplerState * GetImpl() { return mSampler; }

  private:
    ID3D11SamplerState *mSampler = nullptr;
};
} // namespace rh::engine
