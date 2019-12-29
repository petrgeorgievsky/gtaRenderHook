#ifndef D3D1XStateManager_h__
#define D3D1XStateManager_h__
#include <map>
#include "D3D1XConstantBuffer.h"
#include "D3D1XStructuredBuffer.h"
#include "D3D1XEnumParser.h"

class CD3DRenderer;
// TODO: because some renderstates are game-specific, they should be moved out of this structure
struct ShaderRenderStateBuffer
{
    UINT	bHasTexture;
    float	fScreenWidth;
    float	fScreenHeight;
    UINT	uiAlphaTestType;

    float	fAlphaTestRef;
    UINT	bFogEnable;
    UINT	uiFogType;
    RwRGBA	cFogColor;
    RwV4d	vCamPos;
    RwV4d	vSunDir;
    RwV4d   vSkyLightCol;
    RwV4d   vHorizonCol;
    RwV4d   vSunColor;
    RwV4d   vWaterColor;
    RwV4d   vGradingColor0;
    RwV4d   vGradingColor1;
    UINT	uiLightCount;
    float	fFogStart;
    float	fFogRange;
    float	fFarClip;
    float fTimeStep;
    float pad[3];
};

extern ShaderRenderStateBuffer g_shaderRenderStateBuffer;
/*! \class CD3D1XStateManager
    \brief Render state manager class.

    This class manages render states for current frame. Responsible for caching of render states and such stuff.
*/
class CD3D1XStateManager
{
public:
    /*!
        Initializes every state block and resources for state manager.
    */
    CD3D1XStateManager();
    /*!
        Releases all resources allocated by state manager
    */
    ~CD3D1XStateManager();

    /*!
        Changes current culling mode.
    */
    void SetCullMode( RwCullMode mode );
    /*!
        Enables or disables depth test.
    */
    void SetDepthEnable( BOOL bEnable );
    /*!
        Enables or disables writing to Z-Buffer.
    */
    void SetZWriteEnable( bool bEnable );
    /*!
        Changes current fill mode.
    */
    void SetFillMode( D3D11_FILL_MODE mode );
    /*!
        Enables or disables stencil buffer.
    */
    void SetStencilEnable( BOOL bEnable );
    /*!
        Changes current stencil fail operation.
    */
    void SetStencilFail( RwStencilOperation op );
    /*!
        Changes current stencil z-fail operation.
    */
    void SetStencilZFail( RwStencilOperation op );
    /*!
        Changes current stencil pass operation.
    */
    void SetStencilPass( RwStencilOperation op );
    /*!
        Changes current stencil function.
    */
    void SetStencilFunc( RwStencilFunction fn );
    /*!
        Changes current stencil function reference value.
    */
    void SetStencilFuncRef( UINT ref );
    /*!
        Changes current stencil function mask value.
    */
    void SetStencilFuncMask( int mask );
    /*!
        Changes current stencil function write mask value.
    */
    void SetStencilFuncWriteMask( int mask );
    /*!
        Enables or disables alpha blending.
    */
    void SetAlphaBlendEnable( BOOL bEnable );
    /*!
        Enables or disables alpha testing.
    */
    void SetAlphaTestEnable( bool bEnable );
    /*!
        Changes current alpha blending destination function.
    */
    void SetDestAlphaBlend( RwBlendFunction func );
    /*!
        Changes current alpha blending source function.
    */
    void SetSrcAlphaBlend( RwBlendFunction func );
    /*!
        Changes current texture UV adress mode.
    */
    void SetTextureAdressUV( RwTextureAddressMode mode );
    /*!
        Changes current texture U adress mode.
    */
    void SetTextureAdressU( RwTextureAddressMode mode );
    /*!
        Changes current texture V adress mode.
    */
    void SetTextureAdressV( RwTextureAddressMode mode );
    /*!
        Changes current texture filter mode.
    */
    void SetTextureFilterMode( RwTextureFilterMode  mode );
    /*!
        Changes current texture max anisotropy.
    */
    void SetTextureAnisotropy( RwInt8  maxAnisotropy );
    /*!
        Changes current texture border color.
    */
    void SetTextureBorderColor( RwRGBA color );
    /*!
        Enables or disables texture rendering in shaders.
    */
    void SetTextureEnable( UINT enable );
    /*!
        Changes current alpha test function.
    */
    void SetAlphaTestFunc( RwAlphaTestFunction func );
    /*!
        Changes current alpha test reference.
    */
    void SetAlphaTestRef( float ref );
    /*!
        Changes current primitive topology.
    */
    void SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY topology );
    /*!
        Changes current alpha test function.
    */
    void SetInputLayout( ID3D11InputLayout* layout );
    /*!
        Changes current vertex buffer.
    */
    void SetVertexBuffer( ID3D11Buffer* buffer, UINT stride, UINT offset );
    /*!
        Changes current index buffer.
    */
    void SetIndexBuffer( ID3D11Buffer* buffer );
    /*!
        Changes current screen size.
    */
    void SetScreenSize( float w, float h );
    /*!
        Changes current texture in slot 0.
    */
    void SetRaster( RwRaster* r ) { if ( m_pCurrentRaster != r ) { m_pCurrentRaster = r; m_bRasterReqUpdate = true; } }
    /*!
        Changes current texture in slot \param Stage.
    */
    void SetRaster( RwRaster * raster, int Stage );
    /*!
        Changes current texture in slot \param Stage in compute shader.
    */
    void SetRasterCS( RwRaster * raster, int Stage );
    /*!
        Changes current constant buffer in vertex shader.
    */
    template <class T>
    void SetConstantBufferVS( CD3D1XConstantBuffer<T> * buffer, int Stage );
    /*!
        Changes current constant buffer in pixel shader.
    */
    template <class T>
    void SetConstantBufferPS( CD3D1XConstantBuffer<T> * buffer, int Stage );
    /*!
        Changes current constant buffer in domain shader.
    */
    template <class T>
    void SetConstantBufferDS( CD3D1XConstantBuffer<T> * buffer, int Stage );
    /*!
        Changes current constant buffer in compute shader.
    */
    template <class T>
    void SetConstantBufferCS( CD3D1XConstantBuffer<T> * buffer, int Stage );
    /*!
        Changes current structured buffer in pixel shader.
    */
    template <class T>
    void SetStructuredBufferPS( CD3D1XStructuredBuffer<T> * buffer, int Stage );
    /*!
        Changes current constant buffer in compute shader.
    */
    template <class T>
    void SetStructuredBufferCS( CD3D1XStructuredBuffer<T> * buffer, int Stage );
    /*!
        Changes current sun direction and day-night balance.
    */
    void SetSunDir( RwV3d* vec, float dnBalance );
    /*!
        Changes current light count.
    */
    void SetLightCount( int count );
    /*!
        Changes current fog start.
    */
    void SetFogStart( float start );
    /*!
        Changes current fog range.
    */
    void SetFogRange( float range );
    /*!
        Changes current render targets and depthstencil render target.
    */
    void SetRenderTargets( unsigned int count, ID3D11RenderTargetView* const * ppRenderTargets, ID3D11DepthStencilView* pDepthTarget );
    /*!
        Changes current viewport.
    */
    void SetViewport( const D3D11_VIEWPORT &vp );
    /*!
        Flushes all state changes.
    */
    void FlushStates();
    /*!
        Flushes all render target changes.
    */
    void FlushRenderTargets();

    /*!
        Returns current alpha test reference value.
    */
    UINT GetAlphaTestRef() { return (UINT)( g_shaderRenderStateBuffer.fAlphaTestRef * 255 ); }
    /*!
        Returns state of depth test.
    */
    BOOL GetDepthEnable() { return m_depthStencilDesc.DepthEnable; }
    /*!
        Returns state of Z-Buffer write.
    */
    bool GetZWriteEnable() { return m_depthStencilDesc.DepthWriteMask == D3D11_DEPTH_WRITE_MASK_ALL; }
    /*!
        Returns true if depth state has been changed.
    */
    bool IsDepthDescRequiresUpdate();
    /*!
        Returns true if raster state has been changed.
    */
    bool IsRasterDescRequiresUpdate();
    /*!
        Returns true if blend state has been changed.
    */
    bool IsBlendDescRequiresUpdate();
    /*!
        Returns true if sampler description state has been changed.
    */
    bool IsSamplerDescRequiresUpdate();
    /*!
        Returns true if viewport has been changed.
    */
    bool IsViewportRequiresUpdate();
    /*!
        Returns current culling mode.
    */
    RwCullMode GetCullMode()
    {
        switch ( m_rasterDesc.CullMode )
        {
        case D3D11_CULL_BACK:	return rwCULLMODECULLBACK;
        case D3D11_CULL_FRONT:	return rwCULLMODECULLFRONT;
        default:				return rwCULLMODECULLNONE;
        }
    }
    /*!
        Returns true if alpha blending is enabled.
    */
    BOOL GetAlphaBlendEnable() { return m_blendDesc.RenderTarget[0].BlendEnable; };
    /*!
        Returns current alpha blending destination function.
    */
    RwBlendFunction GetDestAlphaBlend() { return CD3D1XEnumParser::ConvertBlendFunc( m_blendDesc.RenderTarget[0].DestBlend ); };
    /*!
        Returns current alpha blending source function.
    */
    RwBlendFunction GetSrcAlphaBlend() { return CD3D1XEnumParser::ConvertBlendFunc( m_blendDesc.RenderTarget[0].SrcBlend ); };
    /*!
        Returns current UV texture address mode.
    */
    RwTextureAddressMode GetTextureAdressUV() { return CD3D1XEnumParser::ConvertTextureAddressMode( m_sampDesc.AddressU ); };
    /*!
        Returns current U texture address mode.
    */
    RwTextureAddressMode GetTextureAdressU() { return CD3D1XEnumParser::ConvertTextureAddressMode( m_sampDesc.AddressU ); };
    /*!
        Returns current V texture address mode.
    */
    RwTextureAddressMode GetTextureAdressV() { return CD3D1XEnumParser::ConvertTextureAddressMode( m_sampDesc.AddressV ); };
    /*!
        Returns current texture filtering mode.
    */
    RwTextureFilterMode  GetTextureFilterMode() { return CD3D1XEnumParser::ConvertTextureFilterMode( m_sampDesc.Filter ); };
    /*!
        Returns current raster in slot #0.
    */
    RwRaster* GetRaster() { return m_pCurrentRaster; }
    /*!
        Returns current shader model postfix.
    */
    std::string GetShaderModel( D3D_FEATURE_LEVEL featureLevel )
    {
        std::map<D3D_FEATURE_LEVEL, std::string> shaderVersionMap;
        shaderVersionMap[D3D_FEATURE_LEVEL_9_1] = "4_0_level_9_1";
        shaderVersionMap[D3D_FEATURE_LEVEL_9_2] = "4_0_level_9_2";
        shaderVersionMap[D3D_FEATURE_LEVEL_9_3] = "4_0_level_9_3";
        shaderVersionMap[D3D_FEATURE_LEVEL_10_0] = "4_0";
        shaderVersionMap[D3D_FEATURE_LEVEL_10_1] = "4_0";
        shaderVersionMap[D3D_FEATURE_LEVEL_11_0] = "5_0";
        shaderVersionMap[D3D_FEATURE_LEVEL_11_1] = "5_0";
        shaderVersionMap[D3D_FEATURE_LEVEL_12_0] = "5_0";
        shaderVersionMap[D3D_FEATURE_LEVEL_12_1] = "5_0";
        return shaderVersionMap[featureLevel];
    }
private:
    ID3D11BlendState*			m_pBlendState = nullptr;
    ID3D11BlendState*	m_pBlendState_Default;
    ID3D11BlendState*   m_pBlendState_AlphaBlend;
    ID3D11BlendState*   m_pBlendState_BlendDestOne;
    ID3D11BlendState*   m_pBlendState_NoBlendDestOne;
    ID3D11BlendState*   m_pBlendState_BlendAdditive;
    ID3D11RasterizerState*		m_pRasterState = nullptr;
    ID3D11DepthStencilState*	m_pDepthStencilState = nullptr;
    ID3D11SamplerState*			m_pSamplerState = nullptr;
    ID3D11SamplerState*			m_pCompSamplerState = nullptr;
    ID3D11Buffer*				m_pGlobalValuesBuffer = nullptr;
    RwRaster*					m_pCurrentRaster = nullptr;
    RwRaster*					m_pOldRaster = nullptr;
    ID3D11InputLayout*			m_pCurrentInputLayout = nullptr;
    ID3D11Buffer*				m_pCurrentVertexBuffer = nullptr;
    ID3D11Buffer*				m_pCurrentIndexBuffer = nullptr;
    ID3D11RenderTargetView*     m_pCurrentRenderTargets[8];
    ID3D11DepthStencilView*     m_pCurrentDepthStencilView = nullptr;

    D3D11_RASTERIZER_DESC		m_rasterDesc{};
    D3D11_RASTERIZER_DESC		m_rasterDescOld{};
    D3D11_DEPTH_STENCIL_DESC	m_depthStencilDesc{};
    D3D11_DEPTH_STENCIL_DESC	m_depthStencilDescOld{};
    D3D11_BLEND_DESC			m_blendDesc{};
    D3D11_BLEND_DESC			m_blendDescOld{};
    D3D11_SAMPLER_DESC			m_sampDesc{};
    D3D11_SAMPLER_DESC			m_sampDescOld{};
    D3D11_SAMPLER_DESC			m_compSampDesc{};
    D3D10_PRIMITIVE_TOPOLOGY	m_currentPrimitiveTopology;
    D3D11_VIEWPORT				m_viewport{};
    D3D11_VIEWPORT				m_viewportOld{};

    UINT						m_currentStencilRef = 0;
    UINT						m_currentAlphaTestType = 1;
    UINT						m_currentVBufferStride = -1;
    UINT						m_currentVBufferOffset = -1;
    UINT						m_currentRenderTargetNeedsUpdate = 0;

    bool						m_bRasterDescReqUpdate;
    bool						m_bDepthStencilDescReqUpdate;
    bool						m_bBlendDescReqUpdate;
    bool						m_bSampDescReqUpdate;
    bool						m_bTopologyReqUpdate;
    bool						m_bGlobalValuesReqUpdate;
    bool						m_bInputLayoutReqUpdate;
    bool						m_bRasterReqUpdate;
    bool						m_bIndexBufferReqUpdate;
    bool						m_bVertexBufferReqUpdate;
    bool						m_bRenderTargetReqUpdate;
};
extern CD3D1XStateManager* g_pStateMgr;
#endif // D3D1XStateManager_h__

template<class T>
inline void CD3D1XStateManager::SetConstantBufferVS( CD3D1XConstantBuffer<T>* buffer, int Stage )
{
    auto buf = buffer->getBuffer();
    GET_D3D_CONTEXT->VSSetConstantBuffers( Stage, 1, &buf );
}

template<class T>
inline void CD3D1XStateManager::SetConstantBufferDS( CD3D1XConstantBuffer<T>* buffer, int Stage )
{
    auto buf = buffer->getBuffer();
    GET_D3D_CONTEXT->DSSetConstantBuffers( Stage, 1, &buf );
}

template<class T>
inline void CD3D1XStateManager::SetConstantBufferPS( CD3D1XConstantBuffer<T>* buffer, int Stage )
{
    auto buf = buffer->getBuffer();
    GET_D3D_CONTEXT->PSSetConstantBuffers( Stage, 1, &buf );
}

template<class T>
inline void CD3D1XStateManager::SetConstantBufferCS( CD3D1XConstantBuffer<T>* buffer, int Stage )
{
    auto buf = buffer->getBuffer();
    GET_D3D_CONTEXT->CSSetConstantBuffers( Stage, 1, &buf );
}

template<class T>
inline void CD3D1XStateManager::SetStructuredBufferPS( CD3D1XStructuredBuffer<T>* buffer, int Stage )
{
    auto buf = buffer->getSRV();
    GET_D3D_CONTEXT->PSSetShaderResources( Stage, 1, &buf );
}

template<class T>
inline void CD3D1XStateManager::SetStructuredBufferCS( CD3D1XStructuredBuffer<T>* buffer, int Stage )
{
    auto buf = buffer->getSRV();
    GET_D3D_CONTEXT->CSSetShaderResources( Stage, 1, &buf );
}
