// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "CDebug.h"
//#include "RwRenderEngine.h"
//#include "RwVulkanEngine.h"
#include "RwD3D1XEngine.h"
#include "SAIdleHook.h"
#include "CustomBuildingPipeline.h"
#include "CustomBuildingDNPipeline.h"
#include "CustomCarFXPipeline.h"
#include "CustomSeabedPipeline.h"
#include "CustomWaterPipeline.h"
#include "DeferredRenderer.h"
#include "VoxelOctreeRenderer.h"
#include "LightManager.h"
#include "FullscreenQuad.h"
#include "D3D1XTextureMemoryManager.h"
#include "DebugBBox.h"
#include "HDRTonemapping.h"
#include "ShadowRenderer.h"
#include "gta_sa_ptrs.h"
#include "SettingsHolder.h"
#include "D3DRenderer.h"
#include "PBSMaterial.h"
#include "D3D1XStateManager.h"
#include "CVisibilityPluginsRH.h"
#include <game_sa\CModelInfo.h>
#include <game_sa\CVehicle.h>
#include <game_sa\CRadar.h>
#include <game_sa\CGame.h>
#include "DebugRendering.h"
#include "VolumetricLighting.h"
#include "AmbientOcclusion.h"
#include <game_sa\CStreaming.h>
#include <game_sa\CEntity.h>
#include "CRwGameHooks.h"
#include "StreamingRH.h"
CDebug*				g_pDebug;
CIRwRenderEngine*	g_pRwCustomEngine;

HHOOK hookHandle;

LRESULT CALLBACK MessageProc(int code, WPARAM wParam, LPARAM lParam) {
	if (lParam & 0x80000000 || lParam & 0x40000000)
	{
		return CallNextHookEx(hookHandle, code, wParam, lParam);
	}

	if (code >= 0) {
		if (TwEventWin(((LPMSG)lParam)->hwnd, ((LPMSG)lParam)->message, ((LPMSG)lParam)->wParam, ((LPMSG)lParam)->lParam)) {
			return FALSE;
		}
	}
	return CallNextHookEx(hookHandle, code, wParam, lParam);
}

// GTA RenderWare Initialization hooks
char GTARwInit() {
#ifdef USE_ANTTWEAKBAR
	hookHandle = SetWindowsHookEx(WH_GETMESSAGE, MessageProc, NULL, GetCurrentThreadId());
#endif
	char c = CGame::InitialiseRenderWare();
	CFullscreenQuad::Init();
	DebugRendering::Init();
	g_pDeferredRenderer = new CDeferredRenderer();
	g_pCustomBuildingPipe = new CCustomBuildingPipeline();
	g_pCustomBuildingDNPipe = new CCustomBuildingDNPipeline();
	g_pCustomCarFXPipe = new CCustomCarFXPipeline();
	g_pCustomSeabedPipe = new CCustomSeabedPipeline();
	if(GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0)
		g_pCustomWaterPipe = new CCustomWaterPipeline();
	CLightManager::Init();
	CVolumetricLighting::Init();
	CAmbientOcclusion::Init();
	//CVoxelOctreeRenderer::Init();
	DebugBBox::Initialize();
	CPBSMaterialMgr::LoadMaterials();
	return c;
}
void GTARwShutdown() {
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
	CGame::ShutdownRenderWare();
#ifdef USE_ANTTWEAKBAR
	UnhookWindowsHookEx(hookHandle);
#endif
}

RxD3D9InstanceData* GetModelsData(RxInstanceData * data)
{
	return reinterpret_cast<RxD3D9InstanceData*>(data + 1);
}
RpMesh * GetMeshesData(RpD3DMeshHeader * data)
{
	return reinterpret_cast<RpMesh*>(data + 1);
}

bool AddLight(char type, CVector pos, CVector dir, float radius, float red, float green, float blue, char fogType, char generateExtraShadows, CEntity *entityAffected) {
	CLight light{};
	
	light.m_fRange = radius;
	if (type == 0) {
		light.m_vDir = { red,green,blue };
		light.m_vPos = pos.ToRwV3d();
	}
	else if (type == 1) {
		light.m_vDir = { dir.x,dir.y,dir.z };
		light.m_vPos = { pos.x+dir.x,pos.y + dir.y,pos.z + dir.z };
	}
	light.m_nLightType = type;
	return CLightManager::AddLight(light);
}

bool AddLightNoSpot(char type, CVector pos, CVector dir, float radius, float red, float green, float blue, char fogType, char generateExtraShadows, CEntity *entityAffected) {
	return true;
}
CLink<CEntity*> * rwobjlink_Add(CLinkList<CEntity*> *list, CEntity *entity)
{
	CLink<CEntity*>* curr_link = list->freeListHead.next;
	if (curr_link == &list->freeListTail)
		return nullptr;
	else
	{
		curr_link->data = entity;
		curr_link->next->prev = curr_link->prev;
		curr_link->prev->next = curr_link->next;
		curr_link->next = list->usedListHead.next;
		list->usedListHead.next->prev = curr_link;
		curr_link->prev = &list->freeListHead;
		list->usedListHead.next = curr_link;
	}
	return curr_link;
}
#define rw_objlink_Add(a,b) ((CLink<CEntity*> * (__thiscall*)(CLinkList<CEntity*> *, CEntity **))0x408230)(a,b)
CLink<CEntity*> * CStreaming__AddEntity(CEntity *pEntity)
{
	CLink<CEntity*> *result = nullptr; // eax@3
	CLink<CEntity*> *currLink; // eax@4

	eEntityType entityType = (eEntityType)pEntity->m_nType;
	if (entityType == eEntityType::ENTITY_TYPE_PED || entityType == eEntityType::ENTITY_TYPE_VEHICLE)
		return nullptr;
	else
	{
		result = rw_objlink_Add(&CStreaming::ms_rwObjectInstances, &pEntity);
		if (result==nullptr) // if we fail to add link
		{
			currLink = CStreaming::ms_rwObjectInstances.usedListTail.prev;

			if (CStreaming::ms_rwObjectInstances.usedListTail.prev != &CStreaming::ms_rwObjectInstances.usedListHead)
			{
				while (true)
				{
					if (!(currLink->data->m_bStreamingDontDelete ||	currLink->data->m_bImBeingRendered))
						break;
					currLink = currLink->prev;
					if (currLink == &CStreaming::ms_rwObjectInstances.usedListHead) {
						pEntity->DeleteRwObject();
						return nullptr;//rw_objlink_Add(&CStreaming__ms_rwObjectInstances, &pEntity);
					}
				}
				currLink->data->DeleteRwObject();
				currLink->data = pEntity;
				//result = rw_objlink_Add(&CStreaming__ms_rwObjectInstances, &pEntity);
			}
			else {
				pEntity->DeleteRwObject();
				//result = rw_objlink_Add(&CStreaming__ms_rwObjectInstances, &pEntity);
			}
		}
	}
	return result;
}
// TODO: Move this out to lights
void __fastcall  AddSpotLight(CVehicle* vehicle, void *Unknown, int a,CMatrix* matrix, bool isRight) {
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
		light.m_vPos = { light.m_vPos.x - dist*matrix->right.x ,light.m_vPos.y - dist*matrix->right.y,light.m_vPos.z - dist*matrix->right.z };
	}
	float distance = 0.15f;
	light.m_vPos = { light.m_vPos.x + matrix->up.x*distance , light.m_vPos.y + matrix->up.y*distance, light.m_vPos.z + matrix->up.z*distance };
	light.m_fRange = 10;
	light.m_vDir = { matrix->up.x,matrix->up.y,matrix->up.z };
	light.m_nLightType = 1;
	CLightManager::AddLight(light);
}
// defines
#if (GTA_SA)//
#define RenderSystemPtr 0x8E249C
#define SetRSPtr 0x8E24A8
#define GetRSPtr 0x8E24AC

#define Im2DRenderPrimPtr 0x8E24B8
#define Im2DRenderIndexedPrimPtr 0x8E24BC
#define Im2DRenderLinePtr 0x8E24B0
#define Im3DSubmitPtr 0x8E297C

#define AtomicAllInOneNodePtr 0x8D633C
#define SkinAllInOneNodePtr 0x8DED0C
#define DefaultInstanceCallbackPtr 0x757899

#define SetRRPtr 0x7F8580
#define SetVMPtr 0x7F8640
//#define Im3DOpenPtr 0x53EA0D

#define RwInitPtr 0x5BF3A1
#define RwShutdownPtr 0x53D910

#define psNativeTextureSupportPtr 0x619D40
#define Im3DOpenPtr 0x80A225

#define CGammaInitPtr 0x747180
#define envMapSupportPtr 0x5D8980
#elif (GTA_III)
#define RenderSystemPtr 0x61948C
#define SetRSPtr 0x619498
#define GetRSPtr 0x61949C

#define Im2DRenderPrimPtr 0x6194A8
#define Im2DRenderIndexedPrimPtr 0x6194AC
#define Im2DRenderLinePtr 0x6194A0
#define Im3DSubmitPtr 0x8E297C

#define AtomicAllInOneNodePtr 0x61B6A4
#define SkinAllInOneNodePtr 0x61BBAC
#define DefaultInstanceCallbackPtr 0x5DB429

#define SetRRPtr 0x74631E
#define SetVMPtr 0x745C75
#define Im3DOpenPtr 0x53EA0D

#define RwInitPtr 0x5BF3A1
#define RwShutdownPtr 0x53D910

#define psNativeTextureSupportPtr 0x584C0B
//#define Im3DOpenPtr 0x80A225

#define CGammaInitPtr 0x748A30
#define envMapSupportPtr 0x5DA044
#elif (GTA_VC)
#define RenderSystemPtr 0x6DDE30
#define SetRSPtr 0x6DDE3E
#define GetRSPtr 0x6DDE40

#define Im2DRenderPrimPtr 0x6DDE4C
#define Im2DRenderIndexedPrimPtr 0x6DDE50
#define Im2DRenderLinePtr 0x6DDE44
//#define Im3DSubmitPtr 0x8E297C

//#define AtomicAllInOneNodePtr 0x8D633C
//#define SkinAllInOneNodePtr 0x8DED0C
//#define DefaultInstanceCallbackPtr 0x757899

//#define SetRRPtr 0x74631E
//#define SetVMPtr 0x745C75
//#define Im3DOpenPtr 0x6431F6

//#define RwInitPtr 0x5BF3A1
//#define RwShutdownPtr 0x53D910

#define psNativeTextureSupportPtr 0x602E8B
//#define Im3DOpenPtr 0x80A225

//#define CGammaInitPtr 0x748A30
//#define envMapSupportPtr 0x5DA044
#endif 

int GetBestRR(int a, int b, int c) {
	return -1;
}
void* destructRwD3D11Raster(void* raster, RwInt32 offset, RwInt32 size) {
	/*auto dxRaster= GetD3D1XRaster(raster);
	if (dxRaster&&dxRaster->resourse) {
		delete dxRaster->resourse;
		dxRaster->resourse = nullptr;
	}*/
	return raster;
}
void RenderRadarSA(RwPrimitiveType prim, RwIm2DVertex * vert, int count) {
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
	g_pStateMgr->SetAlphaTestEnable(false);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, 1);
	for (int i = 0; i < 4; i++)
	{
		CRadar::TransformRadarPointToScreenSpace(radarSquareScreenSpace[0], radarSquare[i]);

		for (int j = 1; j < 8; j++)
		{
			CVector2D vert(cos((j-1)*0.2617994f)*radarSquare[i].x, sin((j-1)*0.2617994f)*radarSquare[i].y);
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
void RenderRadarSAPrimHook(RwPrimitiveType prim, RwIm2DVertex * vert, int count) {
	CRadar::DrawRadarMap();
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILENABLE, 0);
	//g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, rwALPHATESTFUNCTIONGREATEREQUAL);
	g_pStateMgr->SetAlphaTestEnable(true);
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, 1);
}


void InitD3DResourseSystem() {

}
void ShowCursor_fix(UINT show) {
	
}
void TidyUpTextures(int n) {
	CD3D1XTextureMemoryManager::Shutdown();
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	UNREFERENCED_PARAMETER(lpReserved);
	UNREFERENCED_PARAMETER(hModule);
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SettingsHolder::Instance.AddSettingBlock(&gDebugSettings);
		SettingsHolder::Instance.AddSettingBlock(&gShaderDefineSettings);
		SettingsHolder::Instance.AddSettingBlock(&gTonemapSettings);
		SettingsHolder::Instance.AddSettingBlock(&gShadowSettings);
		SettingsHolder::Instance.AddSettingBlock(&gDeferredSettings);
		SettingsHolder::Instance.AddSettingBlock(&gWaterSettings);
		SettingsHolder::Instance.AddSettingBlock(&gVolumetricLightingSettings);
		SettingsHolder::Instance.AddSettingBlock(&gAmbientOcclusionSettings);
		SettingsHolder::Instance.ReloadFile();
		g_pDebug = new CDebug("debug.log");
		g_pRwCustomEngine = new CRwD3D1XEngine(g_pDebug);
		CCustomBuildingPipeline::Patch();
		CCustomBuildingDNPipeline::Patch();
		CCustomCarFXPipeline::Patch();
		CSAIdleHook::Patch();
		CRwGameHooks::Patch(CRwGameHooks::ms_rwPointerTableSA);
#if GTA_SA
		CVisibilityPluginsRH::Patch();
		CStreamingRH::Patch();
		RedirectJump(0x730830, InitD3DResourseSystem);
		RedirectJump(0x730A00, InitD3DResourseSystem);// Shutdown
		//RedirectCall(0x53C812, TidyUpTextures);
		RedirectJump(0x7460A0, GetBestRR);
		SetPointer(0x4C9AB5, destructRwD3D11Raster);
		RedirectJump(0x7000E0, AddLight);
		RedirectCall(0x6E27E6, AddLightNoSpot);// fix for original car spotlights
		/*SetInt(0x5B8E55, 12 * 2000);
		SetInt(0x5B8EB0, 12 * 2000);*/
		RedirectJump(0x6E0E20, AddSpotLight);
		RedirectJump(0x585700, RenderRadarSA);
		RedirectJump(0x586D4E, RenderRadarSAPrimHook);
		
#endif // GTA_SA

#if (GTA_III)
		SetPointer(0x5A151F, envMapSupport);
		SetPointer(0x5DB43D, _rxD3D9DefaultRenderCallback);
#endif
		// GTA stuff
		RedirectCall(RwInitPtr, GTARwInit);
		RedirectCall(RwShutdownPtr, GTARwShutdown);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete g_pDebug;
		delete g_pRwCustomEngine;
		break;
	}
	return TRUE;
}