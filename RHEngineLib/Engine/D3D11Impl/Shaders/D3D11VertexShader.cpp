#include "stdafx.h"
#include "D3D11VertexShader.h"

RHEngine::D3D11VertexShader::D3D11VertexShader(ID3D11Device * device, String fileName, String entryPoint)
{
    // Compile shader
    String shaderModel = TEXT("vs_5_0");//g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

    if (!CALL_D3D_API(CompileShaderFromFile(fileName, entryPoint, shaderModel, &m_pBlob),
        TEXT("Failed to compile vertex shader:") + fileName + TEXT("/") + entryPoint))
        return;

    // Create shader
    if (!CALL_D3D_API(device->CreateVertexShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, (ID3D11VertexShader**)&m_pShaderDC),
        TEXT("Failed to create vertex shader:") + fileName + TEXT("/") + entryPoint))
    {
        m_pBlob->Release();
        m_pBlob = nullptr;
    }
}

RHEngine::D3D11VertexShader::~D3D11VertexShader()
{
    if (m_pBlob) {
        m_pBlob->Release();
        m_pBlob = nullptr;
    }
}

void RHEngine::D3D11VertexShader::Set(ID3D11DeviceContext * context)
{
    context->VSSetShader(static_cast<ID3D11VertexShader*>(m_pShaderDC), nullptr, 0);
}

void RHEngine::D3D11VertexShader::ReSet(ID3D11DeviceContext * context)
{
    context->VSSetShader(nullptr, nullptr, 0);
}
