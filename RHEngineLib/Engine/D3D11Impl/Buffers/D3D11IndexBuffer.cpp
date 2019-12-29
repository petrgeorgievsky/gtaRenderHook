#include "D3D11IndexBuffer.h"
using namespace rh::engine;

D3D11IndexBuffer::D3D11IndexBuffer( ID3D11Device *device,
                                    unsigned int indexCount,
                                    const D3D11_SUBRESOURCE_DATA *data )
    : D3D11Buffer::D3D11Buffer( device,
                                {static_cast<unsigned int>( indexCount * sizeof( unsigned int ) ),
                                 D3D11_USAGE_IMMUTABLE,
                                 D3D11_BIND_INDEX_BUFFER,
                                 0},
                                0,
                                0,
                                data )
{}

D3D11IndexBuffer::~D3D11IndexBuffer() = default;
