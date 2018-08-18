#include "stdafx.h"
#include "D3D11PixelShader.h"

RHEngine::D3D11PixelShader::D3D11PixelShader(ID3D11Device * device, String fileName, String entryPoint)
{
    // Compile shader
    String shaderModel = TEXT("ps_5_0");//g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);
    ID3DBlob* shaderBlob = nullptr;
    if (!CALL_D3D_API(CompileShaderFromFile(fileName, entryPoint, shaderModel, &shaderBlob),
        TEXT("Failed to compile vertex shader:") + fileName + TEXT("/") + entryPoint))
        return;

    // Create shader
    CALL_D3D_API(device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, (ID3D11PixelShader**)&m_pShaderDC),
        TEXT("Failed to create vertex shader:") + fileName + TEXT("/") + entryPoint);
    shaderBlob->Release();
    shaderBlob = nullptr;
}

void RHEngine::D3D11PixelShader::Set(ID3D11DeviceContext * context)
{
    context->PSSetShader(static_cast<ID3D11PixelShader*>(m_pShaderDC), nullptr, 0);
}

void RHEngine::D3D11PixelShader::ReSet(ID3D11DeviceContext * context)
{
    context->PSSetShader(nullptr, nullptr, 0);
}
