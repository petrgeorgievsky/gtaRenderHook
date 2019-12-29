#include "WindowsSampleWrapper.h"

#include "DebugUtils/DebugLogger.h"
#include "ImGUI/imgui_impl_win32.h"
#include <utility>

std::thread rh::tests::WindowsSampleWrapper::ms_renderingThread;
std::unique_ptr<rh::tests::TestSample>
                     rh::tests::WindowsSampleWrapper::ms_pSampleImpl = nullptr;
IDirectInputDevice8 *rh::tests::WindowsSampleWrapper::m_pMouse       = nullptr;

rh::tests::WindowsSampleWrapper::WindowsSampleWrapper(
    WindowsSampleParams params, std::unique_ptr<TestSample> sampleImpl )
    : m_sParams( std::move( params ) )
{
    ms_pSampleImpl = std::move( sampleImpl );
}

BOOL rh::tests::WindowsSampleWrapper::Init()
{
    RegisterWindowClass();
    // Initialize input handlers
    DirectInput8Create(
        m_sParams.instance, DIRECTINPUT_VERSION, IID_IDirectInput8A,
        reinterpret_cast<LPVOID *>( &m_pDirectInput ), nullptr );
    m_pDirectInput->CreateDevice( GUID_SysMouse, &m_pMouse, nullptr );

    m_hWnd = InitWindow();

    if ( !m_hWnd )
        return FALSE;
    m_pMouse->SetDataFormat( &c_dfDIMouse );
    m_pMouse->SetCooperativeLevel( m_hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY |
                                               DISCL_FOREGROUND );
    ms_pSampleImpl->SetMouseInputDevicePtr( m_pMouse );
    ShowWindow( m_hWnd, SW_SHOW );
    UpdateWindow( m_hWnd );

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
    // Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init( m_hWnd );

    if ( !ms_pSampleImpl->Initialize( m_hWnd ) )
        return FALSE;

    return TRUE;
}

void rh::tests::WindowsSampleWrapper::Run()
{
    ms_renderingThread = std::thread( [&]() { ms_pSampleImpl->Update(); } );
    MSG msg;

    while ( GetMessage( &msg, nullptr, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

void rh::tests::WindowsSampleWrapper::RegisterWindowClass()
{
    WNDCLASS wclass = {};

    wclass.style         = /*CS_HREDRAW | CS_VREDRAW | */ CS_OWNDC;
    wclass.lpfnWndProc   = WndProc;
    wclass.hInstance     = m_sParams.instance;
    wclass.lpszClassName = m_sParams.windowClass.c_str();

    RegisterClass( &wclass );
}

HWND rh::tests::WindowsSampleWrapper::InitWindow()
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
                         nullptr, m_sParams.instance, nullptr );
}
extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg,
                                               WPARAM wParam, LPARAM lParam );

LRESULT CALLBACK rh::tests::WindowsSampleWrapper::WndProc( HWND   hWnd,
                                                           UINT   message,
                                                           WPARAM wParam,
                                                           LPARAM lParam )
{
    if ( ImGui_ImplWin32_WndProcHandler( hWnd, message, wParam, lParam ) )
        return true;
    switch ( message )
    {
    case WM_DESTROY:
        ms_pSampleImpl->Stop();

        if ( ms_renderingThread.joinable() )
            ms_renderingThread.join();

        PostQuitMessage( 0 );
        break;
    default: return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}
