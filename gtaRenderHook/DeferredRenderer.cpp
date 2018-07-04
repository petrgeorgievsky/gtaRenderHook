// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "DeferredRenderer.h"
#include "RwRenderEngine.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "SAIdleHook.h"
#include "D3D1XShader.h"
#include "D3D1XTexture.h"
#include "ShadowRenderer.h"
#include "D3D1XIm2DPipeline.h"
#include "FullscreenQuad.h"
#include "HDRTonemapping.h"
#include "D3D1XStateManager.h"
#include "LightManager.h"
#include "CubemapReflectionRenderer.h"
#include "D3D1XShaderDefines.h"
#include "VolumetricLighting.h"
#include "AmbientOcclusion.h"
#include <game_sa\CScene.h>
#include <game_sa\CGame.h>
#include "TemporalAA.h"
/// voxel stuff
struct CB_Raycasting {
	RwMatrix m_QuadToVoxel;
	RwMatrix m_VoxelToScreen;
};
DeferredSettingsBlock gDeferredSettings;
///
extern UINT m_uiDeferredStage = 0;
CDeferredRenderer::CDeferredRenderer()
{
	m_aDeferredRasters[0] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE);
	m_aDeferredRasters[1] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555);
	m_aDeferredRasters[2] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE);
	m_aDeferredRasters[3] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555);
	m_pReflectionRaster = RwRasterCreate((int)(RsGlobal.maximumWidth*gDeferredSettings.GetFloat("SSRScale")),(int)(RsGlobal.maximumHeight*gDeferredSettings.GetFloat("SSRScale")), 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555);
	m_pLightingRaster = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE| rwRASTERFORMAT1555);
	// We use 2 final rasters to have one for rendering effects that use full lighted image texture for example SSR
	m_pFinalRasters[0] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555);
	m_pFinalRasters[1] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555);
	m_pFinalRasters[2] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555);
	m_pFinalRasters[3] = RwRasterCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555);
	
	m_pSunLightingPS	= new CD3D1XPixelShader("shaders/Deferred.hlsl", "SunLightingPS", &gDeferredSettings.m_ShaderDefineList);
	m_pPointLightingPS	= new CD3D1XPixelShader("shaders/Deferred.hlsl", "PointLightingPS", &gDeferredSettings.m_ShaderDefineList);
	m_pFinalPassPS		= new CD3D1XPixelShader("shaders/Deferred.hlsl", "FinalPassPS", &gDeferredSettings.m_ShaderDefineList);
	m_pBlitPassPS		= new CD3D1XPixelShader("shaders/Deferred.hlsl", "BlitPS");
	m_pAtmospherePassPS = new CD3D1XPixelShader("shaders/AtmosphericScattering.hlsl", "AtmosphericScatteringPS", &gDeferredSettings.m_ShaderDefineList);
	m_pReflectionPassPS = new CD3D1XPixelShader("shaders/ScreenSpaceReflections.hlsl", "ReflectionPassPS", &gDeferredSettings.m_ShaderDefineList);

	gDeferredSettings.m_aShaderPointers.push_back(m_pSunLightingPS);
	gDeferredSettings.m_aShaderPointers.push_back(m_pPointLightingPS);
	gDeferredSettings.m_aShaderPointers.push_back(m_pFinalPassPS);
	gDeferredSettings.m_aShaderPointers.push_back(m_pAtmospherePassPS);
	gDeferredSettings.m_aShaderPointers.push_back(m_pReflectionPassPS);
	for (int i = 0; i < 4; i++)
	{
		gDebugSettings.DebugRenderTargetList.push_back(m_aDeferredRasters[i]);
	}
	gDebugSettings.DebugRenderTargetList.push_back(m_pLightingRaster);
	gDebugSettings.DebugRenderTargetList.push_back(m_pReflectionRaster);

	m_pShadowRenderer	= new CShadowRenderer();
	m_pReflRenderer = new CCubemapReflectionRenderer(gDeferredSettings.GetUInt("CubemapSize"));
	m_pTonemapping		= new CHDRTonemapping();
	m_pDeferredBuffer = new CD3D1XConstantBuffer<CBDeferredRendering>();
	m_pDeferredBuffer->SetDebugName("DeferredCB");
}


CDeferredRenderer::~CDeferredRenderer()
{
	delete m_pReflRenderer;
	delete m_pDeferredBuffer;
	delete m_pTonemapping;
	delete m_pShadowRenderer;
	delete m_pReflectionPassPS;
	delete m_pAtmospherePassPS;
	delete m_pSunLightingPS;
	delete m_pPointLightingPS;
	delete m_pFinalPassPS;
	delete m_pBlitPassPS;
	RwRasterDestroy(m_pReflectionRaster);
	RwRasterDestroy(m_pFinalRasters[3]);
	RwRasterDestroy(m_pFinalRasters[2]);
	RwRasterDestroy(m_pFinalRasters[1]);
	RwRasterDestroy(m_pFinalRasters[0]);
	RwRasterDestroy(m_pLightingRaster);
	RwRasterDestroy(m_aDeferredRasters[3]);
	RwRasterDestroy(m_aDeferredRasters[2]);
	RwRasterDestroy(m_aDeferredRasters[1]);
	RwRasterDestroy(m_aDeferredRasters[0]);
}

void CDeferredRenderer::RenderToGBuffer(void(*renderCB)())
{
	g_pDebug->printMsg("Rendering event GBuffer: start",1);
	CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;

	CD3DRenderer*		renderer	= dxEngine->getRenderer();
	
	m_uiDeferredStage = 1;

	renderer->BeginDebugEvent(L"GBuffer pass");
	g_pRwCustomEngine->SetRenderTargets(m_aDeferredRasters, Scene.m_pRwCamera->zBuffer, 4);
	renderCB();
	g_pRwCustomEngine->SetRenderTargets(&Scene.m_pRwCamera->frameBuffer, Scene.m_pRwCamera->zBuffer, 1);
	
	renderer->EndDebugEvent();
	g_pDebug->printMsg("Rendering event GBuffer: end", 1);
}

void CDeferredRenderer::RenderOutput()
{

	//g_pDebug->printMsg("Start of deffered output.");
	m_pDeferredBuffer->data.SSRMaxIterations = gDeferredSettings.GetUInt("SSRMaxIterations");
	m_pDeferredBuffer->data.SSRStep = gDeferredSettings.GetFloat("SSRStep");
	m_pDeferredBuffer->data.MaxShadowBlur = gDeferredSettings.GetFloat("MaxShadowBlur");
	m_pDeferredBuffer->data.MinShadowBlur = gDeferredSettings.GetFloat("MinShadowBlur");
	m_pDeferredBuffer->Update();
	g_pStateMgr->SetConstantBufferPS(m_pDeferredBuffer, 6);
	CAmbientOcclusion::RenderAO(m_aDeferredRasters[1]);
	g_pRwCustomEngine->SetRenderTargets(&m_pLightingRaster, nullptr, 1);
	g_pStateMgr->FlushRenderTargets();
	// Set deferred textures
	g_pStateMgr->SetRaster(m_aDeferredRasters[0]);
	for (auto i = 1; i < 4; i++)
		g_pStateMgr->SetRaster(m_aDeferredRasters[i], i);
	
	m_pShadowRenderer->SetShadowBuffer();

	// Render sun directional light if required
	//if (g_shaderRenderStateBuffer.vSunDir.w > 0&&CGame::currArea==0) 
	//{
		m_pSunLightingPS->Set();
		g_pDebug->printMsg("Sun light rendering.", 1);
		CFullscreenQuad::Draw();
	//}

	// Render point and spot lights
	if (CLightManager::m_nLightCount > 0) {
		CLightManager::SortByDistance(TheCamera.GetPosition().ToRwV3d());
		CLightManager::Update();
		g_pStateMgr->SetStructuredBufferPS(CLightManager::GetBuffer(), 5);
		g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, TRUE);
		g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESRCBLEND, rwBLENDONE);
		g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEDESTBLEND, rwBLENDONE);
		g_pDebug->printMsg("Point light rendering.", 1);
		m_pPointLightingPS->Set();
		CFullscreenQuad::Draw();
	}
	
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, FALSE);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESRCBLEND, rwBLENDSRCALPHA);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEDESTBLEND, rwBLENDINVSRCALPHA);

	// Render reflection pass
	g_pRwCustomEngine->SetRenderTargets(&m_pReflectionRaster, nullptr, 1);
	g_pStateMgr->FlushRenderTargets();
	g_pStateMgr->SetRaster(m_pFinalRasters[1 - m_uiCurrentFinalRaster], 3);
	m_pReflRenderer->SetCubemap();
	m_pReflectionPassPS->Set();
	CFullscreenQuad::Draw();

	// Compose lighting with color and reflections.
	g_pRwCustomEngine->SetRenderTargets(&m_pFinalRasters[m_uiCurrentFinalRaster], nullptr, 1);
	g_pStateMgr->FlushRenderTargets();
	g_pStateMgr->SetRaster(m_pLightingRaster, 4);
	g_pStateMgr->SetRaster(m_pFinalRasters[1 - m_uiCurrentFinalRaster], 5);
	g_pStateMgr->SetRaster(m_pReflectionRaster, 6);
	g_pStateMgr->SetRaster(CAmbientOcclusion::GetAORaster(), 7);
	
	m_pFinalPassPS->Set(); 
	CFullscreenQuad::Draw();

	for (auto i = 0; i < 8; i++)
		g_pStateMgr->SetRaster(nullptr, i);

	// Atmospheric scattering
	g_pRwCustomEngine->SetRenderTargets(&m_pFinalRasters[2], nullptr, 1);
	g_pStateMgr->FlushRenderTargets();
	g_pStateMgr->SetRaster(m_pFinalRasters[m_uiCurrentFinalRaster]);
	g_pStateMgr->SetRaster(m_aDeferredRasters[1], 1);
	//m_pShadowRenderer->SetShadowBuffer();
	m_pAtmospherePassPS->Set();
	CFullscreenQuad::Draw();
	
	for (auto i = 0; i < 7; i++)
		g_pStateMgr->SetRaster(nullptr, i);

	// Restore texture 
	g_pRwCustomEngine->SetRenderTargets(&m_pFinalRasters[m_uiCurrentFinalRaster], Scene.m_pRwCamera->zBuffer, 1);
	g_pStateMgr->FlushRenderTargets();

	g_pStateMgr->SetRaster(m_pFinalRasters[2], 5);
	m_pBlitPassPS->Set();
	CFullscreenQuad::Draw();

	for (auto i = 0; i < 7; i++)
		g_pStateMgr->SetRaster(nullptr, i);
}

void CDeferredRenderer::RenderToCubemap(void(*renderCB)())
{
	m_pReflRenderer->RenderToCubemap(renderCB);
}

void CDeferredRenderer::RenderTonemappedOutput()
{
	CVolumetricLighting::RenderVolumetricEffects(m_aDeferredRasters[1],
		m_pShadowRenderer->m_pShadowCamera->zBuffer, m_pFinalRasters[m_uiCurrentFinalRaster], m_pFinalRasters[3]);
	CTemporalAA::Render(m_pFinalRasters[3], m_pFinalRasters[2], m_aDeferredRasters[1], m_aDeferredRasters[2]);
	m_pTonemapping->Render(m_pFinalRasters[2], Scene.m_pRwCamera->frameBuffer, Scene.m_pRwCamera->zBuffer);
	m_uiCurrentFinalRaster = 1-m_uiCurrentFinalRaster;
	g_pStateMgr->SetRaster(nullptr);
	for (auto i = 1; i < 7; i++)
		g_pStateMgr->SetRaster(nullptr, i);
}

void CDeferredRenderer::SetNormalDepthRaster()
{
	g_pStateMgr->SetRaster(m_aDeferredRasters[1], 1);
}

void CDeferredRenderer::SetPreviousFinalRaster()
{
	g_pStateMgr->SetRaster(m_pFinalRasters[1 - m_uiCurrentFinalRaster], 2);
}

void CDeferredRenderer::SetPreviousNonTonemappedFinalRaster()
{
	g_pStateMgr->SetRaster(m_pFinalRasters[2], 2);
}

void CDeferredRenderer::QueueTextureReload()
{
	CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;

	m_pShadowRenderer->QueueTextureReload();
	
	if (m_bRequiresReloading || dxEngine->m_bScreenSizeChanged) {
		m_pReflectionRaster->width = (int)(RsGlobal.maximumWidth*gDeferredSettings.GetFloat("SSRScale"));
		m_pReflectionRaster->height = (int)(RsGlobal.maximumHeight*gDeferredSettings.GetFloat("SSRScale"));
		dxEngine->m_pRastersToReload.push_back(m_pReflectionRaster);
	}

	if (dxEngine->m_bScreenSizeChanged) {
		for (int i = 0; i < 4; i++) {
			m_aDeferredRasters[i]->width = RsGlobal.maximumWidth;
			m_aDeferredRasters[i]->height = RsGlobal.maximumHeight;

			dxEngine->m_pRastersToReload.push_back(m_aDeferredRasters[i]);
		}
		m_pLightingRaster->width = RsGlobal.maximumWidth;
		m_pLightingRaster->height = RsGlobal.maximumHeight;
		dxEngine->m_pRastersToReload.push_back(m_pLightingRaster);

		
		for (int i = 0; i < 4; i++) {
			m_pFinalRasters[i]->width = RsGlobal.maximumWidth;
			m_pFinalRasters[i]->height = RsGlobal.maximumHeight;
			dxEngine->m_pRastersToReload.push_back(m_pFinalRasters[i]);
		}
	}
	
}

void DeferredSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	SettingsBlock::Load(doc);

	gDeferredSettings.m_ShaderDefineList.AddDefine("SSR_SAMPLE_COUNT", to_string(GetUInt("SSRMaxIterations")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("SAMPLE_SHADOWS", to_string((int)GetToggleField("SampleShadows")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("BLUR_SHADOWS", to_string((int)GetToggleField("BlurShadows")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("USE_PCS_SHADOWS", to_string((int)GetToggleField("UsePCSS")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("SHADOW_BLUR_KERNEL", to_string(GetUInt("ShadowsBlurKernelSize")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("USE_SSR", to_string((int)GetToggleField("UseSSR")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("SAMPLE_CUBEMAP", to_string((int)GetToggleField("SampleCubemap")));
}

void TW_CALL ReloadDeferredShadersCallBack(void *value)
{
	gDeferredSettings.m_bShaderReloadRequired = true;
	gDeferredSettings.m_ShaderDefineList.Reset();
	gDeferredSettings.m_ShaderDefineList.AddDefine("SSR_SAMPLE_COUNT", to_string(gDeferredSettings.GetUInt("SSRMaxIterations")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("SAMPLE_SHADOWS", to_string((int)gDeferredSettings.GetToggleField("SampleShadows")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("BLUR_SHADOWS", to_string((int)gDeferredSettings.GetToggleField("BlurShadows")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("USE_PCS_SHADOWS", to_string((int)gDeferredSettings.GetToggleField("UsePCSS")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("SHADOW_BLUR_KERNEL", to_string(gDeferredSettings.GetUInt("ShadowsBlurKernelSize")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("USE_SSR", to_string((int)gDeferredSettings.GetToggleField("UseSSR")));
	gDeferredSettings.m_ShaderDefineList.AddDefine("SAMPLE_CUBEMAP", to_string((int)gDeferredSettings.GetToggleField("SampleCubemap")));
}
void TW_CALL ReloadDeferredTexturesCallBack(void *value)
{
	g_pDeferredRenderer->m_bRequiresReloading = true;
}
void DeferredSettingsBlock::InitGUI(TwBar * bar)
{
	SettingsBlock::InitGUI(bar);
	TwAddButton(bar, "Reload shaders", ReloadDeferredShadersCallBack, nullptr, "group=Deferred");
	TwAddButton(bar, "Reload textures", ReloadDeferredTexturesCallBack, nullptr, "group=Deferred");
}
