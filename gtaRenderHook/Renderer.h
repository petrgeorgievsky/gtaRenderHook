#pragma once
#include <game_sa\CEntity.h>
#include <game_sa\CBaseModelInfo.h>
#include <game_sa\CModelInfo.h>
#include <game_sa\CPtrList.h>
#include "RwVectorMath.h"
#define CRenderer__ms_bRenderTunnels (*(unsigned char *)0xB745C0)
#define CRenderer__ms_bRenderOutsideTunnels (*(unsigned char *)0xB745C1)
#define CRenderer__ms_bInTheSky (*(unsigned char *)0xB76851)
#define CRenderer__ms_nNoOfVisibleEntities (*(UINT *)0xB76844)
#define CRenderer__ms_nNoOfVisibleLods (*(UINT *)0xB76840)
#define CRenderer__ms_nNoOfInVisibleEntities (*(UINT *)0xB7683C)

#define CRenderer__ms_fFarClipPlane (*(float *)0xB76848)
#define CRenderer__ScanWorld() ((void (__cdecl *)())0x554FE0)()
#define CWorld__ms_nCurrentScanCode (*(short *)0xB7CD78)
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

#define CRenderer__SetupBigBuildingVisibility(entity,renderDistance) ((char (__cdecl *)(CEntity *, float*))0x554650)(entity,renderDistance)
//B992B8     _ZN6CWorld17ms_aRepeatSectorsE CRepeatSector

enum class RendererVisibility
{
    INVISIBLE,
    VISIBLE,
    OFFSCREEN,
    NOT_LOADED,
};

struct sLodListEntry
{
    CEntity* entity;
    float distance;
};

class CRendererRH
{
public:
    /*!
        Renders all road entities.
        This was used by R* because of corona reflections rendered after that.
    */
    static void RenderRoads();
    /*!
        Renders all cubemap entities.
    */
    static void RenderCubemapEntities();
    /*!
        Renders all non-road entities.
    */
    static void RenderEverythingBarRoads();
    /*!
        Renders all time dependant objects
    */
    static void RenderTOBJs();
    /*!
        Renders all shadow casters for i-th cascade
    */
    static void RenderShadowCascade( int i );
    /*!
        Deprecated, to be removed
    */
    static void ConstructShadowList();
    /*!
        Constructs entities lists to be rendered in current frame
    */
    static void ConstructRenderList();
    /*!
        Renders fading in all entities(LOD transitions)
    */
    static void RenderFadingInEntities();
    /*!
        Adds entity to main render list
    */
    static void AddEntityToRenderList( CEntity* entity, float renderDistance );
    /*!
        Adds entity to reflection list
    */
    static void AddEntityToReflectionList( CEntity* entity, float renderDistance );
    /*!
        Adds entity to shadow caster render list
    */
    static char AddEntityToShadowCasterList( CEntity* entity, float renderDistance, int shadowCascade );
    /*!
        Adds entity to LOD render list
    */
    static char AddToLodRenderList( CEntity* entity, float renderDistance );
    /*!
        Adds entity to shadow caster render list if it's inside light frustum
    */
    static void AddEntityToShadowCastersIfNeeded( CEntity* entity, bool checkBBox );
    /*!
        Checks if AABB is inside plane bounded volume
    */
    static bool IsAABBInsideBoundingVolume( RwPlane * boundingPlanes, int planeCount, const RW::BBox& b );
    /*!
        Calculates entity visibility based on flags and distance
    */
    static RendererVisibility SetupEntityVisibility( CEntity* entity, float* renderDistance );
    /*!
        Calculates BIG building entity visibility based on flags and distance
    */
    static RendererVisibility SetupBigBuildingVisibility( CEntity* entity, float* renderDistance );

    /*!
        Calculates map entity visibility
    */
    static RendererVisibility SetupMapEntityVisibility( CEntity *entity, CBaseModelInfo *mapInfo, float renderDistance, char a4 );
    /*!
        Renders one road entity
    */
    static void RenderOneRoad( CEntity* );
    /*!
        Renders one non-road entity
    */
    static void RenderOneNonRoad( CEntity* );
    /*!
        Checks if entity is culled by tunnel
    */
    static bool IsCulledByTunnel( CEntity* );
    /*!
        Scans sector for entities
    */
    static void ScanSectorList( int x, int y );
    /*!
        Scans entity pointer list
    */
    static void ScanPtrList( CPtrList* ptrList, bool loadIfRequired );
    /*!
        Scans entity pointer list for shadows
    */
    static void ScanPtrListForShadows( CPtrList* ptrList, bool loadIfRequired );
    /*!
        Scans entity pointer list for reflections
    */
    static void ScanPtrListForReflections( CPtrList* ptrList, bool loadIfRequired );
    /*!
        Scans sector for BIG buildings
    */
    static void ScanBigBuildingList( int x, int y );
    /*!
        Scans sector for reflection entities
    */
    static void ScanSectorListForReflections( int x, int y );
    /*!
        Scans sector for shadow casting entities
    */
    static void ScanSectorListForShadowCasters( int x, int y );
    /*!
        Prepares render entiy lists before rendering
    */
    static void PreRender();
    /*!
        Cleans up LOD render lists
    */
    static void ProcessLodRenderLists();
    /*!
        Deprecated, to be removed
    */
    static void ScanWorld( RwCamera* camera, RwV3d* gameCamPos, float shadowStart, float shadowEnd );
    /*!
        Scans world for entities
    */
    static void ScanWorld();
    /*!
        Resets LOD render lists
    */
    static void ResetLodRenderLists();
    /*!
        Renders shadow caster entity, skips effect rendering.
    */
    static void RenderShadowCasterEntity( CEntity* );
    /*!
        Renders reflection entity, skips effect rendering.
    */
    static void RenderReflectionEntity( CEntity* );
    /*!
        Returns base of LOD render list
    */
    static sLodListEntry *GetLodRenderListBase();
    static sLodListEntry *GetLodDontRenderListBase();
    static void CalculateShadowBoundingPlanes( int shadowCascade );
    /*!
        Scans for shadows in sector, multithreading experiment.
    */
    static void ScanShadowsMT( const int& x, const int& cs_y );
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
    static RwPlane ms_aShadowCasterBoundingPlanes[4][5];
    //static CBaseModelInfo **ms_modelInfoPtrs;
    static bool TOBJpass;
    static std::list<CEntity*> ms_aVisibleEntities;
    static std::list<CEntity*> ms_aVisibleLods;
    static std::vector<CEntity*> ms_aVisibleShadowCasters[4];
    static std::vector<CEntity*> ms_aVisibleReflectionObjects;
};

