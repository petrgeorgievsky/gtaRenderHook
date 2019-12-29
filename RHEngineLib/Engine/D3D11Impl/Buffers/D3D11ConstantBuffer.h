#pragma once
#include "D3D11Buffer.h"
#include <d3d11_3.h>
namespace rh::engine {
struct ConstantBufferInfo;
class D3D11ConstantBuffer : public D3D11Buffer
{
public:
    D3D11ConstantBuffer( ID3D11Device *device, const ConstantBufferInfo &info );
    ~D3D11ConstantBuffer() override;
};

} // namespace rh::engine
