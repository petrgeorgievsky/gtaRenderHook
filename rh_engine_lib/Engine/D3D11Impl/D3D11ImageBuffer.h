#pragma once
#include "Engine/Common/IImageBuffer.h"

// d3d11 struct forwards:
struct ID3D11Device;
struct ID3D11Resource;

namespace rh::engine
{
struct D3D11ImageBufferCreateParams : ImageBufferCreateParams
{
    // Dependencies..
    ID3D11Device *mDevice;
};

class D3D11ImageBuffer : public IImageBuffer
{
  public:
    D3D11ImageBuffer( const D3D11ImageBufferCreateParams
                     &create_params );
    ~D3D11ImageBuffer() override;
    ID3D11Resource * GetImpl(){
        return mImage;
    }
  private:
    ID3D11Resource *mImage = nullptr;
};
} // namespace rh::engine
