#include "idle_hook.h"
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
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_win32.h>
#include <RwRenderEngine.h>

#include <MemoryInjectionUtils/InjectorHelpers.h>

//static RwRGBA gColourTop = *reinterpret_cast<RwRGBA *>( 0xB72CA0 );
static HHOOK g_hImGuiHook;

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

LRESULT CALLBACK ImGuiMsgProc( int code, WPARAM wParam, LPARAM lParam )
{
    if ( code >= 0 ) {
        auto msg = reinterpret_cast<LPMSG>( lParam );
        if ( ImGui_ImplWin32_WndProcHandler( msg->hwnd, msg->message, msg->wParam, msg->lParam ) ) {
            return FALSE;
        }
    }
    return CallNextHookEx( g_hImGuiHook, code, wParam, lParam );
}

void IdleHook::Patch()
{
    RedirectCall( 0x53ECBD, reinterpret_cast<void *>( Idle ) );
    rh::rw::engine::RwRenderEngine::RegisterPostInitCallback( []() {
        g_hImGuiHook = SetWindowsHookEx( WH_GETMESSAGE,
                                         ImGuiMsgProc,
                                         nullptr,
                                         GetCurrentThreadId() );
        ImGui::CreateContext();
        rh::engine::g_pRHRenderer->InitImGUI();
    } );
    RenderLoop::Init();
}

void IdleHook::Idle( void *data )
{
    ShowCursor( true );
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

    rh::engine::g_pRHRenderer->ImGUIStartFrame();

    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if ( !RsCameraBeginUpdate( Scene.m_pRwCamera ) )
        return;

    if ( !g_pFrontEndMenuManager->m_bMenuActive ) {
        CRenderer::ConstructRenderList();
        CRenderer::PreRender();
        // CWorld::ProcessPedsAfterPreRender();

        RenderLoop::Render();
        int Render2dStuffAddress = *(DWORD *) 0x53EB13 + 0x53EB12 + 5;
        ( (int( __cdecl * )()) Render2dStuffAddress )();
    }

    RenderHUD();

    ImGui::Begin( "Info" );
    ImGui::Text( "Game update time:%f ms.",
                 duration_cast<duration<double>>( t2 - t1 ).count() * 1000.0 );
    ImGui::Text( "Game update fps:%f", 1.0 / duration_cast<duration<double>>( t2 - t1 ).count() );
    ImGui::End();

    ImGui::Render();
    rh::engine::g_pRHRenderer->ImGUIRender();

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
    RwRGBA clear_color = {50, 50, 128, 255};
    rh::RwCameraClear( Scene.m_pRwCamera,
                       &clear_color,
                       rwCAMERACLEARSTENCIL | rwCAMERACLEARZ | rwCAMERACLEARIMAGE );
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
