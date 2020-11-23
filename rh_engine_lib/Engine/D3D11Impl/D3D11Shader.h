#pragma once
#include "Engine/Common/IShader.h"

// d3d11 struct forwards:
struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
struct ID3D11Device;
struct ID3D11DeviceChild;
typedef long HRESULT;

namespace rh::engine
{

// Compiles shader with custom entry point and shader model
HRESULT CompileShaderFromFile( const std::string &fileName,
                               const std::string &entryPoint,
                               const std::string &shaderModel,
                               ID3DBlob **        blobOut );
struct D3D11ShaderDesc
{
    // Dependencies...
    ID3D11Device* mDevice;
    // Params
    ShaderDesc mDesc;
    std::string mShaderModel;
};

/*!
    \class D3D11Shader
    \brief D3D11 shader class.

    This class represents shader module implementation for d3d11.
*/
class D3D11Shader : public IShader
{
  public:
    D3D11Shader(const D3D11ShaderDesc & desc);
    ~D3D11Shader() override;
    operator ID3D11DeviceChild *() { return mShaderImpl; }
    operator ID3DBlob *() { return mShaderBlob; }
  private:
    // Shader pointer.
    ID3D11DeviceChild *mShaderImpl = nullptr;
    ID3DBlob *         mShaderBlob = nullptr;
};
} // namespace rh::engine
