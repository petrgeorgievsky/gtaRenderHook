#pragma once
#include <game_sa\CCamera.h>
#include <game_sa\CTimeCycle.h>
#include <game_sa\CMenuManager.h>
#include "AntTweakBar.h"

// gta functions

#define CTimer__GetTimeMillisecondsFromStart() ((size_t  (__cdecl *)())0x53BAD0)()
#define CGame__TimeMillisecondsFromStart (*(size_t*)0xB72CA8)

#define CTimer__Update() ((void (__cdecl *)())0x561B10)()
#define CAudioEngine__Service(audio) ((void (__thiscall *)(CAudioEngine *))0x507750)(audio)

#define SetLightsWithTimeOfDayColour(world) ((void (__cdecl *)(RpWorld *))0x7354E0)(world)

#define DefinedState() ((void (__cdecl *)())0x734650)()
#define DefinedState2d() ((void (__cdecl *)())0x734750)()
#define CCamera__GetScreenFadeStatus(camera) ((signed int (__thiscall *)(CCamera *))0x50AE20)(camera)
#define CVisibilityPlugins__SetRenderWareCamera(camera) ((void (__cdecl *)(RwCamera *))0x7328C0)(camera)
#define RsMouseSetPos(pos) ((int (__cdecl *)(RwV2d *)) 0x6194A0)(pos)

#define CRenderer__ConstructRenderList() ((void (__cdecl *)())0x5556E0)()
#define CRenderer__PreRender() ((void (__cdecl *)())0x553910)()
#define CWorld__ProcessPedsAfterPreRender() ((void (__cdecl *)())0x563430)()
#define CVisibilityPlugins__RenderWeaponPedsForPC() ((void (__cdecl *)())0x732F30)()
#define _RenderEffects() ((void (__cdecl *)())0x53E170)()
#define Render2dStuff() ((int (__cdecl *)())0x53E230)()
#define DrawMenuManagerFrontEnd(menuManager) ((void (__thiscall *)(CMenuManager *))0x57C290)(menuManager)
#define DoFade() ((void (__cdecl *)())0x53E600)()
#define CHud__DrawAfterFade() ((void (__cdecl *)())0x58D490)()
#define CMessages__Display(priority) ((void (__cdecl *)(char))0x69EFC0)(priority)
#define CFont__RenderFontBuffer() ((int (__cdecl *)())0x71A210)()
#define CCredits__Render() ((void (__cdecl *)())0x5A87F0)()
#define DebugDisplayTextBuffer() ((void (__cdecl *)())0x532260)()
#define FlushObrsPrintfs() ((void (__cdecl *)())0x734640)()
#define DoRWRenderHorizon() ((void (__cdecl *)())0x7178F0)()
#define RenderClouds() ((void (__cdecl *)())0x713950)()
#define CRenderer__RenderRoads() ((void (__cdecl *)())0x553A10)()
#define RenderCoronasReflections() ((void (__cdecl *)())0x6FB630)()
#define CRenderer__RenderEverythingBarRoads() ((void (__cdecl *)())0x553AA0)()
#define BreakManager_c__Render(unk1, unk2) ((void (__thiscall *)(void *, int))0x59E6A0)(unk1, unk2)
//#define RenderFadingInUnderwaterEntities() ((void (__cdecl *)())0x553220)()

#define CRenderer__RenderFadingInEntities() ((void (__cdecl *)())0x5531E0)()
#define CVisibilityPlugins__RenderFadingInEntities() ((void (__cdecl *)())0x733F10)()
#define RenderStaticShadows() ((void (__cdecl *)())0x708300)()
#define RenderStoredShadows() ((void (__cdecl *)())0x70A960)()
#define RenderGrass() ((void (__cdecl *)())0x5DBAE0)()
#define RenderRainStreaks() ((void (__cdecl *)())0x72AF70)()
#define RenderStencil() ((unsigned char (__cdecl *)())0x7113B0)()
#define FindPlayerPos(outvec,id) ((RwV3d* (__cdecl *)(RwV3d*, int))0x56E010)(outvec,id)

typedef void CAudioEngine;
typedef void CShadowManager;
//struct TwBar;
#define CCredits__bCreditsGoing (*(unsigned char *)0xC6E97C)
//#define FrontEndMenuManager ((CMenuManager *)0xBA6748)
#define AudioEngine ((CAudioEngine *)0xB6BC90)

#define Timecycle ((CTimeCycleCurrent *)0xB7C4A0)
#define ShadowManager ((CShadowManager *)0xC40350)
#define CReplay__Mode (*(unsigned char *)0xA43088)
#define D3D9Device (*(IDirect3DDevice9 **)0xC97C28)
/*!
    GTA SA Idle hook, responsible for main game loop
*/
class CSAIdleHook
{
public:
    /*!
        Patches game memory, used to inject RenderHook methods in game.
    */
    static void Patch();
    /// Rendering on separate thread experiments, currently useless
    static void GameLoop();
    static void RenderLoop();
    static void UpdateLoop();
    /*!
        Idle method, responsible for everything in game from rendering to gameplay update
    */
    static void Idle( void *Data );
    /*!
        Experimental Voxel Global Illumination rendering(currently disabled)
    */
    static void RenderVoxelGI();
    /*!
        Renders all shadow objects to cascade shadow buffer for specified light direction
    */
    static void RenderRealTimeShadows( const RwV3d &sundir );
    /*!
        Prepares shadow matrices for each cascade for specified light direction
    */
    static void PrepareRealTimeShadows( const RwV3d &sundir );
    /*!
        Prepares mouse position and GUI overlay rendering
    */
    static void PrepareRenderStuff();
    /*!
        Renders fronted part of game
    */
    static void RenderHUD();
    /*!
        Renders text and credits(for some reason after in-game screen fade)
    */
    static void Render2dStuffAfterFade();
    /*!
        Prints debuging info and sends image to GPU
    */
    static void DoRWStuffEndOfFrame();
    /*!
        Prepares camera params
    */
    static void PrepareRwCamera();
    /*!
        Resets light for current frame and updates colors
    */
    static void LightUpdate();
    /*!
        Updates game and audio
    */
    static void GameUpdate();
    /*!
        Initializes 2D params for current frame
    */
    static void InitPerFrame2D();
    /*!
        Updates time counters and deltas
    */
    static void TimeUpdate();
    /*!
        Renders geometry in deferred render path
    */
    static void RenderDeferred();
    /*!
        Renders geometry in forward render path
    */
    static void RenderForward();
    /*!
        Renders effects(e.g. particles, decals etc.)
    */
    static void RenderEffects();
    /*!
        Render geometry in forward render path before deferred pass
    */
    static void RenderForwardBeforeDeferred();
    /*!
        Render geometry in forward render path after deferred pass
    */
    static void RenderForwardAfterDeferred();
    /*!
        Updates day-night balance, adjusted for shadows
    */
    static void UpdateShadowDNBalance();
    /*!
        Renders game
    */
    static void RenderInGame();

    static float m_fShadowDNBalance;
    static int m_nLastCascadeRenderCount;
private:
    static std::thread renderThread;
    static std::thread updateThread;
};

