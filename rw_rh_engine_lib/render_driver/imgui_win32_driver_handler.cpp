//
// Created by peter on 24.02.2021.
//

#include "imgui_win32_driver_handler.h"
#include <data_desc/imgui_input_state.h>

#include <imgui.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace rh::rw::engine
{

HWND             Window{};
LARGE_INTEGER    Time{};
LARGE_INTEGER    TicksPerSecond{};
ImGuiMouseCursor LastMouseCursor = ImGuiMouseCursor_COUNT;

bool ImGuiWin32DriverHandler::Init( void *hwnd )
{
    if ( !::QueryPerformanceFrequency( &TicksPerSecond ) )
        return false;
    if ( !::QueryPerformanceCounter( &TicksPerSecond ) )
        return false;
    Window = static_cast<HWND>( hwnd );

    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |=
        ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
    // values (optional)
    io.BackendFlags |=
        ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos
    // requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_win32_rh";
    io.ImeWindowHandle     = hwnd;

    // Keyboard mapping. ImGui will use those indices to peek into the
    // io.KeysDown[] array that we will update during the application
    // lifetime.
    io.KeyMap[ImGuiKey_Tab]         = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown]    = VK_NEXT;
    io.KeyMap[ImGuiKey_Home]        = VK_HOME;
    io.KeyMap[ImGuiKey_End]         = VK_END;
    io.KeyMap[ImGuiKey_Insert]      = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete]      = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = VK_BACK;
    io.KeyMap[ImGuiKey_Space]       = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter]       = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape]      = VK_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    io.KeyMap[ImGuiKey_A]           = 'A';
    io.KeyMap[ImGuiKey_C]           = 'C';
    io.KeyMap[ImGuiKey_V]           = 'V';
    io.KeyMap[ImGuiKey_X]           = 'X';
    io.KeyMap[ImGuiKey_Y]           = 'Y';
    io.KeyMap[ImGuiKey_Z]           = 'Z';

    return true;
}

void ImGuiWin32DriverHandler::Shutdown()
{
    Window                  = nullptr;
    Time.QuadPart           = 0;
    TicksPerSecond.QuadPart = 0;
    LastMouseCursor         = ImGuiMouseCursor_COUNT;
}

void ImGuiWin32DriverHandler::UpdateMousePos()
{
    ImGuiIO &io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when
    // ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if ( io.WantSetMousePos )
    {
        POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
        if ( ::ClientToScreen( Window, &pos ) )
            ::SetCursorPos( pos.x, pos.y );
    }

    // Set mouse position
    io.MousePos = ImVec2( -FLT_MAX, -FLT_MAX );
    POINT pos;
    if ( HWND active_window = ::GetForegroundWindow() )
        if ( active_window == Window || ::IsChild( active_window, Window ) )
            if ( ::GetCursorPos( &pos ) && ::ScreenToClient( Window, &pos ) )
                io.MousePos = ImVec2( (float)pos.x, (float)pos.y );
}

bool ImGuiWin32DriverHandler::UpdateMouseCursor()
{
    ImGuiIO &io = ImGui::GetIO();
    if ( io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange )
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if ( imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor )
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no
        // cursor
        ::SetCursor( nullptr );
    }
    else
    {
        // Show OS mouse cursor
        LPTSTR win32_cursor;
        switch ( imgui_cursor )
        {
        default:
        case ImGuiMouseCursor_Arrow: win32_cursor = IDC_ARROW; break;

        case ImGuiMouseCursor_TextInput: win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll: win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW: win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS: win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW: win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE: win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand: win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed: win32_cursor = IDC_NO; break;
        }
        ::SetCursor( ::LoadCursor( nullptr, win32_cursor ) );
    }
    return true;
}

void ImGuiWin32DriverHandler::NewFrame( const ImGuiInputState &state )
{
    ImGui::GetCurrentContext();
    ImGuiIO &io = ImGui::GetIO();
    /* IM_ASSERT( io.Fonts->IsBuilt() &&
                "Font atlas not built! It is generally built by the renderer "
                "backend. Missing call to renderer NewFrame() function? e.g. "
                "ImGui_ImplVulkan_NewFrame()." );*/
    // Setup display size (every frame to accommodate for window resizing)
    RECT rect = { 0, 0, 0, 0 };
    if ( !::GetClientRect( Window, &rect ) )
        assert( false && "Failed to get client rect for renderer window!" );
    io.DisplaySize = ImVec2( (float)( rect.right - rect.left ),
                             (float)( rect.bottom - rect.top ) );
    // Setup time step
    LARGE_INTEGER current_time = { 0ll };
    ::QueryPerformanceCounter( &current_time );
    io.DeltaTime = (float)( current_time.QuadPart - Time.QuadPart ) /
                   TicksPerSecond.QuadPart;
    Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = state.KeyCtrl; //( ::GetKeyState( VK_CONTROL ) & 0x8000 ) != 0;
    io.KeyShift = state.KeyShift; //( ::GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
    io.KeyAlt   = state.KeyAlt;   //( ::GetKeyState( VK_MENU ) & 0x8000 ) != 0;
    io.KeySuper = false;
    // Filled by window proc handler on client side
    for ( auto i = 0; i < 5; i++ )
        io.MouseDown[i] = state.MouseDown[i];
    for ( auto i = 0; i < 512; i++ )
        io.KeysDown[i] = state.KeysDown[i];
    io.MouseWheel  = state.MouseWheel;
    io.MouseWheelH = state.MouseWheelH;

    // Update OS mouse position
    UpdateMousePos();

    io.MousePos = ImVec2( state.MousePos[0], state.MousePos[1] );

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor =
        io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if ( LastMouseCursor != mouse_cursor )
    {
        LastMouseCursor = mouse_cursor;
        UpdateMouseCursor();
    }
}
} // namespace rh::rw::engine