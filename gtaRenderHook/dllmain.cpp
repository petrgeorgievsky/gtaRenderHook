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
#include "GTASAHooks.h"
#include "StreamingRH.h"
#include "SampHaxx.h"
#include "TemporalAA.h"
CDebug*				g_pDebug;
CIRwRenderEngine*	g_pRwCustomEngine;

// defines
// TODO: "DELET DIS" - Monika
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

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
)
{
    UNREFERENCED_PARAMETER( lpReserved );
    UNREFERENCED_PARAMETER( hModule );
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        // Load RenderHook settings
        SettingsHolder::Instance()->ReloadFile();
        // Init basic stuff
        g_pDebug = new CDebug( "debug.log" );
        g_pRwCustomEngine = new CRwD3D1XEngine( g_pDebug );

#if GTA_SA
        // Path rw/game stuff
        CRwGameHooks::Patch( CRwGameHooks::ms_rwPointerTableSA );
        CGTASAHooks::Patch();
        // Replace all pipelines(move to CRwGameHooks or CGTASAHooks)
        CCustomBuildingPipeline::Patch();
        CCustomBuildingDNPipeline::Patch();
        CCustomCarFXPipeline::Patch();
        SampHaxx::Patch();
#endif // GTA_SA

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

// TODO: move this crap out of here
RxD3D9InstanceData* GetModelsData( RxInstanceData * data )
{
    return reinterpret_cast<RxD3D9InstanceData*>( data + 1 );
}

RxD3D9InstanceData *GetModelsData2( RxD3D9ResEntryHeader *data )
{
    return reinterpret_cast<RxD3D9InstanceData *>( data + 1 );
}

RpMesh * GetMeshesData( RpD3DMeshHeader * data )
{
    return reinterpret_cast<RpMesh*>( data + 1 );
}