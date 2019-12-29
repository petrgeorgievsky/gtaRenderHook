// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XBuffer.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3DSpecificHelpers.h"

CD3D1XBuffer::CD3D1XBuffer( unsigned int size, D3D11_USAGE usage, D3D11_BIND_FLAG bindingFlags, unsigned int cpuAccessFlags,
                            unsigned int miscFlags, unsigned int elementSize, const D3D11_SUBRESOURCE_DATA* initialData )
{
    m_uiSize = size;
    D3D11_BUFFER_DESC bd{};
    bd.Usage = usage;
    bd.ByteWidth = m_uiSize;
    bd.BindFlags = bindingFlags;
    bd.StructureByteStride = elementSize;
    bd.CPUAccessFlags = cpuAccessFlags;
    bd.MiscFlags = miscFlags;

    CALL_D3D_API( GET_D3D_DEVICE->CreateBuffer( &bd, initialData, &m_pBuffer ), "Failed to create d3d11 hardware buffer" );
}


CD3D1XBuffer::~CD3D1XBuffer()
{
    if ( m_pBuffer )
    {
        m_pBuffer->Release();
        m_pBuffer = nullptr;
    }
}

void CD3D1XBuffer::Update( void * data, int size )
{
    auto context = GET_D3D_CONTEXT;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    context->Map( m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    memcpy( mappedResource.pData, data, size < 0 ? m_uiSize : size );
    context->Unmap( m_pBuffer, 0 );
}

void CD3D1XBuffer::SetDebugName( const std::string &name )
{
    g_pDebug->SetD3DName( m_pBuffer, name + "(D3D1XBuffer)" );
}
