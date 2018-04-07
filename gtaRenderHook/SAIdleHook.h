#pragma once
#include <game_sa\CCamera.h>
#include <game_sa\CTimeCycle.h>
#include <game_sa\CMenuManager.h>
#include "AntTweakBar.h"

// gta functions

#define CTimer__GetCurrentTimeInCycles() ((size_t  (__cdecl *)())0x561A80)()
#define CTimer__GetCyclesPerMillisecond() ((size_t  (__cdecl *)())0x561A40)()
#define CTimer__GetTimeMillisecondsFromStart() ((size_t  (__cdecl *)())0x53BAD0)()
#define CGame__TimeMillisecondsFromStart (*(size_t*)0xB72CA8)
#define timerDef (*(size_t*)0xB7CB2C)
#define RwInitialized (*(int*)0xC920E8)
#define IsForegroundApp (*(int*)0x8D621C)


#define CTimer__Update() ((void (__cdecl *)())0x561B10)()
#define CSprite2d__InitPerFrame() ((void (__cdecl *)())0x727350)()

#define CFont__InitPerFrame() ((void (__cdecl *)())0x719800)()
#define CGame__Process() ((void (__cdecl *)())0x53BEE0)()
#define CAudioEngine__Service(audio) ((void (__thiscall *)(CAudioEngine *))0x507750)(audio)
#define SetLightsWithTimeOfDayColour(world) ((void (__cdecl *)(RpWorld *))0x7354E0)(world)
#define CreateShadowManagerShadows(shadowManager) ((void (__thiscall *)(CShadowManager *))0x706AB0)(shadowManager)
#define RenderSceneGeometryToMirror() ((void (__cdecl *)())0x727140)()
#define DoRWStuffStartOfFrame_Horizon(topR, topG, topB, bottomR, bottomG, bottomB, A) ((unsigned int (__cdecl *)(unsigned short, unsigned short, \
	unsigned short, unsigned short, unsigned short, unsigned short, unsigned short))0x53D7A0)(topR, topG, topB, bottomR, bottomG, bottomB, A)
#define DefinedState() ((void (__cdecl *)())0x734650)()
#define CCamera__GetScreenFadeStatus(camera) ((signed int (__thiscall *)(CCamera *))0x50AE20)(camera)
#define CDraw__CalculateAspectRatio() ((double (__cdecl *)())0x6FF420)()
#define CVisibilityPlugins__SetRenderWareCamera(camera) ((void (__cdecl *)(RwCamera *))0x7328C0)(camera)
#define GetTimeFromRenderStart() ((DWORD (__cdecl *)())0x561A80)()
#define GetTimerDivider() ((int (__cdecl *)())0x561A40)()
#define RwTextureSetAutoMipmapping(enable) ((signed int (__cdecl*)(int))0x7F3560)(enable)
#define RsMouseSetPos(pos) ((int (__cdecl *)(RwV2d *)) 0x6194A0)(pos)

#define CRenderer__ConstructRenderList() ((void (__cdecl *)())0x5556E0)()
#define CRenderer__PreRender() ((void (__cdecl *)())0x553910)()
#define CWorld__ProcessPedsAfterPreRender() ((void (__cdecl *)())0x563430)()
#define _RenderScene() ((void (__cdecl *)())0x53DF40)()
#define CVisibilityPlugins__RenderWeaponPedsForPC() ((void (__cdecl *)())0x732F30)()
#define sub_53E8D0(unk) ((void (__thiscall *)(void *))0x53E8D0)(unk)
#define _RenderEffects() ((void (__cdecl *)())0x53E170)()
#define SetCameraMotionBlurAlpha(camera, alpha) ((void (__thiscall *)(CCamera *, unsigned char))0x50BF80)(camera, alpha)
#define RenderCameraMotionBlur(camera) ((void (__thiscall *)(CCamera *))0x50B8F0)(camera)
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
#define UpdateSunLightForCustomRenderingPipeline() ((void (__cdecl *)())0x5D5B10)()
#define CRenderer__RenderRoads() ((void (__cdecl *)())0x553A10)()
#define RenderCoronasReflections() ((void (__cdecl *)())0x6FB630)()
#define CRenderer__RenderEverythingBarRoads() ((void (__cdecl *)())0x553AA0)()
#define BreakManager_c__Render(unk1, unk2) ((void (__thiscall *)(void *, int))0x59E6A0)(unk1, unk2)
#define RenderFadingInUnderwaterEntities() ((void (__cdecl *)())0x553220)()

#define CRenderer__RenderFadingInEntities() ((void (__cdecl *)())0x5531E0)()
#define CVisibilityPlugins__RenderFadingInEntities() ((void (__cdecl *)())0x733F10)()
#define sub_707F40() ((void (__cdecl *)())0x707F40)()
#define RenderStaticShadows() ((void (__cdecl *)())0x708300)()
#define RenderStoredShadows() ((void (__cdecl *)())0x70A960)()
#define RenderGrass() ((void (__cdecl *)())0x5DBAE0)()
#define sub_7154B0() ((void (__cdecl *)())0x7154B0)()
#define RenderRainStreaks() ((void (__cdecl *)())0x72AF70)()
//#define RenderSunReflection() ((void (__cdecl *)())0x6FBAA0)()
#define RenderStencil() ((unsigned char (__cdecl *)())0x7113B0)()
#define FindPlayerPos(outvec,id) ((RwV3d* (__cdecl *)(RwV3d*, int))0x56E010)(outvec,id)


typedef void CAudioEngine;
typedef void CShadowManager;
//struct TwBar;
#define CCredits__bCreditsGoing (*(unsigned char *)0xC6E97C)
#define FrontEndMenuManager ((CMenuManager *)0xBA6748)
#define AudioEngine ((CAudioEngine *)0xB6BC90)

#define Timecycle ((CTimeCycleCurrent *)0xB7C4A0)
#define ShadowManager ((CShadowManager *)0xC40350)
#define CPointLights__NumLights (*(unsigned int *)0xC3F0D0)
#define CDraw__ms_fAspectRatio (*(float *)0xC3EFA4)
#define CDraw__ms_fFOV (*(float *)0x8D5038)
#define gCameraSeaDepth (*(float *)0xC8132C)
#define gOcclReflectionsState (*(unsigned int *)0xC7C724)
#define g_DirLight ((RwV3d *)0xB7CB14)
#define LastTickTime (*(unsigned int *)0xB72CA8)
#define CWeather__LightningFlash (*(unsigned char *)0xC812CC)
#define CReplay__Mode (*(unsigned char *)0xA43088)
//
class CSAIdleHook
{
public:
	static void Patch();
	static void GameLoop();
	static void RenderLoop();
	static void UpdateLoop();
	static void Idle(void *Data);

	static void RenderVoxelGI();

	static void RenderRealTimeShadows(const RwV3d &sundir);
	static void PrepareRealTimeShadows(const RwV3d &sundir);

	static void PrepareRenderStuff();

	static void RenderHUD();

	static void Render2dStuffAfterFade();

	static void DoRWStuffEndOfFrame();

	static void PrepareRwCamera();

	static void LightUpdate();

	static void GameUpdate();

	static void InitPerFrame2D();

	static void TimeUpdate();

	static void ConstructCustomRenderList(const RwV3d& viewPos, const RwV3d &viewFrontVec, const float& nearDist, const float& farDist, RwCamera* camera);
	static void ConstructCustomRenderList(RwCamera* cam, const float& nearDist, const float& farDist);
	static void RenderDeferred();
	static void RenderForward();
	static void RenderEffects();
	static void RenderForwardBeforeDeferred();
	static void RenderForwardAfterDeferred();
	static void RenderEmissiveObjects();
	static void UpdateShadowDNBalance();
	static void RenderInGame();
	static float m_fShadowDNBalance;
private:
	static std::thread renderThread;
	static std::thread updateThread;
	static TwBar* m_MainTWBAR;
};

