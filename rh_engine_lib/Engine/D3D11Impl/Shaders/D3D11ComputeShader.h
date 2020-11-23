#pragma once
#include "D3D11Shader.h"
namespace rh::engine {

/*!
    @class D3D11ComputeShader
    @brief D3D11 compute shader class.

    This class represents compute shader.
*/
class D3D11ComputeShader : public D3D11ShaderOld
{
public:
    /*!
    Initializes shader resource.
*/
    D3D11ComputeShader( ID3D11Device *device, const String &fileName, const String &entryPoint );

    /*!
    Sets shader to a D3D context.
*/
    void Set( ID3D11DeviceContext *context ) override;

    /*!
    Removes shader from a D3D context.
*/
    void ReSet( ID3D11DeviceContext *context ) override;

    /*!
    Dispatches shader to D3D context.
*/
    void Dispath( ID3D11DeviceContext *context, unsigned int tX, unsigned int tY, unsigned int tZ );

    /*!
    Releases resources, and reloads shader from file.
*/
    // virtual void Reload(CD3D1XShaderDefineList* localShaderDefineList =
    // nullptr);
};
} // namespace rh::engine
