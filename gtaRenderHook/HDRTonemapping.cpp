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
TonemapSettingsBlock gTonemapSettings{};

CHDRTonemapping::CHDRTonemapping():CPostProcessEffect("HDRTonemapping")
{
	m_pAdaptationRaster[0] = RwRasterCreate(1, 1, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);
	m_pAdaptationRaster[1] = RwRasterCreate(1, 1, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);
	m_pLogAvg = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "LogLuminance");
	m_pTonemap = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "Tonemap");
	m_pDownScale2x2_Lum = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "DownScale2x2_Lum");
	m_pDownScale3x3 = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "DownScale3x3");
	m_pDownScale3x3_BrightPass = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "DownScale3x3_BrightPass");
	m_pAdaptationPass = new CD3D1XPixelShader( "shaders/HDRTonemapping.hlsl", "AdaptationPass");
	
	int nSampleLen = 1;
	for (int i = 0; i < NUM_TONEMAP_TEXTURES; i++)
	{
		m_pToneMapRaster[i] = RwRasterCreate(nSampleLen, nSampleLen, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);

		nSampleLen *= 3;
	}
	m_pPostFXBuffer = new CD3D1XConstantBuffer<CBPostProcess>();
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



void CHDRTonemapping::Render(RwRaster * input)
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
	m_pDownScale2x2_Lum->ReSet();

	// Compute average scene luminance downsampling original raster by 3
	for (int i = NUM_TONEMAP_TEXTURES - 1; i > 0; i--)
	{
		g_pRwCustomEngine->SetRenderTargets(&m_pToneMapRaster[i - 1], nullptr, 1);
		g_pRwCustomEngine->RenderStateSet(RwRenderState::rwRENDERSTATETEXTURERASTER, reinterpret_cast<UINT>(m_pToneMapRaster[i]));
		g_pStateMgr->FlushRenderTargets();
		m_pDownScale3x3->Set();
		CFullscreenQuad::Draw();
		m_pDownScale3x3->ReSet();
	}

	// Adapt luminance using previous frame luminance
	g_pRwCustomEngine->SetRenderTargets(&m_pAdaptationRaster[m_nCurrentAdaptationRaster], nullptr, 1);
	g_pRwCustomEngine->RenderStateSet(RwRenderState::rwRENDERSTATETEXTURERASTER, reinterpret_cast<UINT>(m_pAdaptationRaster[1 - m_nCurrentAdaptationRaster]));
	g_pStateMgr->FlushRenderTargets();

	g_pStateMgr->SetRaster(m_pToneMapRaster[0], 1);
	m_pAdaptationPass->Set();
	CFullscreenQuad::Draw();
	m_pAdaptationPass->ReSet();

	m_nCurrentAdaptationRaster = 1 - m_nCurrentAdaptationRaster;

	g_pRwCustomEngine->SetRenderTargets(&Scene.m_pRwCamera->frameBuffer, Scene.m_pRwCamera->zBuffer, 1);
	g_pRwCustomEngine->RenderStateSet(RwRenderState::rwRENDERSTATETEXTURERASTER, reinterpret_cast<UINT>(input));
	g_pStateMgr->FlushRenderTargets();

	g_pStateMgr->SetRaster(m_pAdaptationRaster[1 - m_nCurrentAdaptationRaster], 1);
	m_pTonemap->Set();
	CFullscreenQuad::Draw();
	m_pTonemap->ReSet();
	g_pStateMgr->SetRaster(nullptr, 0);
	g_pStateMgr->SetRaster(nullptr, 1);
}

tinyxml2::XMLElement * TonemapSettingsBlock::Save(tinyxml2::XMLDocument * doc)
{
	auto node = doc->NewElement(m_sName.c_str());
	node->SetAttribute("LumWhiteDay", gTonemapSettings.LumWhiteDay);
	node->SetAttribute("LumWhiteNight", gTonemapSettings.LumWhiteNight);
	node->SetAttribute("MiddleGrayDay", gTonemapSettings.MiddleGrayDay);
	node->SetAttribute("MiddleGrayNight", gTonemapSettings.MiddleGrayNight);
	return node;
}

void TonemapSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	auto node = doc.FirstChildElement(m_sName.c_str());
	gTonemapSettings.LumWhiteDay = node->FloatAttribute("LumWhiteDay", 1.25f);
	gTonemapSettings.LumWhiteNight = node->FloatAttribute("LumWhiteDay", 1.0f);
	gTonemapSettings.MiddleGrayDay = node->FloatAttribute("MiddleGrayDay", 0.55f);
	gTonemapSettings.MiddleGrayNight = node->FloatAttribute("MiddleGrayNight", 0.25f);
}

void TonemapSettingsBlock::Reset()
{
	MiddleGrayDay = 0.55f;
	LumWhiteDay = 1.25f;
	MiddleGrayNight = 0.25f;
	LumWhiteNight = 1.0f;
}

void TonemapSettingsBlock::InitGUI(TwBar * bar)
{
	TwAddVarRW(bar, "LumWhite Day", TwType::TW_TYPE_FLOAT, &LumWhiteDay, " min=0 max=10 step=0.005 help='meh' group=Tonemap");
	TwAddVarRW(bar, "LumWhite Night", TwType::TW_TYPE_FLOAT, &LumWhiteNight, " min=0 max=10 step=0.005 help='meh' group=Tonemap");
	TwAddVarRW(bar, "MiddleGray Day", TwType::TW_TYPE_FLOAT, &MiddleGrayDay, " min=0 max=10 step=0.005 help='meh' group=Tonemap");
	TwAddVarRW(bar, "MiddleGray Night", TwType::TW_TYPE_FLOAT, &MiddleGrayNight, " min=0 max=10 step=0.005 help='meh' group=Tonemap");
}

float TonemapSettingsBlock::GetCurrentLumWhite()
{
	float dnBalance = *(float*)(0x8D12C0);
	return (1 - dnBalance) * LumWhiteDay + dnBalance * LumWhiteNight;
}

float TonemapSettingsBlock::GetCurrentMiddleGray()
{
	float dnBalance = *(float*)(0x8D12C0);
	return (1 - dnBalance) * MiddleGrayDay + dnBalance * MiddleGrayNight;
}
