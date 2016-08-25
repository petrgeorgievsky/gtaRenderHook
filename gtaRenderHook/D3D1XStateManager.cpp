#include "stdafx.h"
#include "D3D1XStateManager.h"
#include "CDebug.h"

CD3D1XStateManager::CD3D1XStateManager(CD3DRenderer* pRenderer): m_pDevice{pRenderer->getDevice()}, m_pDevContext{pRenderer->getContext()}
{
	m_blendDesc.RenderTarget[0].BlendEnable				= false;
	m_blendDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND_SRC_ALPHA;
	m_blendDesc.RenderTarget[0].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
	m_blendDesc.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	m_blendDesc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;
	m_blendDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;
	//m_blendDesc.AlphaToCoverageEnable = TRUE;
	if (FAILED(m_pDevice->CreateBlendState(&m_blendDesc, &m_pBlendState)))
		g_pDebug->printError("Failed to create blend state");

	// Depth test parameters
	m_depthStencilDesc.DepthEnable		= true;
	m_depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	m_depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	m_depthStencilDesc.StencilEnable	= true;
	m_depthStencilDesc.StencilReadMask	= 0xFF;
	m_depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	m_depthStencilDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	m_depthStencilDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.FrontFace.StencilFunc		= D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	m_depthStencilDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
	m_depthStencilDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	m_depthStencilDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
		g_pDebug->printError("Failed to create depth stencil state");

	m_rasterDesc.AntialiasedLineEnable = false;
	m_rasterDesc.CullMode = D3D11_CULL_BACK;
	m_rasterDesc.DepthBias = 0;
	m_rasterDesc.DepthBiasClamp = 0.0f;
	m_rasterDesc.DepthClipEnable = true;
	m_rasterDesc.FillMode = D3D11_FILL_SOLID;
	m_rasterDesc.FrontCounterClockwise = true;
	m_rasterDesc.MultisampleEnable = false;
	m_rasterDesc.ScissorEnable = false;
	m_rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	if (FAILED(m_pDevice->CreateRasterizerState(&m_rasterDesc, &m_pRasterState)))
		g_pDebug->printError("Failed to create rasterizer state");

	m_sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m_sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	m_sampDesc.MinLOD = 0;
	m_sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_sampDesc.MaxAnisotropy = 16;
	if (FAILED(m_pDevice->CreateSamplerState(&m_sampDesc, &m_pSamplerState)))
		g_pDebug->printError("Failed to create sampler state");
	globalSRSBuffer = {};
	globalSRSBuffer.fAlphaTestRef = 0.5f;
	globalSRSBuffer.uiAlphaTestType = 1;

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShaderRenderStateBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	if (FAILED(m_pDevice->CreateBuffer(&bd, nullptr, &m_pGlobalValuesBuffer)))
		g_pDebug->printError("Failed to create constant buffer");

	m_pDevContext->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
	m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	m_pDevContext->RSSetState(m_pRasterState);
	m_pDevContext->DSSetSamplers(0, 1, &m_pSamplerState);
	m_pDevContext->PSSetSamplers(0, 1, &m_pSamplerState);
}

CD3D1XStateManager::~CD3D1XStateManager()
{ 
	if (m_pGlobalValuesBuffer)	m_pGlobalValuesBuffer->Release();
	if (m_pBlendState)			m_pBlendState->Release();
	if (m_pDepthStencilState)	m_pDepthStencilState->Release();
	if (m_pRasterState)			m_pRasterState->Release();
	if (m_pSamplerState)		m_pSamplerState->Release();
}

void CD3D1XStateManager::SetCullMode(D3D11_CULL_MODE mode)
{
	if (m_rasterDesc.CullMode != mode) {
		m_rasterDesc.CullMode = mode;
		if (m_pRasterState) m_pRasterState->Release();

		if (FAILED(m_pDevice->CreateRasterizerState(&m_rasterDesc, &m_pRasterState)))
			g_pDebug->printError("Failed to create rasterizer state");

		m_pDevContext->RSSetState(m_pRasterState);
	}
}

void CD3D1XStateManager::SetDepthEnable(BOOL bEnable)
{
	if (m_depthStencilDesc.DepthEnable != bEnable) {
		m_depthStencilDesc.DepthEnable = bEnable;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();

		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetZWriteEnable(bool bEnable)
{
	if (m_depthStencilDesc.DepthWriteMask != (D3D11_DEPTH_WRITE_MASK)bEnable) {
		m_depthStencilDesc.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)bEnable;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetFillMode(D3D11_FILL_MODE mode)
{
	if (m_rasterDesc.FillMode != mode) {
		m_rasterDesc.FillMode = mode;
		if (m_pRasterState) m_pRasterState->Release();

		if (FAILED(m_pDevice->CreateRasterizerState(&m_rasterDesc, &m_pRasterState)))
			g_pDebug->printError("Failed to create rasterizer state");

		m_pDevContext->RSSetState(m_pRasterState);
	}
}

void CD3D1XStateManager::SetAlphaBlendEnable(BOOL bEnable)
{
	if (m_blendDesc.RenderTarget[0].BlendEnable != bEnable) {
		m_blendDesc.RenderTarget[0].BlendEnable = bEnable;
		if (m_pBlendState) m_pBlendState->Release();

		if (FAILED(m_pDevice->CreateBlendState(&m_blendDesc, &m_pBlendState)))
			g_pDebug->printError("Failed to create blend state");

		m_pDevContext->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
	}
}

void CD3D1XStateManager::SetAlphaTestEnable(bool bEnable)
{
	if (!bEnable)
		m_currentAlphaTestType = globalSRSBuffer.uiAlphaTestType;

	if (bEnable) {
		globalSRSBuffer.uiAlphaTestType = m_currentAlphaTestType;
		m_pDevContext->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &globalSRSBuffer, 0, 0);
		m_pDevContext->PSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->VSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
	}
	else
	{
		globalSRSBuffer.uiAlphaTestType = 0;
		m_pDevContext->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &globalSRSBuffer, 0, 0);
		m_pDevContext->PSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->VSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
	}
}

void CD3D1XStateManager::SetStencilEnable(BOOL bEnable)
{
	if (m_depthStencilDesc.StencilEnable != bEnable) {
		m_depthStencilDesc.StencilEnable = bEnable;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetStencilFail(RwStencilOperation op)
{
	D3D11_STENCIL_OP d3dOp = ConvertStencilOp(op);

	if (m_depthStencilDesc.FrontFace.StencilFailOp != d3dOp) {
		m_depthStencilDesc.FrontFace.StencilFailOp = d3dOp;
		m_depthStencilDesc.BackFace.StencilFailOp = d3dOp;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

D3D11_STENCIL_OP CD3D1XStateManager::ConvertStencilOp(RwStencilOperation op)
{
	switch (op)
	{
	case rwSTENCILOPERATIONZERO:
		return D3D11_STENCIL_OP_ZERO;
	case rwSTENCILOPERATIONREPLACE:
		return D3D11_STENCIL_OP_REPLACE;
	case rwSTENCILOPERATIONINCRSAT:
		return D3D11_STENCIL_OP_INCR_SAT;
	case rwSTENCILOPERATIONDECRSAT:
		return D3D11_STENCIL_OP_DECR_SAT;
	case rwSTENCILOPERATIONINVERT:
		return D3D11_STENCIL_OP_INVERT;
	case rwSTENCILOPERATIONINCR:
		return D3D11_STENCIL_OP_INCR;
	case rwSTENCILOPERATIONDECR:
		return D3D11_STENCIL_OP_DECR;
	default:
		return D3D11_STENCIL_OP_KEEP;
	}
}

D3D11_COMPARISON_FUNC CD3D1XStateManager::ConvertStencilFunc(RwStencilFunction func)
{
	switch (func)
	{
	case rwSTENCILFUNCTIONNEVER:
		return D3D11_COMPARISON_NEVER;
	case rwSTENCILFUNCTIONLESS:
		return D3D11_COMPARISON_LESS;
	case rwSTENCILFUNCTIONEQUAL:
		return D3D11_COMPARISON_EQUAL;
	case rwSTENCILFUNCTIONLESSEQUAL:
		return D3D11_COMPARISON_LESS_EQUAL;
	case rwSTENCILFUNCTIONGREATER:
		return D3D11_COMPARISON_GREATER;
	case rwSTENCILFUNCTIONNOTEQUAL:
		return D3D11_COMPARISON_NOT_EQUAL;
	case rwSTENCILFUNCTIONGREATEREQUAL:
		return D3D11_COMPARISON_GREATER_EQUAL;
	default:
		return D3D11_COMPARISON_ALWAYS;
	}
}

D3D11_TEXTURE_ADDRESS_MODE CD3D1XStateManager::ConvertTextureAddressMode(RwTextureAddressMode mode)
{
	switch (mode)
	{
	case rwTEXTUREADDRESSMIRROR:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case rwTEXTUREADDRESSCLAMP:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case rwTEXTUREADDRESSBORDER:
		return D3D11_TEXTURE_ADDRESS_BORDER;
	default:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

RwTextureAddressMode CD3D1XStateManager::ConvertTextureAddressMode(D3D11_TEXTURE_ADDRESS_MODE mode)
{
	switch (mode)
	{
	case D3D11_TEXTURE_ADDRESS_MIRROR:
		return rwTEXTUREADDRESSMIRROR;
	case D3D11_TEXTURE_ADDRESS_CLAMP:
		return rwTEXTUREADDRESSCLAMP;
	case D3D11_TEXTURE_ADDRESS_BORDER:
		return rwTEXTUREADDRESSBORDER;
	default:
		return rwTEXTUREADDRESSWRAP;
	}
}

D3D11_FILTER CD3D1XStateManager::ConvertTextureFilterMode(RwTextureFilterMode mode)
{
	switch (mode)
	{
	case rwFILTERNEAREST: case rwFILTERMIPNEAREST:
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	case rwFILTERLINEARMIPNEAREST:
		return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	default:
		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}
}

RwTextureFilterMode CD3D1XStateManager::ConvertTextureFilterMode(D3D11_FILTER mode)
{
	switch (mode)
	{
	case D3D11_FILTER_MIN_MAG_MIP_POINT: 
		return rwFILTERMIPNEAREST;
	case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT :
		return rwFILTERLINEARMIPNEAREST;
	default:
		return rwFILTERLINEARMIPLINEAR;
	}
}

void CD3D1XStateManager::SetStencilZFail(RwStencilOperation op)
{
	D3D11_STENCIL_OP d3dOp = ConvertStencilOp(op);

	if (m_depthStencilDesc.FrontFace.StencilDepthFailOp != d3dOp) {
		m_depthStencilDesc.FrontFace.StencilDepthFailOp = d3dOp;
		m_depthStencilDesc.BackFace.StencilDepthFailOp = d3dOp;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetStencilPass(RwStencilOperation op)
{
	D3D11_STENCIL_OP d3dOp = ConvertStencilOp(op);

	if (m_depthStencilDesc.FrontFace.StencilPassOp != d3dOp) {
		m_depthStencilDesc.FrontFace.StencilPassOp = d3dOp;
		m_depthStencilDesc.BackFace.StencilPassOp = d3dOp;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetStencilFunc(RwStencilFunction fn)
{
	D3D11_COMPARISON_FUNC d3dFn = ConvertStencilFunc(fn);

	if (m_depthStencilDesc.FrontFace.StencilFunc != d3dFn) {
		m_depthStencilDesc.FrontFace.StencilFunc = d3dFn;
		m_depthStencilDesc.BackFace.StencilFunc = d3dFn;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetStencilFuncRef(UINT ref)
{

	if (m_currentStencilRef != ref) {
		m_currentStencilRef = ref;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetStencilFuncMask(int mask)
{
	if (m_depthStencilDesc.StencilReadMask != mask) {
		m_depthStencilDesc.StencilReadMask = mask;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetStencilFuncWriteMask(int mask)
{
	if (m_depthStencilDesc.StencilWriteMask != mask) {
		m_depthStencilDesc.StencilWriteMask = mask;
		if (m_pDepthStencilState) m_pDepthStencilState->Release();
		if (FAILED(m_pDevice->CreateDepthStencilState(&m_depthStencilDesc, &m_pDepthStencilState)))
			g_pDebug->printError("Failed to create depth stencil state");

		m_pDevContext->OMSetDepthStencilState(m_pDepthStencilState, m_currentStencilRef);
	}
}

void CD3D1XStateManager::SetDestAlphaBlend(RwBlendFunction func)
{
	D3D11_BLEND blendFunc = ConvertBlendFunc(func);

	if (m_blendDesc.RenderTarget[0].DestBlend != blendFunc) {
		m_blendDesc.RenderTarget[0].DestBlend = blendFunc;
		if (m_pBlendState) m_pBlendState->Release();

		if (FAILED(m_pDevice->CreateBlendState(&m_blendDesc, &m_pBlendState)))
			g_pDebug->printError("Failed to create blend state");

		m_pDevContext->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
	}
}

void CD3D1XStateManager::SetSrcAlphaBlend(RwBlendFunction func)
{
	D3D11_BLEND blendFunc = ConvertBlendFunc(func);

	if (m_blendDesc.RenderTarget[0].SrcBlend != blendFunc) {
		m_blendDesc.RenderTarget[0].SrcBlend = blendFunc;
		if (m_pBlendState) m_pBlendState->Release();

		if (FAILED(m_pDevice->CreateBlendState(&m_blendDesc, &m_pBlendState)))
			g_pDebug->printError("Failed to create blend state");

		m_pDevContext->OMSetBlendState(m_pBlendState, 0, UINT_MAX);
	}
}

void CD3D1XStateManager::SetTextureAdressUV(RwTextureAddressMode mode)
{
	D3D11_TEXTURE_ADDRESS_MODE d3dMode = ConvertTextureAddressMode(mode);

	if (m_sampDesc.AddressU != d3dMode || m_sampDesc.AddressV != d3dMode) {
		m_sampDesc.AddressU = d3dMode;
		m_sampDesc.AddressV = d3dMode;
		if (m_pSamplerState) m_pSamplerState->Release();

		if (FAILED(m_pDevice->CreateSamplerState(&m_sampDesc, &m_pSamplerState)))
			g_pDebug->printError("Failed to create sampler state");

		m_pDevContext->DSSetSamplers(0, 1, &m_pSamplerState);
		m_pDevContext->PSSetSamplers(0, 1, &m_pSamplerState);
	}
}

void CD3D1XStateManager::SetTextureAdressU(RwTextureAddressMode mode)
{
	D3D11_TEXTURE_ADDRESS_MODE d3dMode = ConvertTextureAddressMode(mode);

	if (m_sampDesc.AddressU != d3dMode) {
		m_sampDesc.AddressU = d3dMode;
		if (m_pSamplerState) m_pSamplerState->Release();

		if (FAILED(m_pDevice->CreateSamplerState(&m_sampDesc, &m_pSamplerState)))
			g_pDebug->printError("Failed to create sampler state");

		m_pDevContext->DSSetSamplers(0, 1, &m_pSamplerState);
		m_pDevContext->PSSetSamplers(0, 1, &m_pSamplerState);
	}
}

void CD3D1XStateManager::SetTextureAdressV(RwTextureAddressMode mode)
{
	D3D11_TEXTURE_ADDRESS_MODE d3dMode = ConvertTextureAddressMode(mode);

	if (m_sampDesc.AddressV != d3dMode) {
		m_sampDesc.AddressV = d3dMode;
		if (m_pSamplerState) m_pSamplerState->Release();

		if (FAILED(m_pDevice->CreateSamplerState(&m_sampDesc, &m_pSamplerState)))
			g_pDebug->printError("Failed to create sampler state");

		m_pDevContext->DSSetSamplers(0, 1, &m_pSamplerState);
		m_pDevContext->PSSetSamplers(0, 1, &m_pSamplerState);
	}
}

void CD3D1XStateManager::SetTextureFilterMode(RwTextureFilterMode mode)
{
	D3D11_FILTER d3dMode = ConvertTextureFilterMode(mode);

	if (m_sampDesc.Filter != d3dMode) {
		m_sampDesc.Filter = d3dMode;
		if (m_pSamplerState) m_pSamplerState->Release();

		if (FAILED(m_pDevice->CreateSamplerState(&m_sampDesc, &m_pSamplerState)))
			g_pDebug->printError("Failed to create sampler state");

		m_pDevContext->DSSetSamplers(0, 1, &m_pSamplerState);
		m_pDevContext->PSSetSamplers(0, 1, &m_pSamplerState);
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

		if (m_pSamplerState) m_pSamplerState->Release();

		if (FAILED(m_pDevice->CreateSamplerState(&m_sampDesc, &m_pSamplerState)))
			g_pDebug->printError("Failed to create sampler state");

		m_pDevContext->DSSetSamplers(0, 1, &m_pSamplerState);
		m_pDevContext->PSSetSamplers(0, 1, &m_pSamplerState);
	}
}

void CD3D1XStateManager::SetTextureEnable(UINT enable)
{
	if (globalSRSBuffer.bHasTexture != enable) {
		globalSRSBuffer.bHasTexture = enable;
		RwV3d* campos= *(RwV3d**)(0xC88050);
		if(campos)
			globalSRSBuffer.vCamPos = { campos->x,campos->y,campos->z};
		m_pDevContext->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &globalSRSBuffer, 0, 0);
		m_pDevContext->PSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->VSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->HSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->DSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
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
	if (globalSRSBuffer.uiAlphaTestType != atType) {
		globalSRSBuffer.uiAlphaTestType = atType;
		m_currentAlphaTestType = atType;
		m_pDevContext->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &globalSRSBuffer, 0, 0);
		m_pDevContext->PSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->VSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
	}
}

void CD3D1XStateManager::SetAlphaTestRef(float ref)
{
	if (globalSRSBuffer.fAlphaTestRef != ref) {
		globalSRSBuffer.fAlphaTestRef = ref;
		m_pDevContext->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &globalSRSBuffer, 0, 0);
		m_pDevContext->PSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->VSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
	}
}

void CD3D1XStateManager::SetScreenSize(float w, float h)
{
	if (globalSRSBuffer.fScreenWidth != w || globalSRSBuffer.fScreenHeight != h) {
		globalSRSBuffer.fScreenWidth = w;
		globalSRSBuffer.fScreenHeight = h;
		m_pDevContext->UpdateSubresource(m_pGlobalValuesBuffer, 0, nullptr, &globalSRSBuffer, 0, 0);
		m_pDevContext->PSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
		m_pDevContext->VSSetConstantBuffers(1, 1, &m_pGlobalValuesBuffer);
	}
}

D3D11_BLEND CD3D1XStateManager::ConvertBlendFunc(RwBlendFunction func)
{
	D3D11_BLEND blendFunc = D3D11_BLEND_INV_SRC_ALPHA;
	switch (func)
	{
	case rwBLENDZERO:
		blendFunc = D3D11_BLEND_ZERO;
		break;
	case rwBLENDONE:
		blendFunc = D3D11_BLEND_ONE;
		break;
	case rwBLENDSRCCOLOR:
		blendFunc = D3D11_BLEND_SRC_COLOR;
		break;
	case rwBLENDINVSRCCOLOR:
		blendFunc = D3D11_BLEND_INV_SRC_COLOR;
		break;
	case rwBLENDSRCALPHA:
		blendFunc = D3D11_BLEND_SRC_ALPHA;
		break;
	case rwBLENDINVSRCALPHA:
		break;
	case rwBLENDDESTALPHA:
		blendFunc = D3D11_BLEND_DEST_ALPHA;
		break;
	case rwBLENDINVDESTALPHA:
		blendFunc = D3D11_BLEND_INV_DEST_ALPHA;
		break;
	case rwBLENDDESTCOLOR:
		blendFunc = D3D11_BLEND_DEST_COLOR;
		break;
	case rwBLENDINVDESTCOLOR:
		blendFunc = D3D11_BLEND_INV_DEST_COLOR;
		break;
	case rwBLENDSRCALPHASAT:
		blendFunc = D3D11_BLEND_SRC_ALPHA_SAT;
		break;
	default:
		break;
	}
	return blendFunc;
}

RwBlendFunction CD3D1XStateManager::ConvertBlendFunc(D3D11_BLEND func)
{
	RwBlendFunction blendFunc = rwBLENDSRCALPHA;
	switch (func)
	{
	case D3D11_BLEND_ZERO:
		blendFunc = rwBLENDZERO;
		break;
	case D3D11_BLEND_ONE:
		blendFunc = rwBLENDONE;
		break;
	case D3D11_BLEND_SRC_COLOR:
		blendFunc = rwBLENDSRCCOLOR;
		break;
	case D3D11_BLEND_INV_SRC_COLOR:
		blendFunc = rwBLENDINVSRCCOLOR;
		break;
	case D3D11_BLEND_SRC_ALPHA:
		blendFunc = rwBLENDSRCALPHA;
		break;
	case D3D11_BLEND_INV_SRC_ALPHA:
		break;
	case D3D11_BLEND_DEST_ALPHA :
		blendFunc = rwBLENDDESTALPHA;
		break;
	case D3D11_BLEND_INV_DEST_ALPHA:
		blendFunc = rwBLENDINVDESTALPHA;
		break;
	case D3D11_BLEND_DEST_COLOR:
		blendFunc = rwBLENDDESTCOLOR;
		break;
	case D3D11_BLEND_INV_DEST_COLOR:
		blendFunc = rwBLENDINVDESTCOLOR;
		break;
	case D3D11_BLEND_SRC_ALPHA_SAT:
		blendFunc = rwBLENDSRCALPHASAT;
		break;
	default:
		break;
	}
	return blendFunc;
}
