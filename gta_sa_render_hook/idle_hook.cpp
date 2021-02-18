#include "idle_hook.h"
#include "gta_sa_internal_classes/CColorSet.h"
#include "gta_sa_internal_classes/audioengine.h"
#include "gta_sa_internal_classes/font.h"
#include "gta_sa_internal_classes/game.h"
#include "gta_sa_internal_classes/hud.h"
#include "gta_sa_internal_classes/menumanager.h"
#include "gta_sa_internal_classes/renderer.h"
#include "gta_sa_internal_classes/rw_func_ptrs.h"
#include "gta_sa_internal_classes/scene.h"
#include "gta_sa_internal_classes/sprite2d.h"
#include "gta_sa_internal_classes/timer.h"
#include "renderloop.h"
#include <Engine/IRenderer.h>

#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <common_headers.h>
#include <render_client/render_client.h>

// static RwRGBA gColourTop = *reinterpret_cast<RwRGBA *>( 0xB72CA0 );
static HHOOK g_hImGuiHook;

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg,
                                               WPARAM wParam, LPARAM lParam );
/*
LRESULT CALLBACK ImGuiMsgProc( int code, WPARAM wParam, LPARAM lParam )
{
    if ( code >= 0 )
    {
        auto msg = reinterpret_cast<LPMSG>( lParam );
        if ( ImGui_ImplWin32_WndProcHandler( msg->hwnd, msg->message,
                                             msg->wParam, msg->lParam ) )
        {
            return FALSE;
        }
    }
    return CallNextHookEx( g_hImGuiHook, code, wParam, lParam );
}*/

void IdleHook::Patch()
{
    // RedirectCall( 0x53ECBD, reinterpret_cast<void *>( Idle ) );
    /*rh::rw::engine::RwRenderEngine::RegisterPostInitCallback( []() {
        g_hImGuiHook = SetWindowsHookEx( WH_GETMESSAGE, ImGuiMsgProc, nullptr,
                                         GetCurrentThreadId() );
        ImGui::CreateContext();
        rh::engine::g_pRHRenderer->InitImGUI();
    } );*/
    RenderLoop::Init();
}

void IdleHook::Idle( void *data )
{
    // ShowCursor( true );

    using namespace std::chrono;
    high_resolution_clock::time_point t1;
    high_resolution_clock::time_point t2;

    t1 = high_resolution_clock::now();

    // Update timers
    TimeUpdate();
    // Init 2D stuff per frame.
    InitPerFrame2D();

    // Update game processes
    GameUpdate();

    t2 = high_resolution_clock::now();

    if ( !data )
        return;
    PrepareRwCamera();

    // rh::engine::g_pRHRenderer->ImGUIStartFrame();

    //    ImGui_ImplWin32_NewFrame();
    // ImGui::NewFrame();

    if ( !RsCameraBeginUpdate( Scene.m_pRwCamera ) )
        return;

    if ( !g_pFrontEndMenuManager->m_bMenuActive )
    {
        RwV2d mousePos{};
        mousePos.x = 1920 * 0.5f;
        mousePos.y = 1080 * 0.5f;
        // POINT pMousePos;

#define RsMouseSetPos( pos ) ( (int( __cdecl * )( RwV2d * ))0x6194A0 )( pos )

        // GetCursorPos( &pMousePos );
        RsMouseSetPos( &mousePos );

        auto &sky_state = rh::rw::engine::gRenderClient->RenderState.SkyState;
        auto &cur_cs    = *(CColourSet *)0xB7C4A0;

        auto *vec_to_sun_arr   = (RwV3d *)0xB7CA50;
        int & current_tc_value = *(int *)0xB79FD0;

        sky_state.mSkyTopColor[0]    = float( cur_cs.m_nSkyTopRed ) / 255.0f;
        sky_state.mSkyTopColor[1]    = float( cur_cs.m_nSkyTopGreen ) / 255.0f;
        sky_state.mSkyTopColor[2]    = float( cur_cs.m_nSkyTopBlue ) / 255.0f;
        sky_state.mSkyTopColor[3]    = 1.0f;
        sky_state.mSkyBottomColor[0] = float( cur_cs.m_nSkyBottomRed ) / 255.0f;
        sky_state.mSkyBottomColor[1] =
            float( cur_cs.m_nSkyBottomGreen ) / 255.0f;
        sky_state.mSkyBottomColor[2] =
            float( cur_cs.m_nSkyBottomBlue ) / 255.0f;
        sky_state.mSkyBottomColor[3] = 1.0f;
        sky_state.mAmbientColor[0]   = cur_cs.m_fAmbientRed;
        sky_state.mAmbientColor[1]   = cur_cs.m_fAmbientGreen;
        sky_state.mAmbientColor[2]   = cur_cs.m_fAmbientBlue;
        sky_state.mAmbientColor[3]   = 1.0f;

        sky_state.mSunDir[0] = vec_to_sun_arr[current_tc_value].x;
        sky_state.mSunDir[1] = vec_to_sun_arr[current_tc_value].y;
        sky_state.mSunDir[2] = vec_to_sun_arr[current_tc_value].z;
        sky_state.mSunDir[3] = 1.0f;

        CRenderer::ConstructRenderList();
        CRenderer::PreRender();
        // CWorld::ProcessPedsAfterPreRender();

        RenderLoop::Render();
        int Render2dStuffAddress = *(DWORD *)0x53EB13 + 0x53EB12 + 5;
        ( (int( __cdecl * )())Render2dStuffAddress )();
    }

    RenderHUD();
    /*
        ImGui::Begin( "Info" );
        ImGui::Text( "Game update time:%f ms.",
                     duration_cast<duration<double>>( t2 - t1 ).count() * 1000.0
       ); ImGui::Text( "Game update fps:%f", 1.0 /
       duration_cast<duration<double>>( t2 - t1 ).count() ); ImGui::End();

        ImGui::Render();*/
    // rh::engine::g_pRHRenderer->ImGUIRender();

    rh::RwCameraEndUpdate( Scene.m_pRwCamera );
    RsCameraShowRaster( Scene.m_pRwCamera );
}

void IdleHook::TimeUpdate()
{
    CGame::TimeMillisecondsFromStart = CTimer::GetTimeMillisecondsFromStart();
    CTimer::Update();
}

void IdleHook::InitPerFrame2D()
{
    CSprite2d::InitPerFrame();
    CFont::InitPerFrame();
}

void IdleHook::PrepareRwCamera()
{
    RwRGBA clear_color = { 50, 50, 128, 255 };
    rh::RwCameraClear( Scene.m_pRwCamera, &clear_color,
                       rwCAMERACLEARSTENCIL | rwCAMERACLEARZ |
                           rwCAMERACLEARIMAGE );
}

void IdleHook::GameUpdate()
{
    CGame::Process();
    gAudioEngine->Service();
}

void IdleHook::RenderHUD()
{
    if ( g_pFrontEndMenuManager->m_bMenuActive )
        g_pFrontEndMenuManager->DrawFrontEnd();

    DoFade();
    CHud::DrawAfterFade();
    CFont::RenderFontBuffer(); // CFont::DrawFonts
}
