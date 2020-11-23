#include "D3D11Buffer.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include <d3d11.h>
using namespace rh::engine;

D3D11Buffer::D3D11Buffer( const D3D11BufferCreateInfo &desc )
    : mImmediateContext( desc.mImmediateContext )
{
    debug::DebugLogger::Log( "D3D11Buffer constructor..." );

    D3D11_BUFFER_DESC impl_desc{};

    impl_desc.Usage          = D3D11_USAGE_DYNAMIC;
    impl_desc.ByteWidth      = desc.mSize;
    impl_desc.BindFlags      = 0;
    impl_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if ( desc.mUsage & BufferUsage::VertexBuffer )
        impl_desc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
    if ( desc.mUsage & BufferUsage::IndexBuffer )
        impl_desc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
    if ( desc.mUsage & BufferUsage::ConstantBuffer )
        impl_desc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
    if ( desc.mUsage & BufferUsage::StagingBuffer )
    {
        impl_desc.Usage = D3D11_USAGE_STAGING;
    }

    impl_desc.StructureByteStride = 0;
    impl_desc.MiscFlags           = 0;
    D3D11_SUBRESOURCE_DATA res_data{};
    res_data.pSysMem          = desc.mInitDataPtr;
    res_data.SysMemPitch      = 0;
    res_data.SysMemSlicePitch = 0;

    CALL_D3D_API( desc.mDevice->CreateBuffer(
                      &impl_desc,
                      desc.mInitDataPtr == nullptr ? nullptr : &res_data,
                      &m_pBuffer ),
                  TEXT( "Failed to create d3d11 hardware buffer" ) );
}

D3D11Buffer::~D3D11Buffer()
{
    debug::DebugLogger::Log( "D3D11Buffer destructor..." );
    std::stringstream ss;
    ss << "buffer_ptr:" << std::hex << reinterpret_cast<uint32_t>( m_pBuffer )
       << ";buffer_refcount=";

    if ( m_pBuffer )
    {
        ss << m_pBuffer->Release();
        m_pBuffer = nullptr;
    }
    debug::DebugLogger::Log( ss.str() );
}

void D3D11Buffer::Update( const void *data, uint32_t size )
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    mImmediateContext->Map( m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
                            &mappedResource );

    memcpy( mappedResource.pData, data, size );

    mImmediateContext->Unmap( m_pBuffer, 0 );
}

void rh::engine::D3D11Buffer::Update( const void *data, uint32_t size,
                                      uint32_t offset )
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    mImmediateContext->Map( m_pBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0,
                            &mappedResource );

    memcpy( ( static_cast<uint8_t *>( mappedResource.pData ) + offset ), data,
            size );

    mImmediateContext->Unmap( m_pBuffer, 0 );
}
void *D3D11Buffer::Lock()
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    mImmediateContext->Map( m_pBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0,
                            &mappedResource );
    return mappedResource.pData;
}
void D3D11Buffer::Unlock() { mImmediateContext->Unmap( m_pBuffer, 0 ); }
