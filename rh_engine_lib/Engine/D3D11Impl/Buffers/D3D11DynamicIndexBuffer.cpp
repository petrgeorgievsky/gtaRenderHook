#include "D3D11DynamicIndexBuffer.h"
#include <d3d11_3.h>
using namespace rh::engine;

D3D11DynamicIndexBuffer::D3D11DynamicIndexBuffer( ID3D11Device *device, unsigned int maxIndexCount )
    : D3D11BufferOld( device,
                   {static_cast<unsigned int>( maxIndexCount * sizeof( unsigned short ) ),
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_INDEX_BUFFER,
                    D3D11_CPU_ACCESS_WRITE} )
{}

D3D11DynamicIndexBuffer::~D3D11DynamicIndexBuffer() = default;
