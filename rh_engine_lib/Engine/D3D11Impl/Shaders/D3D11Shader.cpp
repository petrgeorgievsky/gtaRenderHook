#include "D3D11Shader.h"
#include "Engine/D3D11Impl/D3D11Common.h"
#include <d3d11_3.h>
#include <d3dcompiler.h>
#include <filesystem>
#include "..\D3D11Shader.h"

rh::engine::D3D11ShaderOld::D3D11ShaderOld() = default;

rh::engine::D3D11ShaderOld::~D3D11ShaderOld()
{
    if ( m_pShaderDC )
    {
        static_cast<ID3D11DeviceChild *>( m_pShaderDC )->Release();
        m_pShaderDC = nullptr;
    }
}

HRESULT rh::engine::D3D11ShaderOld::CompileShaderFromFile(
    const String &fileName, const String &entryPoint, const String &shaderModel,
    void **blobOut ) const
{
    auto      hr          = S_OK;
    DWORD     shaderFlags = 0;
    ID3DBlob *errorBlob   = nullptr;

#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still
    // allows the shaders to be optimized and to run exactly the way they will
    // run in the release configuration of this program.
    shaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    // shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    // shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    // shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

    /*std::vector<D3D_SHADER_MACRO> defines;
  auto defineList = g_pGlobalShaderDefines->GetDefineList();
  // concat local defines at the end if they are provided
  if (localShaderDefineList != nullptr) {
      auto localDefineList = localShaderDefineList->GetDefineList();
      defineList.insert(defineList.end(), localDefineList.begin(),
  localDefineList.end());
  }
  // convet list to required form
  for (const auto &define : defineList) {
      defines.push_back({ define.m_sName.c_str(), define.m_sDefinition.c_str()
  });
  }
  defines.push_back({});*/

    auto filePath = std::filesystem::canonical( fileName );
    auto stemp    = filePath.generic_wstring();
    auto ep       = FromRHString( entryPoint );
    auto sm       = FromRHString( shaderModel );
    hr            = D3DCompileFromFile(
        stemp.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, ep.c_str(),
        sm.c_str(), shaderFlags, 0, reinterpret_cast<ID3DBlob **>( blobOut ),
        &errorBlob );

    if ( FAILED( hr ) )
    {
        if ( errorBlob )
        {
            rh::debug::DebugLogger::Error(
                ToRHString( reinterpret_cast<const char *>(
                    errorBlob->GetBufferPointer() ) ) );
            errorBlob->Release();
        }
        return hr;
    }

    if ( errorBlob )
        errorBlob->Release();

    return S_OK;
}