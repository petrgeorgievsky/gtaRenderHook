#pragma once
#include "D3D1XBuffer.h"
#include "RwD3D1XEngine.h"
template <class T>
class CD3D1XStructuredBuffer :
    public CD3D1XBuffer
{
    ID3D11ShaderResourceView* m_pSRV = nullptr;
public:
    CD3D1XStructuredBuffer( int elementCount );
    ~CD3D1XStructuredBuffer();
    ID3D11ShaderResourceView* getSRV() { return m_pSRV; }
};

template<class T>
CD3D1XStructuredBuffer<T>::CD3D1XStructuredBuffer( int elementCount ) :CD3D1XBuffer( elementCount * sizeof( T ), D3D11_USAGE_DYNAMIC,
                                                                                     D3D11_BIND_SHADER_RESOURCE, D3D11_CPU_ACCESS_WRITE,
                                                                                     D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof( T ) )
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory( &srvDesc, sizeof( srvDesc ) );
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = elementCount;
    //srvDesc.Buffer.NumElements = 1024;
    if ( FAILED( GET_D3D_DEVICE->CreateShaderResourceView( m_pBuffer, &srvDesc, &m_pSRV ) ) )
    {
        g_pDebug->printError( "Failed to create structured buffer resource view." );
    }
}

template<class T>
CD3D1XStructuredBuffer<T>::~CD3D1XStructuredBuffer()
{
    if ( m_pSRV )
    {
        m_pSRV->Release();
        m_pSRV = nullptr;
    }
}
