// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "HDRTonemapping.h"
#include "D3D1XShader.h"
#include "RwRenderEngine.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3D1XTexture.h"
#include "FullscreenQuad.h"
#include "D3D1XStateManager.h"
#include "D3D1XBuffer.h"
#include <game_sa\CScene.h>
#include "D3D1XShaderDefines.h"
TonemapSettingsBlock gTonemapSettings{};

CHDRTonemapping::CHDRTonemapping():CPostProcessEffect("HDRTonemapping")
{
	m_pAdaptationRaster[0] = RwRasterCreate(1, 1, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);
	m_pAdaptationRaster[1] = RwRasterCreate(1, 1, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);
	m_pLogAvg = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "LogLuminancePS", gTonemapSettings.m_pShaderDefineList);
	m_pTonemap = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "TonemapPS", gTonemapSettings.m_pShaderDefineList);
	m_pDownScale2x2_Lum = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "DownScale2x2_LumPS", gTonemapSettings.m_pShaderDefineList);
	m_pDownScale3x3 = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "DownScale3x3PS", gTonemapSettings.m_pShaderDefineList);
	m_pDownScale3x3_BrightPass = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "DownScale3x3_BrightPassPS", gTonemapSettings.m_pShaderDefineList);
	m_pAdaptationPass = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "AdaptationPassPS", gTonemapSettings.m_pShaderDefineList);
	
	gTonemapSettings.m_aShaderPointers.push_back(m_pLogAvg);
	gTonemapSettings.m_aShaderPointers.push_back(m_pTonemap);
	gTonemapSettings.m_aShaderPointers.push_back(m_pDownScale2x2_Lum);
	gTonemapSettings.m_aShaderPointers.push_back(m_pDownScale3x3);
	gTonemapSettings.m_aShaderPointers.push_back(m_pDownScale3x3_BrightPass);
	gTonemapSettings.m_aShaderPointers.push_back(m_pAdaptationPass);

	int nSampleLen = 1;
	for (int i = 0; i < NUM_TONEMAP_TEXTURES; i++)
	{
		m_pToneMapRaster[i] = RwRasterCreate(nSampleLen, nSampleLen, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);

		nSampleLen *= 3;
	}
	m_pPostFXBuffer = new CD3D1XConstantBuffer<CBPostProcess>();
	m_pPostFXBuffer->SetDebugName("PostProcessCB");
}



CHDRTonemapping::~CHDRTonemapping()
{
	delete m_pPostFXBuffer;
	for (int i = 0; i < NUM_TONEMAP_TEXTURES; i++)
		RwRasterDestroy(m_pToneMapRaster[i]);
	
	RwRasterDestroy(m_pAdaptationRaster[0]);
	RwRasterDestroy(m_pAdaptationRaster[1]);
	delete m_pLogAvg;
	delete m_pTonemap;
	delete m_pDownScale2x2_Lum;
	delete m_pDownScale3x3;
	delete m_pDownScale3x3_BrightPass;
	delete m_pAdaptationPass;
}



void CHDRTonemapping::Render(RwRaster * input, RwRaster* output)
{
	m_pPostFXBuffer->data.LumWhite = gTonemapSettings.GetCurrentLumWhite();
	m_pPostFXBuffer->data.MiddleGray = gTonemapSettings.GetCurrentMiddleGray();
	m_pPostFXBuffer->Update();
	g_pStateMgr->SetConstantBufferPS(m_pPostFXBuffer,5);

	// Set last tonemap raster to render luminance.
	g_pRwCustomEngine->SetRenderTargets(&m_pToneMapRaster[NUM_TONEMAP_TEXTURES-1], nullptr, 1);
	g_pRwCustomEngine->RenderStateSet(RwRenderState::rwRENDERSTATETEXTURERASTER, reinterpret_cast<UINT>(input));
	g_pStateMgr->FlushRenderTargets();
	m_pDownScale2x2_Lum->Set();
	CFullscreenQuad::Draw();

	// Compute average scene luminance downsampling original raster by 3
	for (int i = NUM_TONEMAP_TEXTURES - 1; i > 0; i--)
	{
		g_pRwCustomEngine->SetRenderTargets(&m_pToneMapRaster[i - 1], nullptr, 1);
		g_pRwCustomEngine->RenderStateSet(RwRenderState::rwRENDERSTATETEXTURERASTER, reinterpret_cast<UINT>(m_pToneMapRaster[i]));
		g_pStateMgr->FlushRenderTargets();
		m_pDownScale3x3->Set();
		CFullscreenQuad::Draw();
	}

	// Adapt luminance using previous frame luminance
	g_pRwCustomEngine->SetRenderTargets(&m_pAdaptationRaster[m_nCurrentAdaptationRaster], nullptr, 1);
	g_pRwCustomEngine->RenderStateSet(RwRenderState::rwRENDERSTATETEXTURERASTER, reinterpret_cast<UINT>(m_pAdaptationRaster[1 - m_nCurrentAdaptationRaster]));
	g_pStateMgr->FlushRenderTargets();

	g_pStateMgr->SetRaster(m_pToneMapRaster[0], 1);
	m_pAdaptationPass->Set();
	CFullscreenQuad::Draw();

	m_nCurrentAdaptationRaster = 1 - m_nCurrentAdaptationRaster;

	g_pRwCustomEngine->SetRenderTargets(&output, nullptr, 1);
	g_pRwCustomEngine->RenderStateSet(RwRenderState::rwRENDERSTATETEXTURERASTER, reinterpret_cast<UINT>(input));
	g_pStateMgr->FlushRenderTargets();

	g_pStateMgr->SetRaster(m_pAdaptationRaster[1 - m_nCurrentAdaptationRaster], 1);
	m_pTonemap->Set();
	CFullscreenQuad::Draw();
	g_pStateMgr->SetRaster(nullptr, 0);
	g_pStateMgr->SetRaster(nullptr, 1);
}

tinyxml2::XMLElement * TonemapSettingsBlock::Save(tinyxml2::XMLDocument * doc)
{
	return SettingsBlock::Save(doc);
}

void TonemapSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	SettingsBlock::Load(doc);
	gTonemapSettings.m_pShaderDefineList = new CD3D1XShaderDefineList();
	gTonemapSettings.m_pShaderDefineList->AddDefine("USE_GTA_CC", to_string((int)GetToggleField("UseGTAColorCorrection")));
}

void TW_CALL ReloadTonemapShadersCallBack(void *value)
{
	gTonemapSettings.m_bShaderReloadRequired = true;
	gTonemapSettings.m_pShaderDefineList->Reset();
	gTonemapSettings.m_pShaderDefineList->AddDefine("USE_GTA_CC", to_string((int)gTonemapSettings.GetToggleField("UseGTAColorCorrection")));
}
void TonemapSettingsBlock::InitGUI(TwBar * bar)
{
	SettingsBlock::InitGUI(bar);
	TwAddButton(bar, "Reload tonemap shaders", ReloadTonemapShadersCallBack, nullptr, "group=Tonemap");
}

float TonemapSettingsBlock::GetCurrentLumWhite()
{
	float dnBalance = *(float*)(0x8D12C0);
	return (1 - dnBalance) * GetFloat("LumWhiteDay") + dnBalance * GetFloat("LumWhiteNight");
}

float TonemapSettingsBlock::GetCurrentMiddleGray()
{
	float dnBalance = *(float*)(0x8D12C0);
	return (1 - dnBalance) * GetFloat("MiddleGrayDay") + dnBalance * GetFloat("MiddleGrayNight");
}
