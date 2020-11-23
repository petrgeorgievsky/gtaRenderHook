#pragma once
#include "D3D11Shader.h"
namespace rh::engine {

/*!
    \class D3D11VertexShader
    \brief D3D11 vertex shader class.

    This class represents vertex shader.
*/
class D3D11VertexShader : public D3D11ShaderOld
{
public:
    /*!
      Initializes shader resource.
  */
    D3D11VertexShader( ID3D11Device *device, const String &fileName, const String &entryPoint );

    /*!
      Releases shader resource.
  */
    ~D3D11VertexShader() override;

    /*!
      Sets shader to a D3D context.
  */
    void Set( ID3D11DeviceContext *context ) override;

    /*!
      Removes shader from a D3D context.
  */
    void ReSet( ID3D11DeviceContext *context ) override;

    /*!
      Releases resources, and reloads shader from file.
  */
    // virtual void Reload(CD3D1XShaderDefineList* localShaderDefineList =
    // nullptr);

    /*!
      Returns shader byte-code blob.
  */
    ID3DBlob *getBlob() { return m_pBlob; }

private:
    // Blob pointer.
    ID3DBlob *m_pBlob = nullptr;
};
} // namespace rh::engine
