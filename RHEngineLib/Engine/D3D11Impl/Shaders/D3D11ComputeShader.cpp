#include "D3D11ComputeShader.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include "Engine/D3D11Impl/D3D11Convert.h"
#include "Engine/IRenderer.h"
#include <common.h>

rh::engine::D3D11ComputeShader::D3D11ComputeShader( ID3D11Device* device, const String& fileName, const String& entryPoint )
{
    // Compile shader
    String shaderModel = TEXT( "cs_" ) + GetD3DShaderPrefix();
    ID3DBlob* shaderBlob = nullptr;

    if ( !CALL_D3D_API( CompileShaderFromFile( fileName,
                                               entryPoint,
                                               shaderModel,
                                               reinterpret_cast<void **>( &shaderBlob ) ),
                        TEXT( "Failed to compile compute shader:" ) + fileName + TEXT( "/" )
                            + entryPoint ) )
        return;

    // Create shader
    CALL_D3D_API( device->CreateComputeShader( shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
                                             nullptr, reinterpret_cast<ID3D11ComputeShader**>( &m_pShaderDC ) ),
                  TEXT( "Failed to create compute shader:" ) + fileName + TEXT( "/" ) + entryPoint );

    shaderBlob->Release();
    shaderBlob = nullptr;
}

void rh::engine::D3D11ComputeShader::Set( ID3D11DeviceContext * context )
{
    context->CSSetShader( reinterpret_cast<ID3D11ComputeShader*>( m_pShaderDC ), nullptr, 0 );
}

void rh::engine::D3D11ComputeShader::ReSet( ID3D11DeviceContext * context )
{
    context->CSSetShader( nullptr, nullptr, 0 );
}

void rh::engine::D3D11ComputeShader::Dispath( ID3D11DeviceContext * context, unsigned int tX, unsigned int tY, unsigned int tZ )
{
    context->Dispatch( tX, tY, tZ );
}
