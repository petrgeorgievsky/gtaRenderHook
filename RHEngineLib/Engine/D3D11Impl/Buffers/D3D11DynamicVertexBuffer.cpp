#include "D3D11DynamicVertexBuffer.h"
using namespace rh::engine;

D3D11DynamicVertexBuffer::D3D11DynamicVertexBuffer( ID3D11Device *device,
                                                    unsigned int vertexSize,
                                                    unsigned int maxVertexCount )
    : D3D11Buffer( device,
                   {vertexSize * maxVertexCount,
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_VERTEX_BUFFER,
                    D3D11_CPU_ACCESS_WRITE} )
{}

D3D11DynamicVertexBuffer::~D3D11DynamicVertexBuffer() = default;
