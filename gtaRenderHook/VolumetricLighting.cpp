#include "stdafx.h"
#include "VolumetricLighting.h"
#include "D3D1XShader.h"
#include "D3D1XShaderDefines.h"
#include "RwRenderEngine.h"
#include "D3D1XStateManager.h"
#include "FullscreenQuad.h"
#include "D3DRenderer.h"
#include <game_sa\CScene.h>

CD3D1XShader* CVolumetricLighting::m_pVolumetricSunlightPS=nullptr;
CD3D1XShader* CVolumetricLighting::m_pVolumetricCombinePS = nullptr;
RwRaster*	 CVolumetricLighting::m_pVolumeLightingRaster = nullptr;
CD3D1XConstantBuffer<CBVolumetricLighting>* CVolumetricLighting::m_pVolumeLightingCB = nullptr;
bool	CVolumetricLighting::m_bRequiresReloading = false;
VolumetricLightingSettingsBlock gVolumetricLightingSettings;

void CVolumetricLighting::Init()
{
	m_pVolumetricSunlightPS = new CD3D1XPixelShader("shaders/VolumetricLighting.hlsl", "VolumetricSunlightPS", gVolumetricLightingSettings.m_pShaderDefineList);
	m_pVolumetricCombinePS = new CD3D1XPixelShader("shaders/VolumetricLighting.hlsl", "VolumetricCombinePS", gVolumetricLightingSettings.m_pShaderDefineList);
	m_pVolumeLightingRaster = RwRasterCreate((RwInt32)(RsGlobal.maximumWidth*gVolumetricLightingSettings.VolumetricRenderingScale),
											(RwInt32)(RsGlobal.maximumHeight*gVolumetricLightingSettings.VolumetricRenderingScale), 
											32, rwRASTERTYPECAMERATEXTURE);
	m_pVolumeLightingCB = new CD3D1XConstantBuffer<CBVolumetricLighting>();
	gVolumetricLightingSettings.m_aShaderPointers.push_back(m_pVolumetricSunlightPS);
	gVolumetricLightingSettings.m_aShaderPointers.push_back(m_pVolumetricCombinePS);
	gDebugSettings.DebugRenderTargetList.push_back(m_pVolumeLightingRaster);
}

void CVolumetricLighting::Shutdown()
{
	if (m_pVolumeLightingCB)
		delete m_pVolumeLightingCB;
	if (m_pVolumetricSunlightPS)
		delete m_pVolumetricSunlightPS;
	if (m_pVolumetricCombinePS)
		delete m_pVolumetricCombinePS;
	if (m_pVolumeLightingRaster)
		RwRasterDestroy(m_pVolumeLightingRaster);
}

void CVolumetricLighting::RenderVolumetricEffects(RwRaster* normalsDepth,RwRaster* cascadeShadowMap, RwRaster* result)
{
	m_pVolumeLightingCB->data.RaymarchingDistance = gVolumetricLightingSettings.RaymarchingDistance;
	m_pVolumeLightingCB->data.SunlightBlendOffset = gVolumetricLightingSettings.SunlightBlendOffset;
	m_pVolumeLightingCB->data.SunlightIntensity = gVolumetricLightingSettings.SunlightIntensity;
	m_pVolumeLightingCB->Update();
	g_pStateMgr->SetConstantBufferPS(m_pVolumeLightingCB, 7);
	// First render all volumetric lighting(sun light, volume streetlights, clouds etc.)
	g_pRwCustomEngine->SetRenderTargets(&m_pVolumeLightingRaster, Scene.m_pRwCamera->zBuffer, 1);
	g_pStateMgr->FlushRenderTargets();
	g_pStateMgr->SetRaster(cascadeShadowMap, 3);
	m_pVolumetricSunlightPS->Set();
	CFullscreenQuad::Draw();
	CFullscreenQuad::Copy(result, Scene.m_pRwCamera->zBuffer);
	// Than combine them with 
	g_pRwCustomEngine->SetRenderTargets(&result, Scene.m_pRwCamera->zBuffer, 1);
	g_pStateMgr->FlushRenderTargets();
	g_pStateMgr->SetRaster(CFullscreenQuad::m_pBlitRaster, 0);
	g_pStateMgr->SetRaster(m_pVolumeLightingRaster, 2);
	m_pVolumetricCombinePS->Set();
	CFullscreenQuad::Draw();
}

void CVolumetricLighting::QueueTextureReload()
{
	CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;

	if (m_bRequiresReloading || dxEngine->m_bScreenSizeChanged) {
		m_pVolumeLightingRaster->width = (int)(RsGlobal.maximumWidth*gVolumetricLightingSettings.VolumetricRenderingScale);
		m_pVolumeLightingRaster->height = (int)(RsGlobal.maximumHeight*gVolumetricLightingSettings.VolumetricRenderingScale);
		dxEngine->m_pRastersToReload.push_back(m_pVolumeLightingRaster);
	}
}

tinyxml2::XMLElement * VolumetricLightingSettingsBlock::Save(tinyxml2::XMLDocument * doc)
{
	auto settingsNode = doc->NewElement(m_sName.c_str());
	settingsNode->SetAttribute("SunlightRMSteps", SunlightRaymarchingSteps);
	settingsNode->SetAttribute("SunlightBlendOffset", SunlightBlendOffset);
	settingsNode->SetAttribute("SunlightIntensity", SunlightIntensity);
	settingsNode->SetAttribute("RaymarchingDistance", RaymarchingDistance);
	settingsNode->SetAttribute("VolumetricRenderingScale", VolumetricRenderingScale);

	return settingsNode;
}

void VolumetricLightingSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	auto settingsNode = doc.FirstChildElement(m_sName.c_str());
	if (settingsNode == nullptr) {
		Reset();
		return;
	}
	SunlightRaymarchingSteps = settingsNode->UnsignedAttribute("SunlightRMSteps", 16);
	SunlightBlendOffset = settingsNode->FloatAttribute("SunlightBlendOffset", 0.25f);
	SunlightIntensity = settingsNode->FloatAttribute("SunlightIntensity", 0.25f);
	RaymarchingDistance = settingsNode->FloatAttribute("RaymarchingDistance", 40.0f);
	VolumetricRenderingScale = settingsNode->FloatAttribute("VolumetricRenderingScale", 0.95f);

	m_pShaderDefineList = new CD3D1XShaderDefineList();
	m_pShaderDefineList->AddDefine("SUNLIGHT_RM_STEPS", std::to_string(SunlightRaymarchingSteps));
}

void VolumetricLightingSettingsBlock::Reset()
{
	SunlightRaymarchingSteps = 16;
	SunlightBlendOffset = 0.25f;
	SunlightIntensity = 0.25f;
	RaymarchingDistance = 40.0f;
	VolumetricRenderingScale = 0.95f;
}
void TW_CALL ReloadVolumetricShadersCallBack(void *value)
{
	gVolumetricLightingSettings.m_bShaderReloadRequired = true;
	gVolumetricLightingSettings.m_pShaderDefineList->Reset();
	gVolumetricLightingSettings.m_pShaderDefineList->AddDefine("SUNLIGHT_RM_STEPS", std::to_string(gVolumetricLightingSettings.SunlightRaymarchingSteps));
}
void TW_CALL ReloadVolumetricTexturesCallBack(void *value)
{
	CVolumetricLighting::m_bRequiresReloading = true;
}
void VolumetricLightingSettingsBlock::InitGUI(TwBar * mainBar)
{
	TwAddVarRW(mainBar, "Volumetric sunlight intensity", TwType::TW_TYPE_FLOAT, &SunlightIntensity, "min=0.001 max=5.0 step=0.001 group=VolumetricLighting");
	TwAddVarRW(mainBar, "Volumetric sunlight blend offset", TwType::TW_TYPE_FLOAT, &SunlightBlendOffset, "min=-1.0 max=1.0 step=0.01 group=VolumetricLighting");
	TwAddVarRW(mainBar, "Volumetric rendering scale", TwType::TW_TYPE_FLOAT, &VolumetricRenderingScale, "min=0.1 max=1.0 step=0.05 group=VolumetricLighting");
	TwAddVarRW(mainBar, "Raymarching distance", TwType::TW_TYPE_FLOAT, &RaymarchingDistance, "min=1 max=1000 step=0.5 group=VolumetricLighting");
	TwAddVarRW(mainBar, "Raymarching steps(Sunlight)", TwType::TW_TYPE_UINT32, &SunlightRaymarchingSteps, "min=2 max=256 step=1 group=VolumetricLighting");
	TwAddButton(mainBar, "Reload volume shaders", ReloadVolumetricShadersCallBack, nullptr, "group=VolumetricLighting");
	TwAddButton(mainBar, "Reload volume textures", ReloadVolumetricTexturesCallBack, nullptr, "group=VolumetricLighting");
}
