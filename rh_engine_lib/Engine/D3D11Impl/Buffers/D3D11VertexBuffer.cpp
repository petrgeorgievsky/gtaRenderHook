#include "D3D11VertexBuffer.h"
#include <d3d11_3.h>
using namespace rh::engine;

D3D11VertexBuffer::D3D11VertexBuffer( ID3D11Device *device,
                                      unsigned int size,
                                      const D3D11_SUBRESOURCE_DATA *data )
    : D3D11BufferOld::D3D11BufferOld( device,
                                {size, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0},
                                0,
                                0,
                                data )
{}

D3D11VertexBuffer::~D3D11VertexBuffer() = default;
