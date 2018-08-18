#include "stdafx.h"
#include "D3D11Shader.h"
#include <d3dcompiler.h>

RHEngine::D3D11Shader::D3D11Shader()
{
}

RHEngine::D3D11Shader::~D3D11Shader()
{
    if (m_pShaderDC) {
        m_pShaderDC->Release();
        m_pShaderDC = nullptr;
    }
}

HRESULT RHEngine::D3D11Shader::CompileShaderFromFile(String fileName, String entryPoint, String shaderModel, ID3DBlob ** blobOut) const
{
    auto hr = S_OK;

    DWORD dwShaderFlags = 0;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows
    // the shaders to be optimized and to run exactly the way they will run in
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    /*std::vector<D3D_SHADER_MACRO> defines;
    auto defineList = g_pGlobalShaderDefines->GetDefineList();
    // concat local defines at the end if they are provided
    if (localShaderDefineList != nullptr) {
        auto localDefineList = localShaderDefineList->GetDefineList();
        defineList.insert(defineList.end(), localDefineList.begin(), localDefineList.end());
    }
    // convet list to required form
    for (const auto &define : defineList) {
        defines.push_back({ define.m_sName.c_str(), define.m_sDefinition.c_str() });
    }
    defines.push_back({});*/
    ID3DBlob* pErrorBlob = nullptr;

    auto stemp = std::wstring(fileName.begin(), fileName.end());
    auto ep = std::string(entryPoint.begin(), entryPoint.end()).c_str();
    auto sm = std::string(entryPoint.begin(), entryPoint.end()).c_str();
    hr = D3DCompileFromFile(stemp.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, ep, sm, dwShaderFlags, 0, blobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            RHDebug::DebugLogger::Error( ToRHString(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer())) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}
