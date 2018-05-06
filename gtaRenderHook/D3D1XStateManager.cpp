// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "stdafx.h"
#include "D3D1XStateManager.h"
#include "D3DRenderer.h"
#include "CDebug.h"
#include "D3D1XTexture.h"
#include "RwD3D1XEngine.h"
#include "D3DSpecificHelpers.h"


CD3D1XStateManager::CD3D1XStateManager()
{
	auto dev = GET_D3D_DEVICE;
	auto context = GET_D3D_CONTEXT;
	// initialize render target list
	for (int i = 0; i < 8; i++)
		m_pCurrentRenderTargets[i] = nullptr;
	// default blend description with blending disabled
	m_blendDesc.RenderTarget[0].BlendEnable				= false;
	m_blendDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND_SRC_ALPHA;
	m_blendDesc.RenderTarget[0].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
	m_blendDesc.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	m_blendDesc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;
	m_blendDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;
	m_blendDescOld = m_blendDesc;
	// create blend state
	if (!CALL_D3D_API(dev->CreateBlendState(&m_blendDesc, &m_pBlendState),
		"Failed to create blend state with default blend description."))
		return;

	// depth test parameters
	m_depthStencilDesc.DepthEnable		= true;
	m_depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	m_depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;

	// stencil test parameters
	m_depthStencilDesc.StencilEnable	= true;
	m_depthStencilDesc.StencilReadMask	= 0xFF;
	m_depthStencilDesc.StencilWriteMask = 0xFF;

	// stencil operations if pixel is front-facing
	m_depthStencilDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	m_depthStencilDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.FrontFace.StencilFunc		= D3D11_COMPARISON_ALWAYS;

	// stencil operations if pixel is back-facing
	m_depthStencilDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
	m_depthStencilDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	m_depthStencilDescOld = m_depthStencilDesc;

	// create depth stencil state
	if (!CALL_D3D_API(dev->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState), 
		"Failed to create depth stencil state"))
		return;

	// default rasterizer state description
	// TODO: review parameters, maybe add some of them to setting file or set better values  
	m_rasterDesc.AntialiasedLineEnable = false;
	m_rasterDesc.CullMode = D3D11_CULL_BACK;
	m_rasterDesc.DepthBias = 0;
	m_rasterDesc.DepthBiasClamp = 0.0f;
	m_rasterDesc.DepthClipEnable = true;
	m_rasterDesc.FillMode = D3D11_FILL_SOLID;
	m_rasterDesc.FrontCounterClockwise = true;
	m_rasterDesc.MultisampleEnable = true;
	m_rasterDesc.ScissorEnable = false;
	m_rasterDesc.SlopeScaledDepthBias = 0.0f;

	m_rasterDescOld = m_rasterDesc;

	// create the rasterizer state.
	if (!CALL_D3D_API(dev->CreateRasterizerState(&m_rasterDesc, &m_pRasterState),
		"Failed to create rasterizer state."))
		return;

	// default sampler state
	// TODO: add control for anisotropy as well as lod quality
	m_sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m_sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	m_sampDesc.MinLOD = 0;
	m_sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_sampDesc.MaxAnisotropy = 16;

	m_sampDescOld = m_sampDesc;

	// create default sampler state
	if (!CALL_D3D_API(dev->CreateSamplerState(&m_sampDesc, &m_pSamplerState), "Failed to create sampler state"))
		return;

	// default shadow sampler state
	// TODO: remove it from here, maybe add some sampler state array to handle different sampler states for different stuff. also decide on format of structure filling 
	m_compSampDesc={ D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,// Filter;
		D3D11_TEXTURE_ADDRESS_BORDER, //AddressU;
		D3D11_TEXTURE_ADDRESS_BORDER, //AddressV;
		D3D11_TEXTURE_ADDRESS_BORDER, //AddressW;
		0,//MipLODBias;
		0,//MaxAnisotropy;
		D3D11_COMPARISON_LESS , //ComparisonFunc;
		1.0,1.0,1.0,1.0,//BorderColor[ 4 ];
		0,//MinLOD;
		0//MaxLOD;
	};
	// create shadow sampler state
	if (!CALL_D3D_API(dev->CreateSamplerState(&m_compSampDesc, &m_pCompSamplerState), "Failed to create shadow sampler state"))
		return;
	
	// initialize global shader render state buffer
	// TODO: convert it to existing buffer template
	g_shaderRenderStateBuffer = {};
	g_shaderRenderStateBuffer.fAlphaTestRef = 0.5f;
	g_shaderRenderStateBuffer.uiAlphaTestType = 1;

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShaderRenderStateBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	if (!CALL_D3D_API(dev->CreateBuffer(&bd, nullptr, &m_pGlobalValuesBuffer), 
		"Failed to create constant buffer"))
		return;
	// set render states
	context->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
	context->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	context->RSSetState(m_pRasterState);
	context->DSSetSamplers(0, 1, &m_pSamplerState);
	context->PSSetSamplers(0, 1, &m_pSamplerState);
	context->PSSetSamplers(1, 1, &m_pCompSamplerState);

	// set constant buffers
	// TODO: set buffers only for required shaders and with dynamic alignment perhaps
	context->PSSetConstantBuffers(0, 1, &m_pGlobalValuesBuffer);
	context->VSSetConstantBuffers(0, 1, &m_pGlobalValuesBuffer);
	context->HSSetConstantBuffers(0, 1, &m_pGlobalValuesBuffer);
	context->DSSetConstantBuffers(0, 1, &m_pGlobalValuesBuffer);
	context->CSSetConstantBuffers(0, 1, &m_pGlobalValuesBuffer);

	m_pBlendState_Default = m_pBlendState;

	// default alpha blending state
	m_blendDesc.RenderTarget[0].BlendEnable=true;
	if (!CALL_D3D_API(dev->CreateBlendState(&m_blendDesc, &m_pBlendState_AlphaBlend), 
		"Failed to create alpha blending state."))
		return;
	// blend source*alpha over destination
	m_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_ONE;
	if (!CALL_D3D_API(dev->CreateBlendState(&m_blendDesc, &m_pBlendState_BlendDestOne),
		"Failed to create blend state"))
		return;
	// some other blending used in gta sa, probably should remove this
	m_blendDesc.RenderTarget[0].BlendEnable = false;
	if (!CALL_D3D_API(dev->CreateBlendState(&m_blendDesc, &m_pBlendState_NoBlendDestOne), 
		"Failed to create blend state"))
		return;
	// additive blending(alpha is additive as well)
	m_blendDesc.RenderTarget[0].BlendEnable = true;
	m_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_ONE;
	m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	if (!CALL_D3D_API(dev->CreateBlendState(&m_blendDesc, &m_pBlendState_BlendAdditive), 
		"Failed to create blend state"))
		return;	
}

CD3D1XStateManager::~CD3D1XStateManager()
{ 
	if (m_pGlobalValuesBuffer)	m_pGlobalValuesBuffer->Release();
	if (m_pDepthStencilState)	m_pDepthStencilState->Release();
	if (m_pRasterState)			m_pRasterState->Release();
	if (m_pSamplerState)		m_pSamplerState->Release();
	if (m_pCompSamplerState)	m_pCompSamplerState->Release();
}

void CD3D1XStateManager::SetCullMode(RwCullMode mode)
{
	auto cullmode = CD3D1XEnumParser::ConvertCullMode(mode);
	if (m_rasterDesc.CullMode != cullmode) {
		m_rasterDesc.CullMode = cullmode;
		m_bRasterDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetFillMode(D3D11_FILL_MODE mode)
{
	if (m_rasterDesc.FillMode != mode) {
		m_rasterDesc.FillMode = mode;
		m_bRasterDescReqUpdate = true;
	}
}



void CD3D1XStateManager::SetAlphaTestEnable(bool bEnable)
{
	auto context = GET_D3D_CONTEXT;
	if (!bEnable)
		m_currentAlphaTestType = g_shaderRenderStateBuffer.uiAlphaTestType;

	if (bEnable) {
		g_shaderRenderStateBuffer.uiAlphaTestType = m_currentAlphaTestType;
		context->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &g_shaderRenderStateBuffer, 0, 0);
	}
	else
	{
		g_shaderRenderStateBuffer.uiAlphaTestType = 7;
		context->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &g_shaderRenderStateBuffer, 0, 0);
	}
}

void CD3D1XStateManager::SetDepthEnable(BOOL bEnable)
{
	if (m_depthStencilDesc.DepthEnable != bEnable) {
		m_depthStencilDesc.DepthEnable = bEnable;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetZWriteEnable(bool bEnable)
{
	if (m_depthStencilDesc.DepthWriteMask != (D3D11_DEPTH_WRITE_MASK)bEnable) {
		m_depthStencilDesc.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)bEnable;
		m_bDepthStencilDescReqUpdate = true;
	}
}
void CD3D1XStateManager::SetStencilEnable(BOOL bEnable)
{
	if (m_depthStencilDesc.StencilEnable != bEnable) {
		m_depthStencilDesc.StencilEnable = bEnable;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetStencilFail(RwStencilOperation op)
{
	D3D11_STENCIL_OP d3dOp = CD3D1XEnumParser::ConvertStencilOp(op);

	if (m_depthStencilDesc.FrontFace.StencilFailOp != d3dOp) {
		m_depthStencilDesc.FrontFace.StencilFailOp = d3dOp;
		m_depthStencilDesc.BackFace.StencilFailOp = d3dOp;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetStencilZFail(RwStencilOperation op)
{
	D3D11_STENCIL_OP d3dOp = CD3D1XEnumParser::ConvertStencilOp(op);

	if (m_depthStencilDesc.FrontFace.StencilDepthFailOp != d3dOp) {
		m_depthStencilDesc.FrontFace.StencilDepthFailOp = d3dOp;
		m_depthStencilDesc.BackFace.StencilDepthFailOp = d3dOp;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetStencilPass(RwStencilOperation op)
{
	D3D11_STENCIL_OP d3dOp = CD3D1XEnumParser::ConvertStencilOp(op);

	if (m_depthStencilDesc.FrontFace.StencilPassOp != d3dOp) {
		m_depthStencilDesc.FrontFace.StencilPassOp = d3dOp;
		m_depthStencilDesc.BackFace.StencilPassOp = d3dOp;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetStencilFunc(RwStencilFunction fn)
{
	D3D11_COMPARISON_FUNC d3dFn = CD3D1XEnumParser::ConvertStencilFunc(fn);

	if (m_depthStencilDesc.FrontFace.StencilFunc != d3dFn) {
		m_depthStencilDesc.FrontFace.StencilFunc = d3dFn;
		m_depthStencilDesc.BackFace.StencilFunc = d3dFn;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetStencilFuncRef(UINT ref)
{

	if (m_currentStencilRef != ref) {
		m_currentStencilRef = ref;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetStencilFuncMask(int mask)
{
	if (m_depthStencilDesc.StencilReadMask != mask) {
		m_depthStencilDesc.StencilReadMask = mask;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetStencilFuncWriteMask(int mask)
{
	if (m_depthStencilDesc.StencilWriteMask != mask) {
		m_depthStencilDesc.StencilWriteMask = mask;
		m_bDepthStencilDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetAlphaBlendEnable(BOOL bEnable)
{
	if (m_blendDesc.RenderTarget[0].BlendEnable != bEnable) {
		m_blendDesc.RenderTarget[0].BlendEnable = bEnable;
		m_bBlendDescReqUpdate = true;

	}
}
void CD3D1XStateManager::SetDestAlphaBlend(RwBlendFunction func)
{
	D3D11_BLEND blendFunc = CD3D1XEnumParser::ConvertBlendFunc(func);

	if (m_blendDesc.RenderTarget[0].DestBlend != blendFunc) {
		m_blendDesc.RenderTarget[0].DestBlend = blendFunc;
		m_bBlendDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetSrcAlphaBlend(RwBlendFunction func)
{
	D3D11_BLEND blendFunc = CD3D1XEnumParser::ConvertBlendFunc(func);

	if (m_blendDesc.RenderTarget[0].SrcBlend != blendFunc) {
		m_blendDesc.RenderTarget[0].SrcBlend = blendFunc;
		m_bBlendDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetTextureAdressUV(RwTextureAddressMode mode)
{
	D3D11_TEXTURE_ADDRESS_MODE d3dMode = CD3D1XEnumParser::ConvertTextureAddressMode(mode);

	if (m_sampDesc.AddressU != d3dMode || m_sampDesc.AddressV != d3dMode) {
		m_sampDesc.AddressU = d3dMode;
		m_sampDesc.AddressV = d3dMode;
		m_sampDesc.AddressW = d3dMode;
		m_bSampDescReqUpdate = true;
		
	}
}

void CD3D1XStateManager::SetTextureAdressU(RwTextureAddressMode mode)
{
	D3D11_TEXTURE_ADDRESS_MODE d3dMode = CD3D1XEnumParser::ConvertTextureAddressMode(mode);

	if (m_sampDesc.AddressU != d3dMode) {
		m_sampDesc.AddressU = d3dMode;
		m_bSampDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetTextureAdressV(RwTextureAddressMode mode)
{
	D3D11_TEXTURE_ADDRESS_MODE d3dMode = CD3D1XEnumParser::ConvertTextureAddressMode(mode);

	if (m_sampDesc.AddressV != d3dMode) {
		m_sampDesc.AddressV = d3dMode;
		m_bSampDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetTextureFilterMode(RwTextureFilterMode mode)
{
	D3D11_FILTER d3dMode = CD3D1XEnumParser::ConvertTextureFilterMode(mode);

	if (m_sampDesc.Filter != d3dMode) {
		m_sampDesc.Filter = d3dMode;
		m_bSampDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetTextureBorderColor(RwRGBA color)
{
	float borderColor[] = { color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, color.alpha / 255.0f };
	if (m_sampDesc.BorderColor[0] != borderColor[0] || m_sampDesc.BorderColor[1] != borderColor[1] || 
		m_sampDesc.BorderColor[2] != borderColor[2] || m_sampDesc.BorderColor[3] != borderColor[3]) {

		m_sampDesc.BorderColor[0] = borderColor[0];
		m_sampDesc.BorderColor[1] = borderColor[1];
		m_sampDesc.BorderColor[2] = borderColor[2];
		m_sampDesc.BorderColor[3] = borderColor[3];

		m_bSampDescReqUpdate = true;
	}
}

void CD3D1XStateManager::SetTextureEnable(UINT enable)
{
	if (g_shaderRenderStateBuffer.bHasTexture != enable) {
		g_shaderRenderStateBuffer.bHasTexture = enable;
		m_bGlobalValuesReqUpdate = true;
	}
}

void CD3D1XStateManager::SetAlphaTestFunc(RwAlphaTestFunction func)
{
	UINT atType = 1;
	switch (func)
	{
	case rwALPHATESTFUNCTIONNEVER:
		atType = 0;
		break;
	case rwALPHATESTFUNCTIONLESS:
		atType = 3;
		break;
	case rwALPHATESTFUNCTIONEQUAL:
		atType = 6;
		break;
	case rwALPHATESTFUNCTIONLESSEQUAL:
		atType = 4;
		break;
	case rwALPHATESTFUNCTIONNOTEQUAL:
		atType = 5;
		break;
	case rwALPHATESTFUNCTIONGREATEREQUAL:
		atType = 2;
		break;
	case rwALPHATESTFUNCTIONALWAYS:
		atType = 0;
		break;
	default:
		break;
	}
	if (g_shaderRenderStateBuffer.uiAlphaTestType != atType) {
		g_shaderRenderStateBuffer.uiAlphaTestType = atType;
		m_currentAlphaTestType = atType;
		m_bGlobalValuesReqUpdate = true;
	}
}

void CD3D1XStateManager::SetAlphaTestRef(float ref)
{
	if (fabs(g_shaderRenderStateBuffer.fAlphaTestRef - ref)>0.001f){
		g_shaderRenderStateBuffer.fAlphaTestRef = ref;
		m_bGlobalValuesReqUpdate = true;
	}
}

void CD3D1XStateManager::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	if (m_currentPrimitiveTopology != topology) {
		m_bTopologyReqUpdate = true;
		m_currentPrimitiveTopology = topology;
	}
}

void CD3D1XStateManager::SetInputLayout(ID3D11InputLayout * layout)
{
	if (m_pCurrentInputLayout != layout) {
		m_bInputLayoutReqUpdate=true;
		m_pCurrentInputLayout = layout;
	}
}

void CD3D1XStateManager::SetVertexBuffer(ID3D11Buffer * buffer, UINT stride, UINT offset)
{
	if (m_pCurrentVertexBuffer != buffer || m_currentVBufferStride != stride || m_currentVBufferOffset != offset) {
		m_pCurrentVertexBuffer = buffer;
		m_currentVBufferStride = stride;
		m_currentVBufferOffset = offset;
		m_bVertexBufferReqUpdate = true;
	}
}

void CD3D1XStateManager::SetIndexBuffer(ID3D11Buffer * buffer)
{
	if (m_pCurrentIndexBuffer != buffer) {
		m_pCurrentIndexBuffer = buffer;
		m_bIndexBufferReqUpdate = true;
	}
}

void CD3D1XStateManager::SetScreenSize(float w, float h)
{
	if (fabs(g_shaderRenderStateBuffer.fScreenWidth- w) > 0.001f || fabs(g_shaderRenderStateBuffer.fScreenHeight - h) > 0.001f) {
		g_shaderRenderStateBuffer.fScreenWidth = w;
		g_shaderRenderStateBuffer.fScreenHeight = h;
		m_bGlobalValuesReqUpdate = true;
	}
}

void CD3D1XStateManager::SetRaster(RwRaster * raster, int Stage)
{
	auto context = GET_D3D_CONTEXT;
	// todo: add lazy set
	if (raster != nullptr) {
		auto res = GetD3D1XRaster(raster)->resourse->GetSRV();
		context->PSSetShaderResources(Stage, 1, &res);
	}
	else {
		ID3D11ShaderResourceView* res = nullptr;
		context->PSSetShaderResources(Stage, 1, &res);
	}
}

void CD3D1XStateManager::SetRasterCS(RwRaster * raster, int Stage)
{
	auto context = GET_D3D_CONTEXT;
	// todo: add lazy set
	if (raster != nullptr) {
		auto res = GetD3D1XRaster(raster)->resourse->GetSRV();
		context->CSSetShaderResources(Stage, 1, &res);
	}
	else {
		ID3D11ShaderResourceView* res = nullptr;
		context->CSSetShaderResources(Stage, 1, &res);
	}
}

void CD3D1XStateManager::SetSunDir(RwV3d * vec, float dnBalance)
{
	auto context = GET_D3D_CONTEXT;
	// todo: move out of this, remove hardcoded constants
	g_shaderRenderStateBuffer.vSunDir = { vec->x,vec->y,vec->z, 1.0f-dnBalance };
	context->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &g_shaderRenderStateBuffer, 0, 0);
}

void CD3D1XStateManager::SetLightCount(int count)
{
	auto context = GET_D3D_CONTEXT;
	g_shaderRenderStateBuffer.uiLightCount=count;
	context->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &g_shaderRenderStateBuffer, 0, 0);
}

void CD3D1XStateManager::SetFogStart(float start)
{
	auto context = GET_D3D_CONTEXT;
	g_shaderRenderStateBuffer.fFogStart = start;
	context->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &g_shaderRenderStateBuffer, 0, 0);
}

void CD3D1XStateManager::SetFogRange(float range)
{
	auto context = GET_D3D_CONTEXT;
	g_shaderRenderStateBuffer.fFogRange = range;
	context->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &g_shaderRenderStateBuffer, 0, 0);
}

void CD3D1XStateManager::SetRenderTargets(unsigned int count, ID3D11RenderTargetView * const * ppRenderTargets, ID3D11DepthStencilView * pDepthTarget)
{
	if (ppRenderTargets == nullptr) {
		for (UINT i = 0; i < count; i++) {
			if (m_pCurrentRenderTargets[i] != nullptr) {
				m_currentRenderTargetNeedsUpdate = max(m_currentRenderTargetNeedsUpdate, i+1);
				m_pCurrentRenderTargets[i] = nullptr;
				m_bRenderTargetReqUpdate = true;
			}
		}
		if (pDepthTarget != m_pCurrentDepthStencilView) {
			m_pCurrentDepthStencilView = pDepthTarget;
			m_bRenderTargetReqUpdate = true;
		}
	}
	else
	{
		for (UINT i = 0; i < count; i++) {
			if (m_pCurrentRenderTargets[i] != ppRenderTargets[i]) {
				m_currentRenderTargetNeedsUpdate = max(m_currentRenderTargetNeedsUpdate, i + 1);
				m_pCurrentRenderTargets[i] = ppRenderTargets[i];
				m_bRenderTargetReqUpdate = true;
			}
		}
		if (pDepthTarget != m_pCurrentDepthStencilView) {
			m_pCurrentDepthStencilView = pDepthTarget;
			m_bRenderTargetReqUpdate = true;
		}
	}
}

void CD3D1XStateManager::SetViewport(const D3D11_VIEWPORT &vp)
{
	m_viewport = vp;
}


void CD3D1XStateManager::FlushStates()
{
	// TODO: split state manager into different parts, that manage different states, so it would be easier to understand and to refactor
	// If we have any state changed we should update it.
	auto context = GET_D3D_CONTEXT;
	auto dev = GET_D3D_DEVICE;
	// Render targets
	FlushRenderTargets();

	if (IsViewportRequiresUpdate()) {
		context->RSSetViewports(1, &m_viewport);
		m_viewportOld = m_viewport;
	}
	// Depth and stencil state.
	if (IsDepthDescRequiresUpdate()) {
		if (m_pDepthStencilState) m_pDepthStencilState->Release();

		CALL_D3D_API(dev->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState), 
			"Failed to create depth stencil state");

		context->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
		m_depthStencilDescOld = m_depthStencilDesc;
	}
	// Blend state.
	if (IsBlendDescRequiresUpdate()) {
		
		if(m_pBlendState != m_pBlendState_Default  && m_pBlendState != m_pBlendState_AlphaBlend && m_pBlendState != m_pBlendState_BlendDestOne&&
			m_pBlendState != m_pBlendState_BlendAdditive && m_pBlendState != m_pBlendState_NoBlendDestOne)
			if (m_pBlendState) { 
				m_pBlendState->Release();
				m_pBlendState = nullptr;
			}

		if (m_blendDesc.RenderTarget[0].SrcBlend == D3D11_BLEND::D3D11_BLEND_SRC_ALPHA &&
			m_blendDesc.RenderTarget[0].DestBlend == D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA) {
			if (!m_blendDesc.RenderTarget[0].BlendEnable) {
				m_pBlendState = m_pBlendState_Default;
			}
			else {
				m_pBlendState = m_pBlendState_AlphaBlend;
			}
			context->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
		}
		else if (m_blendDesc.RenderTarget[0].DestBlend == D3D11_BLEND::D3D11_BLEND_ONE) 
		{
			if (m_blendDesc.RenderTarget[0].BlendEnable) {
				if (m_blendDesc.RenderTarget[0].SrcBlend == D3D11_BLEND::D3D11_BLEND_SRC_ALPHA)
					m_pBlendState = m_pBlendState_BlendDestOne;
				else if (m_blendDesc.RenderTarget[0].SrcBlend == D3D11_BLEND::D3D11_BLEND_ONE)
					m_pBlendState = m_pBlendState_BlendAdditive;
				else
					g_pDebug->printMsg("Blend state: SRC:" + to_string(m_blendDesc.RenderTarget[0].SrcBlend) + " DST: " +
						to_string(m_blendDesc.RenderTarget[0].DestBlend) + " Blend: " +
						to_string(m_blendDesc.RenderTarget[0].BlendEnable), 2);
			}
			else if(m_blendDesc.RenderTarget[0].SrcBlend == D3D11_BLEND::D3D11_BLEND_SRC_ALPHA)
				m_pBlendState = m_pBlendState_NoBlendDestOne;
			else
				g_pDebug->printMsg("Blend state: SRC:" + to_string(m_blendDesc.RenderTarget[0].SrcBlend) + " DST: " +
					to_string(m_blendDesc.RenderTarget[0].DestBlend) + " Blend: " +
					to_string(m_blendDesc.RenderTarget[0].BlendEnable),2);
			context->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
		}
		else {
			
			CALL_D3D_API(dev->CreateBlendState(&m_blendDesc, &m_pBlendState), "Failed to create blend state");
			g_pDebug->printMsg("Blend state realtime create event: SRC:" + 
				to_string(m_blendDesc.RenderTarget[0].SrcBlend) + " DST: " +
				to_string(m_blendDesc.RenderTarget[0].DestBlend) + " Blend: " +
				to_string(m_blendDesc.RenderTarget[0].BlendEnable),2);
			//m_pBlendState;m_pBlendStateCache[found];
			context->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
		}
		m_blendDescOld = m_blendDesc;
	}
	// Raster state.
	if (IsRasterDescRequiresUpdate()) {
		if (m_pRasterState) m_pRasterState->Release();

		CALL_D3D_API(dev->CreateRasterizerState(&m_rasterDesc, &m_pRasterState), "Failed to create rasterizer state");

		context->RSSetState(m_pRasterState);
		m_rasterDescOld = m_rasterDesc;
	}
	// Sampler state.
	if (IsSamplerDescRequiresUpdate()) {
		if (m_pSamplerState) m_pSamplerState->Release();

		CALL_D3D_API(dev->CreateSamplerState(&m_sampDesc, &m_pSamplerState), "Failed to create sampler state");

		context->DSSetSamplers(0, 1, &m_pSamplerState);
		context->PSSetSamplers(0, 1, &m_pSamplerState);
		m_sampDescOld = m_sampDesc;
		m_bSampDescReqUpdate = false;
	}
	// Primitive topology.
	if (m_bTopologyReqUpdate) {
		context->IASetPrimitiveTopology(m_currentPrimitiveTopology);
		m_bTopologyReqUpdate = false;
	}
	
	// Input layout.
	if (m_bInputLayoutReqUpdate) {
		context->IASetInputLayout(m_pCurrentInputLayout);
		m_bInputLayoutReqUpdate = false;
	}
	// First texture.
	if (m_pOldRaster!= m_pCurrentRaster) {

		if (m_pCurrentRaster != nullptr) {
			RwD3D1XRaster* d3dRaster = GetD3D1XRaster(m_pCurrentRaster);
			if (d3dRaster->resourse!=nullptr) {
				SetTextureEnable(true);
				auto srv = d3dRaster->resourse->GetSRV();
				// TODO: add ability to set raster-to-shader binding inside pipeline, to allow setting textures to different shaders
				context->PSSetShaderResources(0, 1, &srv);
				//context->DSSetShaderResources(0, 1, &srv);
			}else
				SetTextureEnable(false);
		}
		else
		{
			SetTextureEnable(false);
			ID3D11ShaderResourceView* srv[] = { nullptr };
			context->PSSetShaderResources(0, 1, srv);
			//context->DSSetShaderResources(0, 1, srv);
		}
		m_pOldRaster = m_pCurrentRaster;
	}
	// Vertex Buffer.
	if (m_bVertexBufferReqUpdate) {
		context->IASetVertexBuffers(0,1,&m_pCurrentVertexBuffer,&m_currentVBufferStride,&m_currentVBufferOffset);
		m_bVertexBufferReqUpdate = false;
	}
	// Index Buffer.
	if (m_bIndexBufferReqUpdate) {
		context->IASetIndexBuffer(m_pCurrentIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_bIndexBufferReqUpdate = false;
	}
	// Global values CB.
	if (m_bGlobalValuesReqUpdate) {
		context->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &g_shaderRenderStateBuffer, 0, 0);

		m_bGlobalValuesReqUpdate = false;
	}
}

void CD3D1XStateManager::FlushRenderTargets()
{
	auto context = GET_D3D_CONTEXT;
	if (m_bRenderTargetReqUpdate) {
		for (int i = m_currentRenderTargetNeedsUpdate; i < 8; i++) {
			m_pCurrentRenderTargets[i] = nullptr;
		}
		if (m_currentRenderTargetNeedsUpdate == 0)
			context->OMSetRenderTargets(0, nullptr, m_pCurrentDepthStencilView);
		else
			context->OMSetRenderTargets(8, m_pCurrentRenderTargets, m_pCurrentDepthStencilView);
		m_currentRenderTargetNeedsUpdate = 0;
		m_bRenderTargetReqUpdate = false;
	}
}

bool CD3D1XStateManager::IsDepthDescRequiresUpdate()
{
	return m_depthStencilDescOld.DepthEnable				!= m_depthStencilDesc.DepthEnable ||
		m_depthStencilDescOld.StencilWriteMask				!= m_depthStencilDesc.StencilWriteMask ||
		m_depthStencilDescOld.StencilReadMask				!= m_depthStencilDesc.StencilReadMask ||
		m_depthStencilDescOld.FrontFace.StencilFunc			!= m_depthStencilDesc.FrontFace.StencilFunc ||
		m_depthStencilDescOld.FrontFace.StencilPassOp		!= m_depthStencilDesc.FrontFace.StencilPassOp ||
		m_depthStencilDescOld.FrontFace.StencilDepthFailOp	!= m_depthStencilDesc.FrontFace.StencilDepthFailOp ||
		m_depthStencilDescOld.FrontFace.StencilFailOp		!= m_depthStencilDesc.FrontFace.StencilFailOp ||
		m_depthStencilDescOld.StencilEnable					!= m_depthStencilDesc.StencilEnable ||
		m_depthStencilDescOld.DepthWriteMask				!= m_depthStencilDesc.DepthWriteMask ||
		m_depthStencilDescOld.DepthFunc						!= m_depthStencilDesc.DepthFunc;
}
bool CD3D1XStateManager::IsRasterDescRequiresUpdate()
{
	return	m_rasterDescOld.CullMode!= m_rasterDesc.CullMode ||
			m_rasterDescOld.FillMode != m_rasterDesc.FillMode;
}
bool CD3D1XStateManager::IsBlendDescRequiresUpdate()
{
	return	m_blendDescOld.RenderTarget[0].SrcBlend != m_blendDesc.RenderTarget[0].SrcBlend ||
		m_blendDescOld.RenderTarget[0].DestBlend != m_blendDesc.RenderTarget[0].DestBlend ||
		m_blendDescOld.RenderTarget[0].SrcBlendAlpha != m_blendDesc.RenderTarget[0].SrcBlendAlpha ||
		m_blendDescOld.RenderTarget[0].DestBlendAlpha != m_blendDesc.RenderTarget[0].DestBlendAlpha ||
			m_blendDescOld.RenderTarget[0].BlendEnable	!= m_blendDesc.RenderTarget[0].BlendEnable;
}

bool CD3D1XStateManager::IsSamplerDescRequiresUpdate()
{
	return		m_sampDescOld.AddressU		!= m_sampDesc.AddressU ||
				m_sampDescOld.AddressV		!= m_sampDesc.AddressV ||
				m_sampDescOld.AddressW		!= m_sampDesc.AddressW ||
				m_sampDescOld.Filter		!= m_sampDesc.Filter ||
		m_sampDescOld.BorderColor[0] - m_sampDesc.BorderColor[0] >0.01 ||
		m_sampDescOld.BorderColor[1] - m_sampDesc.BorderColor[1] >0.01 ||
		m_sampDescOld.BorderColor[2] - m_sampDesc.BorderColor[2] >0.01 ||
		m_sampDescOld.BorderColor[3] - m_sampDesc.BorderColor[3] >0.01;
}

bool CD3D1XStateManager::IsViewportRequiresUpdate()
{
	return m_viewport.Height!=m_viewportOld.Height ||
		m_viewport.Width != m_viewportOld.Width ||
		m_viewport.TopLeftX != m_viewportOld.TopLeftX ||
		m_viewport.TopLeftY != m_viewportOld.TopLeftY ||
		m_viewport.MaxDepth != m_viewportOld.MaxDepth ||
		m_viewport.MinDepth != m_viewportOld.MinDepth;
}