#include "WindowsSampleWrapper.h"

#include "DebugUtils/DebugLogger.h"

#include <utility>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::thread rh::tests::WindowsSampleWrapper::ms_renderingThread;
std::unique_ptr<rh::tests::TestSample>
      rh::tests::WindowsSampleWrapper::ms_pSampleImpl = nullptr;
void *rh::tests::WindowsSampleWrapper::m_pMouse       = nullptr;

rh::tests::WindowsSampleWrapper::WindowsSampleWrapper(
    WindowsSampleParams params, std::unique_ptr<TestSample> sampleImpl )
    : m_sParams( std::move( params ) )
{
    ms_pSampleImpl = std::move( sampleImpl );
}

bool rh::tests::WindowsSampleWrapper::Init()
{
    RegisterWindowClass();

    m_hWnd = InitWindow();

    if ( !m_hWnd )
        return FALSE;

    ms_pSampleImpl->SetMouseInputDevicePtr( m_pMouse );
    ShowWindow( static_cast<HWND>( m_hWnd ), SW_SHOW );
    UpdateWindow( static_cast<HWND>( m_hWnd ) );

    return ms_pSampleImpl->Initialize( m_hWnd );
}

void rh::tests::WindowsSampleWrapper::Run()
{
    ms_renderingThread = std::thread( []() { ms_pSampleImpl->Update(); } );

    MSG msg;

    while ( GetMessage( &msg, nullptr, 0, 0 ) != 0 )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg,
                                               WPARAM wParam, LPARAM lParam );

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam,
                          LPARAM lParam )
{
    if ( ImGui_ImplWin32_WndProcHandler( hWnd, message, wParam, lParam ) )
        return true;
    switch ( message )
    {
    case WM_DESTROY:
        rh::tests::WindowsSampleWrapper::ms_pSampleImpl->Stop();

        if ( rh::tests::WindowsSampleWrapper::ms_renderingThread.joinable() )
            rh::tests::WindowsSampleWrapper::ms_renderingThread.join();

        PostQuitMessage( 0 );
        break;
    default: return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}
void rh::tests::WindowsSampleWrapper::RegisterWindowClass()
{
    WNDCLASS wclass = {};

    wclass.style         = /*CS_HREDRAW | CS_VREDRAW | */ CS_OWNDC;
    wclass.lpfnWndProc   = WndProc;
    wclass.hInstance     = static_cast<HINSTANCE>( m_sParams.instance );
    wclass.lpszClassName = m_sParams.windowClass.c_str();

    RegisterClass( &wclass );
}

void *rh::tests::WindowsSampleWrapper::InitWindow()
{
    RECT r{};
    r.top    = 0;
    r.bottom = 720;
    r.left   = 0;
    r.right  = 1280;

    AdjustWindowRect( &r, WS_OVERLAPPEDWINDOW, false );

    return CreateWindow( m_sParams.windowClass.c_str(),
                         m_sParams.sampleTitle.c_str(), WS_OVERLAPPEDWINDOW, 20,
                         20, r.right - r.left, r.bottom - r.top, nullptr,
                         nullptr, static_cast<HINSTANCE>( m_sParams.instance ),
                         nullptr );
}