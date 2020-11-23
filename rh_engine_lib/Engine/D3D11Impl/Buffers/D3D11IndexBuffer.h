#pragma once
#include "D3D11Buffer.h"

// TODO: Add documentation

namespace rh::engine {

class D3D11IndexBuffer : public D3D11BufferOld
{
public:
    D3D11IndexBuffer( ID3D11Device *device,
                      unsigned int indexCount,
                      const D3D11_SUBRESOURCE_DATA *data );
    ~D3D11IndexBuffer() override;
};

} // namespace rh::engine
