#include "D3D11InputLayout.h"
#include "D3D11Common.h"
#include <d3d11_3.h>
#include <cassert>

rh::engine::D3D11InputLayout::D3D11InputLayout( ID3D11Device* device,
                                              const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
                                              D3D11VertexShader* shader )
{
    auto blob = shader->getBlob();

    assert( blob != nullptr );

    CALL_D3D_API( device->CreateInputLayout( elements.data(), static_cast<UINT>( elements.size() ),
                                             blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayout ),
                  TEXT( "Failed to create input layout for vertex shader" ) );
}

rh::engine::D3D11InputLayout::~D3D11InputLayout()
{
    if ( m_inputLayout )
    {
        m_inputLayout->Release();
        m_inputLayout = nullptr;
    }
}

void rh::engine::D3D11InputLayout::Set( ID3D11DeviceContext* context )
{
    context->IASetInputLayout( m_inputLayout );
}
