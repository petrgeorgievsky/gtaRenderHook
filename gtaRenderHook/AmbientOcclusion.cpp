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
	m_pAORaser = RwRasterCreate((RwInt32)(RsGlobal.maximumWidth*gAmbientOcclusionSettings.AOScale), 
								(RwInt32)(RsGlobal.maximumHeight*gAmbientOcclusionSettings.AOScale), 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16);
	gDebugSettings.DebugRenderTargetList.push_back(m_pAORaser);
	m_pAmbientOcclusionPS = new CD3D1XPixelShader("shaders/AmbientOcclusion.hlsl", "AmbientOcclusionPS",gAmbientOcclusionSettings.m_pShaderDefineList);
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
	m_pAmbientOcclusionCB->data.AORadius = gAmbientOcclusionSettings.AORadius;
	m_pAmbientOcclusionCB->data.AOCurve = gAmbientOcclusionSettings.AOCurve;
	m_pAmbientOcclusionCB->data.AOIntensity = gAmbientOcclusionSettings.AOIntesity;
	m_pAmbientOcclusionCB->Update();
	g_pRwCustomEngine->SetRenderTargets(&m_pAORaser, Scene.m_pRwCamera->zBuffer, 1);
	g_pStateMgr->SetConstantBufferPS(m_pAmbientOcclusionCB, 8);
	g_pStateMgr->FlushRenderTargets();
	g_pStateMgr->SetRaster(normalsDepth, 0);
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
		m_pAORaser->width = (int)(RsGlobal.maximumWidth*gAmbientOcclusionSettings.AOScale);
		m_pAORaser->height = (int)(RsGlobal.maximumHeight*gAmbientOcclusionSettings.AOScale);
		dxEngine->m_pRastersToReload.push_back(m_pAORaser);
	}
}

tinyxml2::XMLElement * AmbientOcclusionSettingsBlock::Save(tinyxml2::XMLDocument * doc)
{
	auto settingsNode = doc->NewElement(m_sName.c_str());
	settingsNode->SetAttribute("Radius", AORadius);
	settingsNode->SetAttribute("Curve", AOCurve);
	settingsNode->SetAttribute("Intesity", AOIntesity);
	settingsNode->SetAttribute("Samples", AOSamples);
	settingsNode->SetAttribute("Scale", AOScale);

	return settingsNode;
}

void AmbientOcclusionSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	auto settingsNode = doc.FirstChildElement(m_sName.c_str());
	if (settingsNode == nullptr) {
		Reset();
		return;
	}
	AOSamples = settingsNode->UnsignedAttribute("Samples", 8);
	AORadius = settingsNode->FloatAttribute("Radius", 0.5f);
	AOCurve = settingsNode->FloatAttribute("Curve", 1.5f);
	AOIntesity = settingsNode->FloatAttribute("Intesity", 1.0f);
	AOScale = settingsNode->FloatAttribute("Scale", 0.75f);

	m_pShaderDefineList = new CD3D1XShaderDefineList();
	m_pShaderDefineList->AddDefine("AO_SAMPLE_COUNT", std::to_string(AOSamples));
}

void AmbientOcclusionSettingsBlock::Reset()
{
	AORadius = 0.5f;
	AOCurve = 1.5f;
	AOIntesity = 1.0f;
	AOSamples = 8;
	AOScale = 0.75f;
}
void TW_CALL ReloadAOShadersCallBack(void *value)
{
	gAmbientOcclusionSettings.m_bShaderReloadRequired = true;
	if(gAmbientOcclusionSettings.m_pShaderDefineList==nullptr)
		gAmbientOcclusionSettings.m_pShaderDefineList = new CD3D1XShaderDefineList();
	gAmbientOcclusionSettings.m_pShaderDefineList->Reset();
	gAmbientOcclusionSettings.m_pShaderDefineList->AddDefine("AO_SAMPLE_COUNT", std::to_string(gAmbientOcclusionSettings.AOSamples));
}
void TW_CALL ReloadAOTexturesCallBack(void *value)
{
	CAmbientOcclusion::m_bRequiresReloading = true;
}
void AmbientOcclusionSettingsBlock::InitGUI(TwBar * mainBar)
{
	TwAddVarRW(mainBar, "Intensity", TwType::TW_TYPE_FLOAT, &AOIntesity, "min=0.001 max=5.0 step=0.001 group=AmbientOcclusion");
	TwAddVarRW(mainBar, "Radius", TwType::TW_TYPE_FLOAT, &AORadius, "min=0.005 max=5.0 step=0.001 group=AmbientOcclusion");
	TwAddVarRW(mainBar, "Scale", TwType::TW_TYPE_FLOAT, &AOScale, "min=0.1 max=1.0 step=0.05 group=AmbientOcclusion");
	TwAddVarRW(mainBar, "Curve", TwType::TW_TYPE_FLOAT, &AOCurve, "min=0.01 max=10 step=0.005 group=AmbientOcclusion");
	TwAddVarRW(mainBar, "Sample count", TwType::TW_TYPE_UINT32, &AOSamples, "min=4 max=128 step=1 group=AmbientOcclusion");
	TwAddButton(mainBar, "Reload AO shaders", ReloadAOShadersCallBack, nullptr, "group=AmbientOcclusion");
	TwAddButton(mainBar, "Reload AO textures", ReloadAOTexturesCallBack, nullptr, "group=AmbientOcclusion");
}