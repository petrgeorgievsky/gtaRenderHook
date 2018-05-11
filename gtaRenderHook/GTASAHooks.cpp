// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "stdafx.h"
#include "GTASAHooks.h"
#include "StreamingRH.h"
#include "CVisibilityPluginsRH.h"
#include "SAIdleHook.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3D1XStateManager.h"
#include <game_sa\CRadar.h>
#include <game_sa\CHud.h>

#include <game_sa\CVehicleModelInfo.h>
#include <game_sa\CModelInfo.h>
#include "LightManager.h"
#include "FullscreenQuad.h"
#include "DebugRendering.h"
#include "CustomBuildingPipeline.h"
#include "CustomBuildingDNPipeline.h"
#include "CustomCarFXPipeline.h"
#include "CustomSeabedPipeline.h"
#include "CustomWaterPipeline.h"
#include "DeferredRenderer.h"
#include "VolumetricLighting.h"
#include "AmbientOcclusion.h"
#include "DebugBBox.h"
#include "PBSMaterial.h"

HHOOK CGTASAHooks::m_hAntTweakBarHook;

void CGTASAHooks::Patch()
{
	CSAIdleHook::Patch();
	CVisibilityPluginsRH::Patch();
	CStreamingRH::Patch();

	SetPointer(0x4C9AB5, RwD3D11RasterDestructor);

	RedirectJump(0x730830, InitD3DCacheSystem);
	RedirectJump(0x730A00, ShutdownD3DCacheSystem);

	RedirectJump(0x7460A0, GetBestRefreshRate);
	
	RedirectJump(0x7000E0, AddLight);
	RedirectCall(0x6E27E6, AddLightNoSpot);// fix for original car spotlights

	RedirectJump(0x6E0E20, AddSpotLight);
	RedirectJump(0x585700, RenderRadar);
	RedirectJump(0x586D4E, RenderRadarMap);

	// GTA stuff
	RedirectCall(0x5BD76F, InitWithRW);
	RedirectCall(0x53BBA2, ShutdownWithRW);
}



LRESULT CALLBACK CGTASAHooks::AntTweakBarHookMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (lParam & 0x80000000 || lParam & 0x40000000)
	{
		return CallNextHookEx(m_hAntTweakBarHook, code, wParam, lParam);
	}

	if (code >= 0) {
		if (TwEventWin(((LPMSG)lParam)->hwnd, ((LPMSG)lParam)->message, ((LPMSG)lParam)->wParam, ((LPMSG)lParam)->lParam)) {
			return FALSE;
		}
	}
	return CallNextHookEx(m_hAntTweakBarHook, code, wParam, lParam);
}

void CGTASAHooks::InitWithRW() {
#ifdef USE_ANTTWEAKBAR
	m_hAntTweakBarHook = SetWindowsHookEx(WH_GETMESSAGE, AntTweakBarHookMsgProc, NULL, GetCurrentThreadId());
#endif
	CHud::Initialise();
	CFullscreenQuad::Init();
	DebugRendering::Init();
	g_pDeferredRenderer = new CDeferredRenderer();
	g_pCustomBuildingPipe = new CCustomBuildingPipeline();
	g_pCustomBuildingDNPipe = new CCustomBuildingDNPipeline();
	g_pCustomCarFXPipe = new CCustomCarFXPipeline();
	g_pCustomSeabedPipe = new CCustomSeabedPipeline();
	if (GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0)
		g_pCustomWaterPipe = new CCustomWaterPipeline();
	CLightManager::Init();
	CVolumetricLighting::Init();
	CAmbientOcclusion::Init();
	//CVoxelOctreeRenderer::Init();
	DebugBBox::Initialize();
	CPBSMaterialMgr::LoadMaterials();
}

void CGTASAHooks::ShutdownWithRW() {
	DebugBBox::Shutdown();
	DebugRendering::Shutdown();
	CAmbientOcclusion::Shutdown();
	//CVoxelOctreeRenderer::Shutdown();
	CVolumetricLighting::Shutdown();
	CLightManager::Shutdown();
	delete g_pCustomWaterPipe;
	delete g_pCustomSeabedPipe;
	delete g_pCustomBuildingDNPipe;
	delete g_pCustomBuildingPipe;
	delete g_pCustomCarFXPipe;
	delete g_pDeferredRenderer;
	CFullscreenQuad::Shutdown();
	CHud::Shutdown();
#ifdef USE_ANTTWEAKBAR
	UnhookWindowsHookEx(m_hAntTweakBarHook);
#endif
}


void CGTASAHooks::InitD3DCacheSystem()
{
}

void CGTASAHooks::ShutdownD3DCacheSystem()
{
}

int CGTASAHooks::GetBestRefreshRate(int width, int height, int depth)
{
	return -1;
}

void* CGTASAHooks::RwD3D11RasterDestructor(void* raster, RwInt32 offset, RwInt32 size) 
{
	return raster;
}

void CGTASAHooks::RenderRadar(RwPrimitiveType prim, RwIm2DVertex * vert, int count) 
{
	CVector2D radarSquare[4] = { { -1.0f,1.0f },{ 1.0f,1.0f },{ 1.0f,-1.0f },{ -1.0f,-1.0f } };
	CVector2D radarSquareScreenSpace[8] = {};
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILENABLE, 1);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEZTESTENABLE, 0);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEZWRITEENABLE, 0);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFUNCTIONREF, 5);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFUNCTIONMASK, 0xFF);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFUNCTIONWRITEMASK, 0xFF);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFUNCTION, rwSTENCILFUNCTIONALWAYS);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILPASS, rwSTENCILOPERATIONREPLACE);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFAIL, rwSTENCILOPERATIONKEEP);
	// TODO: Replace with proper render state set call
	g_pStateMgr->SetAlphaTestEnable(false);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, 1);
	for (int i = 0; i < 4; i++)
	{
		CRadar::TransformRadarPointToScreenSpace(radarSquareScreenSpace[0], radarSquare[i]);

		for (int j = 1; j < 8; j++)
		{
			CVector2D vert(cos((j - 1)*0.2617994f)*radarSquare[i].x, sin((j - 1)*0.2617994f)*radarSquare[i].y);
			CRadar::TransformRadarPointToScreenSpace(radarSquareScreenSpace[j], vert);
		}
		CSprite2d::SetMaskVertices(8, (float*)radarSquareScreenSpace, CSprite2d::NearScreenZ);
		g_pRwCustomEngine->Im2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 8);
	}

	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFUNCTIONREF, 5);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFUNCTIONWRITEMASK, 0);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFUNCTION, rwSTENCILFUNCTIONNOTEQUAL);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILPASS, rwSTENCILOPERATIONKEEP);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILFAIL, rwSTENCILOPERATIONKEEP);
}

void CGTASAHooks::RenderRadarMap() {
	CRadar::DrawRadarMap();
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILENABLE, 0);
	//g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, rwALPHATESTFUNCTIONGREATEREQUAL);
	g_pStateMgr->SetAlphaTestEnable(true);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, 1);
}

bool CGTASAHooks::AddLight(char type, CVector pos, CVector dir, float radius, float red, float green, float blue, char fogType, char generateExtraShadows, CEntity *entityAffected) {
	CLight light{};

	light.m_fRange = radius;
	if (type == 0) {
		light.m_vDir = { red,green,blue };
		light.m_vPos = pos.ToRwV3d();
	}
	else if (type == 1) {
		light.m_vDir = { dir.x,dir.y,dir.z };
		light.m_vPos = { pos.x + dir.x,pos.y + dir.y,pos.z + dir.z };
	}
	light.m_nLightType = type;
	return CLightManager::AddLight(light);
}

bool CGTASAHooks::AddLightNoSpot(char type, CVector pos, CVector dir, float radius, float red, float green, float blue, char fogType, char generateExtraShadows, CEntity *entityAffected) {
	return true;
}

void __fastcall  CGTASAHooks::AddSpotLight(CVehicle* vehicle, void *Unknown, int a, CMatrix* matrix, bool isRight) {
	CLight light{};
	CVehicleModelInfo* vehicleMI = reinterpret_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[vehicle->m_nModelIndex]);
	CVector* headlightPos = &vehicleMI->m_pVehicleStruct->m_avDummyPos[a];//reinterpret_cast<CVector*>( reinterpret_cast<UINT>(vehicleMI) + 0x5C);
																		  //RwMatrix m = matrix->;
	light.m_vPos = {
		matrix->at.x * headlightPos->z + matrix->up.x * headlightPos->y + matrix->right.x * headlightPos->x + matrix->pos.x ,
		matrix->at.y * headlightPos->z + matrix->up.y * headlightPos->y + matrix->right.y * headlightPos->x + matrix->pos.y ,
		matrix->at.z * headlightPos->z + matrix->up.z * headlightPos->y + matrix->right.z * headlightPos->x + matrix->pos.z };
	if (!isRight) {
		float dist = headlightPos->x + headlightPos->x;
		light.m_vPos = { light.m_vPos.x - dist * matrix->right.x ,light.m_vPos.y - dist * matrix->right.y,light.m_vPos.z - dist * matrix->right.z };
	}
	float distance = 0.15f;
	light.m_vPos = { light.m_vPos.x + matrix->up.x*distance , light.m_vPos.y + matrix->up.y*distance, light.m_vPos.z + matrix->up.z*distance };
	light.m_fRange = 10;
	light.m_vDir = { matrix->up.x,matrix->up.y,matrix->up.z };
	light.m_nLightType = 1;
	CLightManager::AddLight(light);
}