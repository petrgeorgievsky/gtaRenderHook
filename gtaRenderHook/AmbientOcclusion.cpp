// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "AmbientOcclusion.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "D3D1XShaderDefines.h"
#include "D3DRenderer.h"
#include <game_sa\CScene.h>
#include "FullscreenQuad.h"
AmbientOcclusionSettingsBlock gAmbientOcclusionSettings;
RwRaster*	 CAmbientOcclusion::m_pAORaser = nullptr;
CD3D1XShader* CAmbientOcclusion::m_pAmbientOcclusionPS = nullptr;
CD3D1XConstantBuffer<CBAmbientOcclusion>* CAmbientOcclusion::m_pAmbientOcclusionCB = nullptr;
bool	CAmbientOcclusion::m_bRequiresReloading = false;
void CAmbientOcclusion::Init()
{
	m_pAORaser = RwRasterCreate((RwInt32)(RsGlobal.maximumWidth*gAmbientOcclusionSettings.GetFloat("Scale")),
								(RwInt32)(RsGlobal.maximumHeight*gAmbientOcclusionSettings.GetFloat("Scale")), 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);
	gDebugSettings.DebugRenderTargetList.push_back(m_pAORaser);
	m_pAmbientOcclusionPS = new CD3D1XPixelShader("shaders/AmbientOcclusion.hlsl", "AmbientOcclusionPS", &gAmbientOcclusionSettings.m_ShaderDefineList);
	m_pAmbientOcclusionCB = new CD3D1XConstantBuffer<CBAmbientOcclusion>();
	gAmbientOcclusionSettings.m_aShaderPointers.push_back(m_pAmbientOcclusionPS);
}

void CAmbientOcclusion::Shutdown()
{
	if (m_pAORaser)
		RwRasterDestroy(m_pAORaser);
	delete m_pAmbientOcclusionPS;
	delete m_pAmbientOcclusionCB;
}

void CAmbientOcclusion::RenderAO(RwRaster * normalsDepth)
{
	if (m_bRequiresReloading) {
		m_pAmbientOcclusionCB->data.AORadius = gAmbientOcclusionSettings.GetFloat("Radius");
		m_pAmbientOcclusionCB->data.AOCurve = gAmbientOcclusionSettings.GetFloat("Curve");
		m_pAmbientOcclusionCB->data.AOIntensity = gAmbientOcclusionSettings.GetFloat("Intensity");
		m_pAmbientOcclusionCB->Update();
	}
	g_pRwCustomEngine->SetRenderTargets(&m_pAORaser, nullptr, 1);
	g_pStateMgr->SetConstantBufferPS(m_pAmbientOcclusionCB, 8);
	g_pStateMgr->FlushRenderTargets();
	g_pStateMgr->SetRaster(normalsDepth);
	m_pAmbientOcclusionPS->Set();
	CFullscreenQuad::Draw();
}

RwRaster* CAmbientOcclusion::GetAORaster()
{
	return m_pAORaser;
}

void CAmbientOcclusion::QueueTextureReload()
{
	CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;

	if (m_bRequiresReloading || dxEngine->m_bScreenSizeChanged) {
		m_pAORaser->width = (int)(RsGlobal.maximumWidth*gAmbientOcclusionSettings.GetFloat("Scale"));
		m_pAORaser->height = (int)(RsGlobal.maximumHeight*gAmbientOcclusionSettings.GetFloat("Scale"));
		dxEngine->m_pRastersToReload.push_back(m_pAORaser);
	}
}

void AmbientOcclusionSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	SettingsBlock::Load(doc);

	m_ShaderDefineList.AddDefine("AO_SAMPLE_COUNT", std::to_string(gAmbientOcclusionSettings.GetUInt("SampleCount")));
}

void TW_CALL ReloadAOShadersCallBack(void *value)
{
	gAmbientOcclusionSettings.m_bShaderReloadRequired = true;
	gAmbientOcclusionSettings.m_ShaderDefineList.Reset();
	gAmbientOcclusionSettings.m_ShaderDefineList.AddDefine("AO_SAMPLE_COUNT", std::to_string(gAmbientOcclusionSettings.GetUInt("SampleCount")));
}
void TW_CALL ReloadAOTexturesCallBack(void *value)
{
	CAmbientOcclusion::m_bRequiresReloading = true;
}
void AmbientOcclusionSettingsBlock::InitGUI(TwBar * mainBar)
{
	TwAddButton(mainBar, "Reload AO shaders", ReloadAOShadersCallBack, nullptr, "group=AmbientOcclusion");
	TwAddButton(mainBar, "Reload AO textures", ReloadAOTexturesCallBack, nullptr, "group=AmbientOcclusion");
}