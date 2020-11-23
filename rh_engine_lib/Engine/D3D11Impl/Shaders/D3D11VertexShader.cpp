#include "D3D11VertexShader.h"
#include "../D3D11Convert.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include "Engine/IRenderer.h"
#include <d3d11_3.h>

rh::engine::D3D11VertexShader::D3D11VertexShader( ID3D11Device* device, const String& fileName, const String& entryPoint )
{
    // Compile shader
    String shaderModel = TEXT( "vs_" ) + GetD3DShaderPrefix();//g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

    if ( !CALL_D3D_API( CompileShaderFromFile( fileName,
                                               entryPoint,
                                               shaderModel,
                                               reinterpret_cast<void **>( &m_pBlob ) ),
                        TEXT( "Failed to compile vertex shader:" ) + fileName + TEXT( "/" )
                            + entryPoint ) )
        return;
    if ( m_pBlob == nullptr ) {
        rh::debug::DebugLogger::Error(
            "Failed to compile vertex shader: CompileShaderFromFile returned empty bytecode!" );
        return;
    }

    // Create shader
    if ( !CALL_D3D_API( device->CreateVertexShader( m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(),
                                                    nullptr, reinterpret_cast<ID3D11VertexShader**>( &m_pShaderDC ) ),
                        TEXT( "Failed to create vertex shader:" ) + fileName + TEXT( "/" ) + entryPoint ) )
    {
        m_pBlob->Release();
        m_pBlob = nullptr;
    }
}

rh::engine::D3D11VertexShader::~D3D11VertexShader()
{
    if ( m_pBlob )
    {
        m_pBlob->Release();
        m_pBlob = nullptr;
    }
}

void rh::engine::D3D11VertexShader::Set( ID3D11DeviceContext* context )
{
    context->VSSetShader( reinterpret_cast<ID3D11VertexShader*>( m_pShaderDC ), nullptr, 0 );
}

void rh::engine::D3D11VertexShader::ReSet( ID3D11DeviceContext* context )
{
    context->VSSetShader( nullptr, nullptr, 0 );
}
