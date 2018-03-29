#ifndef D3D1XStateManager_h__
#define D3D1XStateManager_h__
#include <map>
#include "D3D1XConstantBuffer.h"
#include "D3D1XStructuredBuffer.h"
class CD3DRenderer;
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

extern ShaderRenderStateBuffer globalSRSBuffer;
class CD3D1XStateManager
{
public:
	CD3D1XStateManager();
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
	void SetPrimitiveTopology		(D3D11_PRIMITIVE_TOPOLOGY topology);
	void SetInputLayout				(ID3D11InputLayout* layout);
	void SetVertexBuffer			(ID3D11Buffer* buffer, UINT stride,UINT offset);
	void SetIndexBuffer				(ID3D11Buffer* buffer);
	
	void SetScreenSize				(float w, float h);
	void SetRaster(RwRaster*r) { if (m_pCurrentRaster != r) { m_pCurrentRaster = r; m_bRasterReqUpdate = true; } }
	void SetRaster(RwRaster * raster, int Stage);
	void SetRasterCS(RwRaster * raster, int Stage);
	template <class T>
	void SetConstantBufferPS(CD3D1XConstantBuffer<T> * buffer, int Stage);
	template <class T>
	void SetConstantBufferCS(CD3D1XConstantBuffer<T> * buffer, int Stage);
	template <class T>
	void SetStructuredBufferPS(CD3D1XStructuredBuffer<T> * buffer, int Stage);
	template <class T>
	void SetStructuredBufferCS(CD3D1XStructuredBuffer<T> * buffer, int Stage);
	void SetSunDir(RwV3d* vec);
	void SetLightCount(int count);
	void SetFogStart(float start);
	void SetFogRange(float range);
	void SetRenderTargets(unsigned int count, ID3D11RenderTargetView* const * ppRenderTargets, ID3D11DepthStencilView* pDepthTarget);
	void SetViewport(D3D11_VIEWPORT vp);

	void FlushStates();
	void FlushRenderTargets();

	//Get State functions
	UINT GetAlphaTestRef() { return (UINT)(globalSRSBuffer.fAlphaTestRef * 255); }
	BOOL GetDepthEnable() { return m_depthStencilDesc.DepthEnable; }
	bool GetZWriteEnable() { return m_depthStencilDesc.DepthWriteMask==D3D11_DEPTH_WRITE_MASK_ALL; }
	bool IsDepthDescRequiresUpdate();
	bool IsRasterDescRequiresUpdate();
	bool IsBlendDescRequiresUpdate();
	bool IsSamplerDescRequiresUpdate();
	bool IsViewportRequiresUpdate();
	RwCullMode GetCullMode() {
		switch (m_rasterDesc.CullMode)
		{		
		case D3D11_CULL_BACK:
			return rwCULLMODECULLBACK;
		case D3D11_CULL_FRONT:
			return rwCULLMODECULLFRONT;
		default:
			return rwCULLMODECULLNONE;
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
	std::string GetShaderModel(D3D_FEATURE_LEVEL featureLevel){
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
	ID3D11BlendState*			m_pBlendState			= nullptr;
	std::list<D3D11_BLEND_DESC>				m_pBlendStateDescCache;
	std::map<int, ID3D11BlendState*>		m_pBlendStateCache;
	CComPtr<ID3D11BlendState>	m_pBlendState_Default;
	CComPtr<ID3D11BlendState>   m_pBlendState_NoBlend;
	CComPtr<ID3D11BlendState>   m_pBlendState_BlendDestOne;
	CComPtr<ID3D11BlendState>   m_pBlendState_NoBlendDestOne;
	CComPtr<ID3D11BlendState>   m_pBlendState_BlendAdditive;
	ID3D11RasterizerState*		m_pRasterState			= nullptr;
	ID3D11DepthStencilState*	m_pDepthStencilState	= nullptr;
	ID3D11SamplerState*			m_pSamplerState			= nullptr;
	ID3D11SamplerState*			m_pCompSamplerState		= nullptr;
	ID3D11Buffer*				m_pGlobalValuesBuffer	= nullptr;
	RwRaster*					m_pCurrentRaster		= nullptr;
	RwRaster*					m_pOldRaster			= nullptr;
	ID3D11InputLayout*			m_pCurrentInputLayout	= nullptr;
	ID3D11Buffer*				m_pCurrentVertexBuffer  = nullptr;
	ID3D11Buffer*				m_pCurrentIndexBuffer	= nullptr;
	ID3D11RenderTargetView*     m_pCurrentRenderTargets[8];
	ID3D11DepthStencilView*     m_pCurrentDepthStencilView = nullptr;

	D3D11_RASTERIZER_DESC		m_rasterDesc		{};
	D3D11_RASTERIZER_DESC		m_rasterDescOld		{};
	D3D11_DEPTH_STENCIL_DESC	m_depthStencilDesc  {};
	D3D11_DEPTH_STENCIL_DESC	m_depthStencilDescOld {};
	D3D11_BLEND_DESC			m_blendDesc			{};
	D3D11_BLEND_DESC			m_blendDescOld		{};
	D3D11_SAMPLER_DESC			m_sampDesc			{};
	D3D11_SAMPLER_DESC			m_sampDescOld		{};
	D3D11_SAMPLER_DESC			m_compSampDesc		{};
	D3D10_PRIMITIVE_TOPOLOGY	m_currentPrimitiveTopology;
	D3D11_VIEWPORT				m_viewport{};
	D3D11_VIEWPORT				m_viewportOld{};

	UINT						m_currentStencilRef		= 0;
	UINT						m_currentAlphaTestType	= 1;
	UINT						m_currentVBufferStride  = -1;
	UINT						m_currentVBufferOffset = -1;
	UINT						m_currentRenderTargetNeedsUpdate  = 0;

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
inline void CD3D1XStateManager::SetConstantBufferPS(CD3D1XConstantBuffer<T>* buffer, int Stage)
{
	auto buf = buffer->getBuffer();
	GET_D3D_CONTEXT->PSSetConstantBuffers(Stage, 1, &buf);
}

template<class T>
inline void CD3D1XStateManager::SetConstantBufferCS(CD3D1XConstantBuffer<T>* buffer, int Stage)
{
	auto buf = buffer->getBuffer();
	GET_D3D_CONTEXT->CSSetConstantBuffers(Stage, 1, &buf);
}

template<class T>
inline void CD3D1XStateManager::SetStructuredBufferPS(CD3D1XStructuredBuffer<T>* buffer, int Stage)
{
	auto buf = buffer->getSRV();
	GET_D3D_CONTEXT->PSSetShaderResources(Stage, 1, &buf);
}

template<class T>
inline void CD3D1XStateManager::SetStructuredBufferCS(CD3D1XStructuredBuffer<T>* buffer, int Stage)
{
	auto buf = buffer->getSRV();
	GET_D3D_CONTEXT->CSSetShaderResources(Stage, 1, &buf);
}
