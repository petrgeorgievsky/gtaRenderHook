#include "D3D11Shader.h"
#include "D3D11Common.h"
#include "DebugUtils/DebugLogger.h"
#include <Engine/Definitions.h>
#include <array>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <filesystem>

using namespace rh::engine;

HRESULT rh::engine::CompileShaderFromFile( const std::string &fileName,
                               const std::string &entryPoint,
                                           const std::string &shaderModel,
                                           ID3DBlob **        blobOut )
{

    auto      hr          = S_OK;
    DWORD     shaderFlags = 0;
    ID3DBlob *errorBlob   = nullptr;

    if constexpr ( gDebugEnabled )
    {
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the
        // shaders. Setting this flag improves the shader debugging experience,
        // but still allows the shaders to be optimized and to run exactly the
        // way they will run in the release configuration of this program.
        shaderFlags |= D3DCOMPILE_DEBUG;

        // Disable optimizations to further improve shader debugging
        // shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
        // shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
        // shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
    }
    std::array macros   = { D3D_SHADER_MACRO{ "D3D11_API", "1" },
                          D3D_SHADER_MACRO{} };
    auto       filePath = std::filesystem::canonical( fileName );
    auto       stemp    = filePath.generic_wstring();
    auto       ep       = entryPoint;
    auto       sm       = shaderModel;
    hr                  = D3DCompileFromFile( stemp.c_str(), macros.data(),
                             D3D_COMPILE_STANDARD_FILE_INCLUDE, ep.c_str(),
                             sm.c_str(), shaderFlags, 0, blobOut, &errorBlob );

    if ( FAILED( hr ) )
    {
        if ( errorBlob )
        {
            debug::DebugLogger::Error(
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

D3D11Shader::D3D11Shader( const D3D11ShaderDesc &desc )
{
    std::string shader_model;

    // Generate shader model string
    switch ( desc.mDesc.mShaderStage )
    {
    case ShaderStage::Vertex: shader_model = "vs_";
        break;
    case ShaderStage::Pixel: shader_model = "ps_";
        break;
    default: break;// throw std::runtime_error( "not implemented" );
    }
    shader_model += desc.mShaderModel;

    // Try to compile shader
    // TODO: Add ability to preload shaders from compiled binary, 
    // perhaps a packed representation
    std::stringstream error_msg;
    error_msg << "Failed to compile shader from " << desc.mDesc.mShaderPath
              << " " << desc.mDesc.mEntryPoint << " with " << desc.mShaderModel
              << " shader model.";

    if ( !CALL_D3D_API( CompileShaderFromFile( desc.mDesc.mShaderPath,
                                               desc.mDesc.mEntryPoint,
                                               shader_model, &mShaderBlob ),
                        error_msg.str() ) )
        return;

    switch ( desc.mDesc.mShaderStage )
    {
    case ShaderStage::Vertex:

        desc.mDevice->CreateVertexShader(
            mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(),
            nullptr, reinterpret_cast<ID3D11VertexShader **>( &mShaderImpl ) );
        break;
    case ShaderStage::Pixel:

        desc.mDevice->CreatePixelShader(
            mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(),
            nullptr, reinterpret_cast<ID3D11PixelShader **>( &mShaderImpl ) );
        break;
    default:
        break;//throw std::runtime_error( "not implemented" );
    }
}

D3D11Shader::~D3D11Shader()
{
    if ( mShaderImpl )
        mShaderImpl->Release();
    if ( mShaderBlob )
        mShaderBlob->Release();
}