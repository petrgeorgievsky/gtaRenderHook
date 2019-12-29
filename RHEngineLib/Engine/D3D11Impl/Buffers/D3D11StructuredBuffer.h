#pragma once
#include "../ImageBuffers/ImageBuffer.h"
#include "D3D11Buffer.h"
#include <d3d11_3.h>

namespace rh::engine {
struct StructuredBufferInfo;
class D3D11StructuredBuffer : public D3D11Buffer
{
    ID3D11ShaderResourceView *m_pSRV = nullptr;

public:
    D3D11StructuredBuffer( ID3D11Device *device, const StructuredBufferInfo &buffer_info );
    ~D3D11StructuredBuffer() override;
    ID3D11ShaderResourceView *GetShaderResourceView() override { return m_pSRV; }
};
} // namespace rh::engine
