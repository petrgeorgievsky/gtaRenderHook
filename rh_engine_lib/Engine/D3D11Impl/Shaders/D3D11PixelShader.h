#pragma once
#include "D3D11Shader.h"
namespace rh::engine {

/*!
    @class D3D11PixelShader
    @brief D3D11 pixel shader class.

    This class represents pixel shader.
*/
class D3D11PixelShader : public D3D11ShaderOld
{
public:
    /*!
    Initializes shader resource.
*/
    D3D11PixelShader( ID3D11Device *device, const String &fileName, const String &entryPoint );

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
};
} // namespace rh::engine
