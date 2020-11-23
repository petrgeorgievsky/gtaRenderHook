#include "D3D11StructuredBuffer.h"
#include "Engine/Common/types/structured_buffer_info.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include <d3d11_3.h>
using namespace rh::engine;

D3D11StructuredBuffer::D3D11StructuredBuffer( ID3D11Device *device,
                                              const StructuredBufferInfo &buffer_info )
    : D3D11BufferOld( device,
                   {static_cast<unsigned int>( buffer_info.elementCount * buffer_info.elementSize ),
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_SHADER_RESOURCE,
                    D3D11_CPU_ACCESS_WRITE},
                   D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
                   buffer_info.elementSize )
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = buffer_info.elementCount;

    CALL_D3D_API( device->CreateShaderResourceView( m_pBuffer, &srvDesc, &m_pSRV ),
                  TEXT( "Failed to create shader resource view over a structured buffer" ) );
}

D3D11StructuredBuffer::~D3D11StructuredBuffer()
{
    if ( m_pSRV ) {
        m_pSRV->Release();
        m_pSRV = nullptr;
    }
}
