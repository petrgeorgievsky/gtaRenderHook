#include "D3D11ConstantBuffer.h"
#include "Engine/Common/types/constant_buffer_info.h"
#include <d3d11_3.h>
using namespace rh::engine;

D3D11ConstantBuffer::D3D11ConstantBuffer( ID3D11Device *device, const ConstantBufferInfo &info )
    : D3D11BufferOld( device,
                   {static_cast<uint32_t>( info.size ),
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_CONSTANT_BUFFER,
                    D3D11_CPU_ACCESS_WRITE} )
{}

D3D11ConstantBuffer::~D3D11ConstantBuffer() = default;
