#pragma once
#include "D3D11Buffer.h"
namespace rh::engine {
class D3D11DynamicVertexBuffer : public D3D11BufferOld
{
public:
    D3D11DynamicVertexBuffer( ID3D11Device *device,
                              unsigned int vertexSize,
                              unsigned int maxVertexCount );
    ~D3D11DynamicVertexBuffer() override;
};
} // namespace rh::engine
