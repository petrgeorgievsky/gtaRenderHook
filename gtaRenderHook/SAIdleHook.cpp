#include "stdafx.h"
#include "SAIdleHook.h"
#include "RwD3D1XEngine.h"
#include "DeferredRenderer.h"
#include "ShadowRenderer.h"
#include "Renderer.h"
#include "D3D1XStateManager.h"
#include "VoxelOctreeRenderer.h"
#include "LightManager.h"
#include "HDRTonemapping.h"
#include "CustomCarFXPipeline.h"
#include "CustomBuildingPipeline.h"
#include "SettingsHolder.h"
#include "CWaterLevel.h"
#include "CubemapReflectionRenderer.h"
#include "RwMethods.h"
#include <mutex>
#include "AntTweakBar.h"
#include "DebugRendering.h"
#include "D3DRenderer.h"
#include "DebugBBox.h"
#include <game_sa\CPointLights.h>
#include <game_sa\CFont.h>
#include <game_sa\CWorld.h>
#include <game_sa\CScene.h>
#include <game_sa\CClock.h>

//#include <plugin.h>
//#include <plugin_sa\game_sa\CFont.h>

/* GTA Definitions TODO: Move somewhere else*/
#define g_breakMan ((void *)0xBB4240)		// Break manager


#define CCullZones__FindTunnelAttributesForCoors(coords) ((int (__cdecl *)(CVector*))0x72D9F0)(coords)
#define CStreaming__StartRenderEntities() ((void (__cdecl *)())0x4096C0)()

#define RenderPedWeapons() ((void (__cdecl *)())0x732F30)()
#define sub_5071D0(audio) ((bool(__thiscall *)(CAudioEngine *))0x5071D0)(audio)
// Font methods, used to display preformance counters
#define CFont__PrintString(x,y,text) ((void(__cdecl *)(float, float, const char *))0x71A700)(x,y,text)
#define CFont__SetFontStyle(style) ((void(__cdecl *)(unsigned char))0x719490)(style)
#define CFont__SetColor(color) ((void(__cdecl *)(RwRGBA))0x719430)(color)
// Renderer methods, probably will end up rewriten
#define CRenderer__ProcessLodRenderLists() ((void (__cdecl *)())0x553770)()
// B7CB49     ; CTimer::m_UserPause
#define CTimer__m_UserPause (*(bool *)0xB7CB49)	
#define CPostEffects__m_bDisableAllPostEffect (*(bool *)0xC402CF)	

int drawCallCount=0;
void RenderEntity2dfx(CEntity* e);
void *CreateEntity2dfx(void* e);
TwBar* CSAIdleHook::m_MainTWBAR;
float CSAIdleHook::m_fShadowDNBalance = 1.0;
void CSAIdleHook::Patch()
{

	//RedirectCall(0x748D9B, GameLoop);
	RedirectCall(0x5343B2, RenderEntity2dfx);
	RedirectCall(0x6FECF0, CreateEntity2dfx);//createroadsign
	RedirectCall(0x53ECBD, Idle);
}

/*
void TW_CALL SetShadowSizeCallback(const void *value, void *clientData)
{
	gShadowSettings.Size = *(int*)value;
	g_pDeferredRenderer->m_pShadowRenderer->m_bRequiresReloading = true;
}
void TW_CALL GetShadowSizeCallback(void *value, void *clientData)
{
	*(int*)value = gShadowSettings.Size;
}*/

void TW_CALL SaveDataCallBack(void *value)
{
	SettingsHolder::Instance.SaveSettings();
}

void TW_CALL ReloadDataCallBack(void *value)
{
	SettingsHolder::Instance.ReloadFile();
}

bool g_bDrawTweakBar = false;

void CSAIdleHook::Idle(void *Data)
{
	if (m_MainTWBAR == nullptr) {
		m_MainTWBAR = TwNewBar("Settings");

		gShadowSettings.InitGUI(m_MainTWBAR);
		gTonemapSettings.InitGUI(m_MainTWBAR);
		gDeferredSettings.InitGUI(m_MainTWBAR);

		TwAddButton(m_MainTWBAR, "Save", SaveDataCallBack, nullptr, "");
		TwAddButton(m_MainTWBAR, "Reload", ReloadDataCallBack, nullptr, "");
	}
	// Update timers
	TimeUpdate();
	// Init 2D stuff per frame.
	InitPerFrame2D();
	// Update game processes
	GameUpdate();
	UpdateShadowDNBalance();
	// Update lighting
	LightUpdate();

	g_pDeferredRenderer->QueueTextureReload();
	CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;
	if (dxEngine->m_bScreenSizeChanged || g_pDeferredRenderer->m_pShadowRenderer->m_bRequiresReloading) {
		dxEngine->ReloadTextures();
		dxEngine->m_bScreenSizeChanged = false;
		g_pDeferredRenderer->m_pShadowRenderer->m_bRequiresReloading = false;
	}
	if (!Data)
		return;
	PrepareRwCamera();
	if (!RsCameraBeginUpdate(Scene.m_pRwCamera))
		return;
	if (!FrontEndMenuManager->m_bMenuActive /*&& !CCamera__GetScreenFadeStatus(TheCamera) == 2*/) {
		RenderInGame();
		if(g_bDrawTweakBar)
			TwDraw();
	}
	RenderHUD();
}

void CSAIdleHook::UpdateShadowDNBalance()
{
	float currentMinutes = (CClock::ms_nGameClockMinutes + 60 * CClock::ms_nGameClockHours) + CClock::ms_nGameClockSeconds * 0.016666668;
	if (currentMinutes < 360.0) {
		m_fShadowDNBalance = 1.0;
		return;
	}
	if (currentMinutes < 420.0)
	{
		m_fShadowDNBalance = (420.0 - currentMinutes) / 60.0;
		return;
	}
	// 
	if (currentMinutes < 1140.0)
	{
		m_fShadowDNBalance = 0.0;
		return;
	}
	if (currentMinutes >= 1200.0)
		m_fShadowDNBalance = 1.0;
	else
		m_fShadowDNBalance = 1.0 - (1200.0 - currentMinutes) / 60.0;
}

// Render one game frame
void CSAIdleHook::RenderInGame()
{
	CTimer shadowTimer("Shadow Time");
	CTimer deferredTimer("Deferred Time");
	CTimer scanTimer("Renderlist construction time");
	CTimer gameTimer("Game Time");
	
	PrepareRenderStuff();

	DefinedState();
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATESTENCILENABLE, 0);

	RwCameraSetFarClipPlane(Scene.m_pRwCamera, CTimeCycle::m_CurrentColours.m_fFarClip);

	Scene.m_pRwCamera->fogPlane = CTimeCycle::m_CurrentColours.m_fFogStart;
	
	// Game variables initialization
	const auto sunDirs = reinterpret_cast<RwV3d*>(0xB7CA50); // Sun direction table pointer(sun directions is always the same)
	const auto curr_sun_dir = *reinterpret_cast<int*>(0xB79FD0); // Current sun direction id
	if (sunDirs) {
		const auto sundir = &sunDirs[curr_sun_dir];
		g_pStateMgr->SetSunDir(sundir, m_fShadowDNBalance);
	}
	g_pStateMgr->SetFogStart(CTimeCycle::m_CurrentColours.m_fFogStart);
	g_pStateMgr->SetFogRange(CTimeCycle::m_CurrentColours.m_fFarClip - CTimeCycle::m_CurrentColours.m_fFogStart);
	g_shaderRenderStateBuffer.vSkyLightCol = {	CTimeCycle::m_CurrentColours.m_nSkyTopRed / 255.0f,
										CTimeCycle::m_CurrentColours.m_nSkyTopGreen / 255.0f,
										CTimeCycle::m_CurrentColours.m_nSkyTopBlue / 255.0f,1.0f };
	g_shaderRenderStateBuffer.vHorizonCol = { CTimeCycle::m_CurrentColours.m_nSkyBottomRed / 255.0f,
									CTimeCycle::m_CurrentColours.m_nSkyBottomGreen / 255.0f,
									CTimeCycle::m_CurrentColours.m_nSkyBottomBlue / 255.0f,1.0f };
	g_shaderRenderStateBuffer.vSunColor = {	CTimeCycle::m_CurrentColours.m_nSunCoreRed / 255.0f,
									CTimeCycle::m_CurrentColours.m_nSunCoreGreen / 255.0f,
									CTimeCycle::m_CurrentColours.m_nSunCoreBlue / 255.0f, 4.5f/*Timecycle->m_fCurrentSpriteBrightness */};
	g_shaderRenderStateBuffer.vWaterColor = { CTimeCycle::m_CurrentColours.m_fWaterRed / 255.0f,
									CTimeCycle::m_CurrentColours.m_fWaterGreen / 255.0f ,
									CTimeCycle::m_CurrentColours.m_fWaterBlue / 255.0f,
									CTimeCycle::m_CurrentColours.m_fWaterAlpha / 255.0f };
	g_shaderRenderStateBuffer.vGradingColor0 = {	CTimeCycle::m_CurrentColours.m_fPostFx1Red / 255.0f,
										CTimeCycle::m_CurrentColours.m_fPostFx1Green / 255.0f ,
										CTimeCycle::m_CurrentColours.m_fPostFx1Blue / 255.0f,
										CTimeCycle::m_CurrentColours.m_fPostFx1Alpha / 255.0f };
	g_shaderRenderStateBuffer.vGradingColor1 = {	CTimeCycle::m_CurrentColours.m_fPostFx2Red / 255.0f,
										CTimeCycle::m_CurrentColours.m_fPostFx2Green / 255.0f ,
										CTimeCycle::m_CurrentColours.m_fPostFx2Blue / 255.0f,
										CTimeCycle::m_CurrentColours.m_fPostFx2Alpha / 255.0f };
	g_shaderRenderStateBuffer.fFarClip = Scene.m_pRwCamera->farPlane;
	g_shaderRenderStateBuffer.fTimeStep += 0.08f * CTimer__ms_fTimeStep * 0.04f;
	if (g_shaderRenderStateBuffer.fTimeStep > 3.14f * 2)
		g_shaderRenderStateBuffer.fTimeStep -= 3.14f * 2;

	// First forward pass(clouds, sky etc.)
	RenderForwardBeforeDeferred();

	g_pDeferredRenderer->m_pShadowRenderer->m_bShadowsRendered = false;
	if (sunDirs && !CGame__currArea&&(m_fShadowDNBalance < 1.0))
		PrepareRealTimeShadows(sunDirs[curr_sun_dir]);
	
	DebugRendering::ResetList();
	//ConstructCustomRenderList(Scene.m_pRwCamera, Scene.m_pRwCamera->nearPlane, Scene.m_pRwCamera->farPlane);
	
	//CRenderer::PreRender();
	

	RwCameraEndUpdate(Scene.m_pRwCamera);
	drawCallCount = 0;

	// Render custom preprocess effects - shadows and voxel GI(disabled atm)

	m_uiDeferredStage = 5;
	//
	scanTimer.Start();
	CRenderer::ConstructRenderList();
	scanTimer.Stop();
	//CRenderer__ConstructRenderList();
	CRenderer__PreRender();
	CWorld::ProcessPedsAfterPreRender();
	//g_pDeferredRenderer->m_pCubemapRenderer->RenderToCubemap(RenderForward);

	shadowTimer.Start();

	//CRenderer::ConstructShadowList();

	if (sunDirs && !CGame__currArea&&m_fShadowDNBalance < 1.0) // Render shadows only if we are not inside interiors
		RenderRealTimeShadows(sunDirs[curr_sun_dir]);

	shadowTimer.Stop();

	//RenderVoxelGI();

	// Do standard gta culling.
	g_pDebug->printMsg("Culling events: begin", 1);

	
	//CRenderer__ConstructRenderList();
	//ConstructCustomRenderList(Scene.curCamera, Scene.curCamera->nearPlane, Scene.curCamera->farPlane);
	

	g_pDebug->printMsg("Culling events: end", 1);

	RwCameraBeginUpdate(Scene.m_pRwCamera);

	deferredTimer.Start();
	g_pCustomCarFXPipe->ResetAlphaList();
	g_pCustomBuildingPipe->ResetAlphaList();
	// Render scene to geometry buffers.
	g_pDeferredRenderer->RenderToGBuffer(RenderDeferred);
	
	// Reset renderstates and disable Z-Test
	DefinedState();
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEZTESTENABLE, 0);

	// Render deferred shading
	CD3DRenderer* renderer = static_cast<CRwD3D1XEngine*>(g_pRwCustomEngine)->getRenderer();
	renderer->BeginDebugEvent(L"Deferred composition pass");
	g_pDeferredRenderer->RenderOutput();
	renderer->EndDebugEvent();
	deferredTimer.Stop();

	// Enable Z-Test and render alpha entities
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEZTESTENABLE, 1);
	m_uiDeferredStage = 0;
	RenderForwardAfterDeferred();
	
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEZTESTENABLE, 0);
	renderer->BeginDebugEvent(L"Tonemapping pass");
	g_pDeferredRenderer->RenderTonemappedOutput(); //TODO fix
	renderer->EndDebugEvent();
	DebugRendering::Render();

	
	/*int Render2dStuffAddress = *(DWORD *)0x53EB13 + 0x53EB12 + 5;
	((int (__cdecl *)())Render2dStuffAddress)();*/
	Render2dStuff();
	// Render preformance counters if required.
	if (gDebugSettings.ShowPreformanceCounters) {
		CFont::SetFontStyle(eFontStyle::FONT_SUBTITLES);
		CRGBA color{ 255,255,255,255 };//pi/360
		//CFont::SetAlignment(eFontAlignment::ALIGN_RIGHT);
		//CFont::SetScale(0.5, 0.5);
		CFont::SetColor(color);
		float FontXPos = Scene.m_pRwCamera->frameBuffer->width - 20.0f;
		float FontYPos = Scene.m_pRwCamera->frameBuffer->height - 20.0f;

		
		CFont::PrintString(FontXPos, FontYPos - 450.0f, (char*)("Draw call count: " + to_string(drawCallCount)).c_str());
		CFont::PrintString(FontXPos, FontYPos - 400.0f, (char*)("Visible Entity count: " + to_string(CRenderer::ms_nNoOfVisibleEntities)).c_str());
		CFont::PrintString(FontXPos, FontYPos - 350.0f, (char*)("SC 3 Entity count: " + to_string(CRenderer::ms_aVisibleShadowCasters[3].size())).c_str());
		CFont::PrintString(FontXPos, FontYPos - 300.0f, (char*)("SC 2 Entity count: " + to_string(CRenderer::ms_aVisibleShadowCasters[2].size())).c_str());
		CFont::PrintString(FontXPos, FontYPos - 250.0f, (char*)("SC 1 Entity count: " + to_string(CRenderer::ms_aVisibleShadowCasters[1].size())).c_str());
		CFont::PrintString(FontXPos, FontYPos - 200.0f, (char*)("SC 0 Entity count: " + to_string(CRenderer::ms_aVisibleShadowCasters[0].size())).c_str());
			 
		CFont::PrintString(FontXPos, FontYPos - 150.0f, (char*)scanTimer.GetTimerResult().c_str());
		CFont::PrintString(FontXPos, FontYPos - 100.0f, (char*)shadowTimer.GetTimerResult().c_str());
		CFont::PrintString(FontXPos, FontYPos - 50.0f, (char*)deferredTimer.GetTimerResult().c_str());
	}

}

void CSAIdleHook::RenderHUD()
{
	if (FrontEndMenuManager->m_bMenuActive)
		DrawMenuManagerFrontEnd(FrontEndMenuManager); // CMenuManager::DrawFrontEnd
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

	DoFade();
	Render2dStuffAfterFade();

	DoRWStuffEndOfFrame();
}

void CSAIdleHook::RenderForwardBeforeDeferred()
{
}

void CSAIdleHook::RenderForwardAfterDeferred()
{
	g_pCustomCarFXPipe->RenderAlphaList();
	g_pCustomBuildingPipe->RenderAlphaList();

	g_pDeferredRenderer->SetNormalDepthRaster();
	g_pDeferredRenderer->SetPreviousNonTonemappedFinalRaster();
	g_pDeferredRenderer->m_pShadowRenderer->SetShadowBuffer();
	CWaterLevel::RenderWater();
	DefinedState();
	CPostEffects__m_bDisableAllPostEffect = true;
	// Render effects and 2d stuff
	_RenderEffects();
	DefinedState();
	//RenderGrass();
}

// Renders all entities in deferred render path
void CSAIdleHook::RenderDeferred()
{
	RenderFadingInUnderwaterEntities();
	CVisibilityPlugins__RenderFadingInEntities();
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATECULLMODE, rwCULLMODECULLBACK);
	CRenderer::RenderRoads([](void* e) { return true; });
	CRenderer::RenderEverythingBarRoads();
	BreakManager_c__Render(g_breakMan, 0);
	 // CRenderer::RenderFadingInUnderwaterEntities
	CVisibilityPlugins__RenderWeaponPedsForPC();
	CRenderer::RenderTOBJs();
	//CWaterLevel::RenderSeaBed();
}

void CSAIdleHook::RenderForward()
{
	g_pRwCustomEngine->RenderStateSet(rwRENDERSTATECULLMODE, rwCULLMODECULLNONE);
	CRenderer::RenderRoads([](void* e) { return true; });
}

void CSAIdleHook::PrepareRealTimeShadows(const RwV3d &sundir)
{
	g_pDebug->printMsg("Shadow PrePass: begin", 1);

	auto shadowRenderer = g_pDeferredRenderer->m_pShadowRenderer;

	shadowRenderer->CalculateShadowDistances(Scene.m_pRwCamera->nearPlane, Scene.m_pRwCamera->farPlane);
	// Render cascades
	for (int i = 0; i < 4; i++)
		shadowRenderer->DirectionalLightTransform(Scene.m_pRwCamera, sundir, i);
	
	g_pDebug->printMsg("Shadow PrePass: end", 1);
}
void CSAIdleHook::RenderRealTimeShadows(const RwV3d &sundir)
{
	g_pDebug->printMsg("Shadow Pass: begin", 1);

	auto shadowRenderer = g_pDeferredRenderer->m_pShadowRenderer;
	m_uiDeferredStage = 2;
	RwCameraClear(shadowRenderer->m_pShadowCamera, gColourTop, rwCAMERACLEARZ);

	// Render cascades
	for (int i = 0; i < 4; i++) {

		shadowRenderer->RenderShadowToBuffer(i, [](int k) { RenderDeferred(); });
	}
	shadowRenderer->m_bShadowsRendered = true;
	g_pDebug->printMsg("Shadow Pass: end", 1);
}

void RenderEntity2dfx(CEntity* e) { }

void* CreateEntity2dfx(void* e) {
	return reinterpret_cast<void*>(0xC3EF84);
}



void CopyViewMatrix(RwMatrix* viewTransformRef,RwFrame* camframe) {
	RwMatrixInvert(viewTransformRef, RwFrameGetLTM(camframe));
	viewTransformRef->right.x = -viewTransformRef->right.x;
	viewTransformRef->up.x = -viewTransformRef->up.x;
	viewTransformRef->at.x = -viewTransformRef->at.x;
	viewTransformRef->pos.x = -viewTransformRef->pos.x;
	viewTransformRef->flags = 0;
	viewTransformRef->pad1 = 0;
	viewTransformRef->pad2 = 0;
	viewTransformRef->pad3 = 0x3F800000;
}
//typedef std::chrono::high_resolution_clock hr_clock;

void CSAIdleHook::RenderVoxelGI()
{
	auto s_cam_frame = RwCameraGetFrame(CVoxelOctreeRenderer::m_pVoxelCamera);
	auto campos=RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame(Scene.m_pRwCamera)));
	/*if (FindPlayerPos(&cpos, 0) == nullptr)
		Campos = RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame(Scene.curCamera)));
	else
		Campos = &cpos;*/
	RwFrameTranslate(s_cam_frame, campos, rwCOMBINEREPLACE);
	//g_pRwCustomEngine->RenderStateSet(rwRENDERSTATECULLMODE, rwCULLMODECULLNONE);
	CVoxelOctreeRenderer::CleanVoxelOctree();
	
	for (size_t i = 1; i < 4; i++)
	{
		m_uiDeferredStage = 3;
		CVoxelOctreeRenderer::SetVoxelLODSize(i);
		/*CopyViewMatrix(&CVoxelOctreeRenderer::cb_voxel.View[0], RwCameraGetFrame(CVoxelOctreeRenderer::m_pVoxelCamera));
		CameraRotate(CVoxelOctreeRenderer::m_pVoxelCamera, nullptr, 180);
		CopyViewMatrix(&CVoxelOctreeRenderer::cb_voxel.View[1], RwCameraGetFrame(CVoxelOctreeRenderer::m_pVoxelCamera));
		CameraRotate(CVoxelOctreeRenderer::m_pVoxelCamera, nullptr, -90);
		CopyViewMatrix(&CVoxelOctreeRenderer::cb_voxel.View[2], RwCameraGetFrame(CVoxelOctreeRenderer::m_pVoxelCamera));
		CameraRotate(CVoxelOctreeRenderer::m_pVoxelCamera, nullptr, 180);
		CopyViewMatrix(&CVoxelOctreeRenderer::cb_voxel.View[3], RwCameraGetFrame(CVoxelOctreeRenderer::m_pVoxelCamera));
		CameraRotate2(CVoxelOctreeRenderer::m_pVoxelCamera, nullptr, 90);
		CopyViewMatrix(&CVoxelOctreeRenderer::cb_voxel.View[4], RwCameraGetFrame(CVoxelOctreeRenderer::m_pVoxelCamera));
		CameraRotate2(CVoxelOctreeRenderer::m_pVoxelCamera, nullptr, 180);
		CopyViewMatrix(&CVoxelOctreeRenderer::cb_voxel.View[5], RwCameraGetFrame(CVoxelOctreeRenderer::m_pVoxelCamera));
		*/
		CVoxelOctreeRenderer::RenderToVoxelOctree(RenderDeferred, i);

		g_pDeferredRenderer->m_pShadowRenderer->SetShadowBuffer();
		CLightManager::SortByDistance(*campos);

		m_uiDeferredStage = 4;
		CVoxelOctreeRenderer::InjectRadiance(g_pDeferredRenderer->m_pShadowRenderer->m_pShadowCamera->zBuffer, CRenderer::RenderTOBJs, i-1);

	}
	CVoxelOctreeRenderer::FilterVoxelOctree();
}

void CSAIdleHook::PrepareRenderStuff()
{
	RwV2d mousePos;
	mousePos.x = RsGlobal.maximumWidth * 0.5f;
	mousePos.y = RsGlobal.maximumHeight * 0.5f;
	POINT pMousePos;
	g_bDrawTweakBar = GetKeyState(VK_F12) & 1;
	GetCursorPos(&pMousePos);
	if(!g_bDrawTweakBar)
		RsMouseSetPos(&mousePos);
	else
		TwMouseMotion(pMousePos.x, pMousePos.y);
	ShowCursor(g_bDrawTweakBar);
	CTimer__m_UserPause = g_bDrawTweakBar;
	//CRenderer__ConstructRenderList();
	//CRenderer__PreRender();           // CRenderer::PreRender
	//CWorld__ProcessPedsAfterPreRender();
}

void CSAIdleHook::Render2dStuffAfterFade()
{
	CHud__DrawAfterFade();
	CMessages__Display(0); // CMessages::Display
	CFont__RenderFontBuffer();     // CFont::DrawFonts
	if (CCredits__bCreditsGoing)
	{
		if (!FrontEndMenuManager->m_bMenuActive)
			CCredits__Render();       // CCredits::Render
	}
}

void CSAIdleHook::DoRWStuffEndOfFrame()
{
	DebugDisplayTextBuffer();
	FlushObrsPrintfs();
	RwCameraEndUpdate(Scene.m_pRwCamera);
	RsCameraShowRaster(Scene.m_pRwCamera);
}

void CSAIdleHook::PrepareRwCamera()
{
	CDraw__CalculateAspectRatio(); // CDraw::CalculateAspectRatio
	CameraSize(Scene.m_pRwCamera, 0, tanf(CDraw__ms_fFOV*( 3.1415927f / 360.0f)), CDraw__ms_fAspectRatio);
	CVisibilityPlugins__SetRenderWareCamera(Scene.m_pRwCamera); // CVisibilityPlugins::SetRenderWareCamera
	RwCameraClear(Scene.m_pRwCamera, gColourTop, rwCAMERACLEARZ);
}


void CSAIdleHook::LightUpdate()
{
	CPointLights::NumLights = 0;
	CLightManager::Reset();
	SetLightsWithTimeOfDayColour(Scene.m_pRpWorld);
}

void CSAIdleHook::GameUpdate()
{
	CGame__Process();
	CAudioEngine__Service(AudioEngine);
}

void CSAIdleHook::InitPerFrame2D()
{
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
}

void CSAIdleHook::TimeUpdate()
{
	while (CTimer__GetTimeMillisecondsFromStart() - CGame__TimeMillisecondsFromStart < 1)
		;
	CGame__TimeMillisecondsFromStart = CTimer__GetTimeMillisecondsFromStart();
	CTimer__Update();
}

// todo: implement sa culling and in sky checks, move out of CSAIdleHook
void CSAIdleHook::ConstructCustomRenderList(const RwV3d & viewPos, const RwV3d & viewFrontVec, const float& nearDist, const float& farDist,RwCamera* camera)
{
	CRenderer__ms_bRenderTunnels		= true;
	CRenderer__ms_bRenderOutsideTunnels = true;
	CRenderer::ms_bInTheSky				= false;
	CRenderer::ms_nNoOfVisibleEntities		= 0;
	CRenderer::ms_nNoOfVisibleLods			= 0;
	CRenderer::ms_nNoOfInVisibleEntities	= 0;
	//CRenderer::ms_vecCameraPosition			= viewPos;
	if(viewFrontVec.y==0&&viewFrontVec.x==0)
		CRenderer::ms_fCameraHeading = 0.0f;
	else
		CRenderer::ms_fCameraHeading			= atan2(viewFrontVec.y,-viewFrontVec.x);
	CRenderer__ms_fFarClipPlane				= farDist;
	auto sceneCamMatrix = RwFrameGetMatrix(RwCameraGetFrame(Scene.m_pRwCamera));
	auto camPos = *RwMatrixGetPos(sceneCamMatrix);
	CRenderer::ScanWorld(camera,(RwV3d*)&camPos,nearDist, farDist);
	//CRenderer::ms_pLodRenderList = (CEntity*)(0xC8E0C8+0x18);
	CRenderer__ProcessLodRenderLists();
	CStreaming__StartRenderEntities();
}


// Construct's object list to render for selected camera frustum.
void CSAIdleHook::ConstructCustomRenderList(RwCamera* cam, const float& nearDist, const float& farDist) {
	auto camMatrix = RwFrameGetMatrix(RwCameraGetFrame(cam));
	auto camPos = *RwMatrixGetPos(camMatrix);
	auto camAt = *RwMatrixGetAt(camMatrix);

	ConstructCustomRenderList(camPos, camAt, nearDist, farDist,cam);

}
