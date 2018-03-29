#pragma once
#include <game_sa\CEntity.h>
#include <game_sa\CBaseModelInfo.h>
#include <game_sa\CModelInfo.h>
#include <game_sa\CPtrList.h>
#define CRenderer__ms_bRenderTunnels (*(unsigned char *)0xB745C0)
#define CRenderer__ms_bRenderOutsideTunnels (*(unsigned char *)0xB745C1)
#define CRenderer__ms_bInTheSky (*(unsigned char *)0xB76851)
#define CRenderer__ms_nNoOfVisibleEntities (*(UINT *)0xB76844)
#define CRenderer__ms_nNoOfVisibleLods (*(UINT *)0xB76840)
#define CRenderer__ms_nNoOfInVisibleEntities (*(UINT *)0xB7683C)

#define CRenderer__ms_fFarClipPlane (*(float *)0xB76848)
#define CRenderer__ScanWorld() ((void (__cdecl *)())0x554FE0)()
#define CWorld__ms_nCurrentScanCode (*(unsigned short *)0xB7CD78)
#define CWorld__ClearScanCodes() ((void (__cdecl *)())0x563470)()
#define CVisibilityPlugins__InitAlphaEntityList() ((void (__cdecl *)())0x734540)()
#define CWorldScan__ScanWorld(a1,a2,a3) ((void (__cdecl *)(RwV2d *, signed int , void (__cdecl *)(int, int) ) )0x72CAE0)(a1,a2,a3)
#define CRenderer__ScanSectorList(a1,a2) ((void (__cdecl *)(int, int))0x554840)(a1,a2)
#define CRenderer__ScanBigBuildingList(a1,a2) ((void (__cdecl *)(int, int))0x554B10)(a1,a2)
//553F60     ; signed int __cdecl CRenderer::SetupMapEntityVisibility(CEntity *entity, CBaseModelInfo *mapInfo, float renderDistance, char a4)
#define CRenderer__SetupMapEntityVisibility(entity,mapInfo,renderDistance,a4) ((signed int (__cdecl *)(CEntity *, CBaseModelInfo *,float,char))0x553F60)(entity,mapInfo,renderDistance,a4)
//5534B0     ; char __cdecl CRenderer::AddEntityToRenderList(CEntity *a1, float renderDistance)
#define CRenderer__AddEntityToRenderList(entity,renderDistance) ((char (__cdecl *)(CEntity *, float))0x5534B0)(entity,renderDistance)
//553710     ; int __cdecl CRenderer::AddToLodRenderList(CEntity *a1, float a2)
#define CRenderer__AddToLodRenderList(entity,renderDistance) ((char (__cdecl *)(CEntity *, float))0x553710)(entity,renderDistance)
#define TheCamera_ (*(CCamera *)0xB6F028)
//B992B8     _ZN6CWorld17ms_aRepeatSectorsE CRepeatSector

enum RendererVisibility {
	INVISIBLE,
	VISIBLE,
	UNKNOWN_0,
	NOT_LOADED,
};
struct sLodListEntry {
	CEntity* entity;
	float distance;
};
/*enum eEntityType
{
	ENTITY_TYPE_NOTHING,
	ENTITY_TYPE_BUILDING,
	ENTITY_TYPE_VEHICLE,
	ENTITY_TYPE_PED,
	ENTITY_TYPE_OBJECT,
	ENTITY_TYPE_DUMMY,
	ENTITY_TYPE_NOTINPOOLS
};*/
//eEntityType getEntityType(CEntity* e);
class CRenderer
{
public:
	// methods
	static void RenderRoads(bool(*predicate)(void* entity));
	static void RenderEverythingBarRoads();
	static void RenderTOBJs();
	static void RenderShadowCascade(int i);
	static void ConstructShadowList();
	static void ConstructRenderList();
	static void RenderFadingInEntities();
	static void AddEntityToRenderList(CEntity* entity, float renderDistance);
	static char AddEntityToShadowCasterList(CEntity* entity, float renderDistance);
	static char AddToLodRenderList(CEntity* entity, float renderDistance);
	static bool CheckInsideFrustum(RwCamera* camera, const RwBBox& b);
	static RendererVisibility SetupEntityVisibility(CEntity* entity, float* renderDistance);
	static RendererVisibility SetupMapEntityVisibility(CEntity *entity, CBaseModelInfo *mapInfo, float renderDistance, char a4);
	static void RenderOneRoad(CEntity*);
	static void RenderOneNonRoad(CEntity*);
	static bool IsCulledByTunnel(CEntity*);
	static void ScanSectorList(int x, int y);
	static void ScanPtrList(CPtrList* ptrList, bool loadIfRequired);
	static void ScanPtrListForShadows(CPtrList* ptrList, bool loadIfRequired);
	static void ScanBigBuildingList(int x, int y);
	static void ScanSectorListForShadowCasters(int x, int y);
	static void PreRender();
	static void ProcessLodRenderLists();
	static void ScanWorld(RwCamera* camera, RwV3d* gameCamPos, float shadowStart, float shadowEnd);
	static void ScanWorld();
	static void ResetLodRenderLists();
	static sLodListEntry *GetLodRenderListBase();
	static sLodListEntry *GetLodDontRenderListBase();
public:
	// fields
	static bool		&ms_bRenderTunnels;
	static bool		&ms_bRenderOutsideTunnels;
	static bool		&ms_bInTheSky;
	static UINT&		ms_nNoOfVisibleEntities;
	static UINT&		ms_nNoOfInVisibleEntities;
	static UINT&		ms_nNoOfVisibleLods;
	static UINT&		m_loadingPriority;
	static CEntity*		m_pFirstPersonVehicle;
	static CVector		&ms_vecCameraPosition;
	static float&		ms_fCameraHeading;
	static float&		ms_lowLodDistScale;
	static float&		ms_fFarClipPlane;
	static CEntity** ms_aVisibleEntityPtrs;
	static CEntity** ms_aInVisibleEntityPtrs;
	static CEntity** ms_aVisibleLodPtrs;
	static sLodListEntry* &ms_pLodRenderList;
	static sLodListEntry* &ms_pLodDontRenderList;
	
	//static CBaseModelInfo **ms_modelInfoPtrs;
	static bool TOBJpass;
	static std::list<CEntity*> ms_aVisibleEntities;
	static std::list<CEntity*> ms_aVisibleLods;
	static std::list<CEntity*> ms_aVisibleShadowCasters[4];
};

