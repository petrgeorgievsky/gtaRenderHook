#ifndef D3D1XStateManager_h__
#define D3D1XStateManager_h__
#include "D3DRenderer.h"

struct ShaderRenderStateBuffer {
	UINT	bHasTexture;
	float	fScreenWidth;
	float	fScreenHeight;
	UINT	uiAlphaTestType;

	float	fAlphaTestRef;
	UINT	bFogEnable;
	UINT	uiFogType;
	RwRGBA	cFogColor;
	RwV4d	vCamPos;
};

extern ShaderRenderStateBuffer globalSRSBuffer;
class CD3D1XStateManager
{
public:
	CD3D1XStateManager(CD3DRenderer* pRenderer);
	~CD3D1XStateManager();

	void SetCullMode				(D3D11_CULL_MODE mode);
	void SetDepthEnable				(BOOL bEnable);
	void SetZWriteEnable			(bool bEnable);
	void SetFillMode				(D3D11_FILL_MODE mode);

	void SetStencilEnable			(BOOL bEnable);
	void SetStencilFail				(RwStencilOperation op);
	void SetStencilZFail			(RwStencilOperation op);
	void SetStencilPass				(RwStencilOperation op);
	void SetStencilFunc				(RwStencilFunction fn);
	void SetStencilFuncRef			(UINT ref);
	void SetStencilFuncMask			(int mask);
	void SetStencilFuncWriteMask	(int mask);	

	void SetAlphaBlendEnable		(BOOL bEnable);
	void SetAlphaTestEnable			(bool bEnable);
	void SetDestAlphaBlend			(RwBlendFunction func);
	void SetSrcAlphaBlend			(RwBlendFunction func);

	void SetTextureAdressUV			(RwTextureAddressMode mode);
	void SetTextureAdressU			(RwTextureAddressMode mode);
	void SetTextureAdressV			(RwTextureAddressMode mode);
	void SetTextureFilterMode		(RwTextureFilterMode  mode);
	void SetTextureBorderColor		(RwRGBA color);

	void SetTextureEnable			(UINT enable);
	void SetAlphaTestFunc			(RwAlphaTestFunction func);
	void SetAlphaTestRef			(float ref);
	void SetScreenSize				(float w, float h);
	void SetRaster					(RwRaster*r) { m_pCurrentRaster = r; }

	//Get State functions
	UINT GetAlphaTestRef() { return (UINT)(globalSRSBuffer.fAlphaTestRef * 255); }
	BOOL GetDepthEnable() { return m_depthStencilDesc.DepthEnable; }
	bool GetZWriteEnable() { return m_depthStencilDesc.DepthWriteMask==D3D11_DEPTH_WRITE_MASK_ALL; }
	RwCullMode GetCullMode() {
		switch (m_rasterDesc.CullMode)
		{
		case D3D11_CULL_NONE:
			return rwCULLMODECULLNONE;
		case D3D11_CULL_BACK:
			return rwCULLMODECULLBACK;
		case D3D11_CULL_FRONT:
			return rwCULLMODECULLFRONT;
		}
	}
	BOOL GetAlphaBlendEnable() { return m_blendDesc.RenderTarget[0].BlendEnable; };
	RwBlendFunction GetDestAlphaBlend() { return ConvertBlendFunc(m_blendDesc.RenderTarget[0].DestBlend); };
	RwBlendFunction GetSrcAlphaBlend() { return ConvertBlendFunc(m_blendDesc.RenderTarget[0].SrcBlend); };
	RwTextureAddressMode GetTextureAdressUV() { return ConvertTextureAddressMode(m_sampDesc.AddressU); }
	RwTextureAddressMode GetTextureAdressU() { return ConvertTextureAddressMode(m_sampDesc.AddressU); };
	RwTextureAddressMode GetTextureAdressV() { return ConvertTextureAddressMode(m_sampDesc.AddressV); };
	RwTextureFilterMode  GetTextureFilterMode() { return ConvertTextureFilterMode(m_sampDesc.Filter); };
	RwRaster* GetRaster() { return m_pCurrentRaster; }
private://Convert functions. TODO: move to EnumParser
	D3D11_BLEND					ConvertBlendFunc	(RwBlendFunction func);
	RwBlendFunction				ConvertBlendFunc	(D3D11_BLEND func);
	D3D11_STENCIL_OP			ConvertStencilOp	(RwStencilOperation op);
	D3D11_COMPARISON_FUNC		ConvertStencilFunc	(RwStencilFunction func);
	D3D11_TEXTURE_ADDRESS_MODE	ConvertTextureAddressMode(RwTextureAddressMode mode);
	RwTextureAddressMode		ConvertTextureAddressMode( D3D11_TEXTURE_ADDRESS_MODE mode);
	D3D11_FILTER				ConvertTextureFilterMode(RwTextureFilterMode mode);
	RwTextureFilterMode			ConvertTextureFilterMode(D3D11_FILTER mode);
private:
	ID3D11Device*				m_pDevice				= nullptr;
	ID3D11DeviceContext*		m_pDevContext			= nullptr;
	ID3D11BlendState*			m_pBlendState			= nullptr;
	ID3D11RasterizerState*		m_pRasterState			= nullptr;
	ID3D11DepthStencilState*	m_pDepthStencilState	= nullptr;
	ID3D11SamplerState*			m_pSamplerState			= nullptr;
	ID3D11Buffer*				m_pGlobalValuesBuffer	= nullptr;
	RwRaster*					m_pCurrentRaster		= nullptr;

	D3D11_RASTERIZER_DESC		m_rasterDesc		{};
	D3D11_DEPTH_STENCIL_DESC	m_depthStencilDesc	{};
	D3D11_BLEND_DESC			m_blendDesc			{};
	D3D11_SAMPLER_DESC			m_sampDesc			{};

	UINT						m_currentStencilRef = 0;
	UINT						m_currentAlphaTestType = 1;
};
extern CD3D1XStateManager* g_pStateMgr;
#endif // D3D1XStateManager_h__
