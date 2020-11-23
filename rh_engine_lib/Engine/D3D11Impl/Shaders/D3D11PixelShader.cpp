#include "D3D11PixelShader.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include "Engine/D3D11Impl/D3D11Convert.h"
#include "Engine/IRenderer.h"
#include <d3d11_3.h>

rh::engine::D3D11PixelShader::D3D11PixelShader( ID3D11Device* device, const String& fileName, const String& entryPoint )
{
    // Compile shader
    String shaderModel = TEXT( "ps_" ) + GetD3DShaderPrefix();//g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);
    ID3DBlob* shaderBlob = nullptr;

    if ( !CALL_D3D_API( CompileShaderFromFile( fileName,
                                               entryPoint,
                                               shaderModel,
                                               reinterpret_cast<void **>( &shaderBlob ) ),
                        TEXT( "Failed to compile pixel shader:" ) + fileName + TEXT( "/" )
                            + entryPoint ) )
        return;

    // Create shader
    CALL_D3D_API( device->CreatePixelShader( shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
                                             nullptr, reinterpret_cast<ID3D11PixelShader**>( &m_pShaderDC ) ),
                  TEXT( "Failed to create pixel shader:" ) + fileName + TEXT( "/" ) + entryPoint );

    shaderBlob->Release();
    shaderBlob = nullptr;
}

void rh::engine::D3D11PixelShader::Set( ID3D11DeviceContext* context )
{
    context->PSSetShader( reinterpret_cast<ID3D11PixelShader*>( m_pShaderDC ), nullptr, 0 );
}

void rh::engine::D3D11PixelShader::ReSet( ID3D11DeviceContext* context )
{
    context->PSSetShader( nullptr, nullptr, 0 );
}
