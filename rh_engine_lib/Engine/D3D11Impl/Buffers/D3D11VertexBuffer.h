#pragma once
#include "D3D11Buffer.h"

namespace rh::engine {

class D3D11VertexBuffer : public D3D11BufferOld
{
public:
    D3D11VertexBuffer( ID3D11Device *device, unsigned int size, const D3D11_SUBRESOURCE_DATA *data );
    ~D3D11VertexBuffer() override;
};

} // namespace rh::engine
