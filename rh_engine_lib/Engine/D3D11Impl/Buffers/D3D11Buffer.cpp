#include "D3D11Buffer.h"
#include "..\D3D11Buffer.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include <d3d11_3.h>
using namespace rh::engine;
D3D11BufferOld::D3D11BufferOld( ID3D11Device *                device,
                                const D3D11BufferInfo &       info,
                                unsigned int                  miscFlags,
                                unsigned int                  elementSize,
                                const D3D11_SUBRESOURCE_DATA *initialData )
{
    debug::DebugLogger::Log( "D3D11Buffer constructor..." );

    D3D11_BUFFER_DESC desc{};

    m_uiSize = info.size;

    desc.Usage               = info.usage;
    desc.ByteWidth           = m_uiSize;
    desc.BindFlags           = info.bindingFlags;
    desc.StructureByteStride = elementSize;
    desc.CPUAccessFlags      = info.cpuAccessFlags;
    desc.MiscFlags           = miscFlags;

    CALL_D3D_API( device->CreateBuffer( &desc, initialData, &m_pBuffer ),
                  TEXT( "Failed to create d3d11 hardware buffer" ) );
}

D3D11BufferOld::~D3D11BufferOld()
{
    debug::DebugLogger::Log( "D3D11Buffer destructor..." );
    std::stringstream ss;
    ss << std::hex << static_cast<const void *>( m_pBuffer ) << ";";

    if ( m_pBuffer )
    {
        ss << m_pBuffer->Release();
        m_pBuffer = nullptr;
    }
    debug::DebugLogger::Log( ss.str() );
}

void D3D11BufferOld::Update( ID3D11DeviceContext *context, const void *data,
                             int size )
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    context->Map( m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );

    memcpy( mappedResource.pData, data,
            size < 0 ? m_uiSize : static_cast<size_t>( size ) );

    context->Unmap( m_pBuffer, 0 );
}

void D3D11BufferOld::SetDebugName( const String & /*name*/ )
{
    // g_pDebug->SetD3DName(m_pBuffer, name + "(D3D1XBuffer)");
}
