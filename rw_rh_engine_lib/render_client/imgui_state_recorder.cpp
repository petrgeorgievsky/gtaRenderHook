//
// Created by peter on 23.02.2021.
//
#include "imgui_state_recorder.h"
#include "render_client.h"
#include <data_desc/imgui_input_state.h>

#include <imgui.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <DebugUtils/DebugLogger.h>
#include <Windows.h>

// Win32 message handler (process Win32 mouse/keyboard inputs, etc.)
LRESULT ImGui_ImplWin32_WndProcHandler( HWND hwnd, UINT msg, WPARAM wParam,
                                        LPARAM lParam )
{
    if ( !rh::rw::engine::gRenderClient )
        return 0;
    auto &input_state =
        rh::rw::engine::gRenderClient->RenderState.ImGuiInputState;
    switch ( msg )
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
    {
        int button = 0;
        if ( msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK )
        {
            button = 0;
        }
        if ( msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK )
        {
            button = 1;
        }
        if ( msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK )
        {
            button = 2;
        }
        if ( msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK )
        {
            button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4;
        }
        if ( !input_state.IsAnyMouseDown() && ::GetCapture() == nullptr )
            ::SetCapture( hwnd );
        input_state.MouseDown[button] = true;
        return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        int button = 0;
        if ( msg == WM_LBUTTONUP )
        {
            button = 0;
        }
        if ( msg == WM_RBUTTONUP )
        {
            button = 1;
        }
        if ( msg == WM_MBUTTONUP )
        {
            button = 2;
        }
        if ( msg == WM_XBUTTONUP )
        {
            button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4;
        }
        input_state.MouseDown[button] = false;
        if ( !input_state.IsAnyMouseDown() && ::GetCapture() == hwnd )
            ::ReleaseCapture();
        return 0;
    }
    case WM_MOUSEWHEEL:
        input_state.MouseWheel +=
            (float)GET_WHEEL_DELTA_WPARAM( wParam ) / (float)WHEEL_DELTA;
        return 0;
    case WM_MOUSEHWHEEL:
        input_state.MouseWheelH +=
            (float)GET_WHEEL_DELTA_WPARAM( wParam ) / (float)WHEEL_DELTA;
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if ( wParam < 256 )
            input_state.KeysDown[wParam] = true;
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if ( wParam < 256 )
            input_state.KeysDown[wParam] = false;
        return 0;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        // TODO: decide if needed
        // if ( wParam > 0 && wParam < 0x10000 )
        //    io.AddInputCharacterUTF16( (unsigned short)wParam );
        return 0;
    case WM_SETCURSOR:
        // if ( LOWORD( lParam ) == HTCLIENT &&
        //     ImGui_ImplWin32_UpdateMouseCursor() )
        //    return 1;
        return 0;
    case WM_DEVICECHANGE:
        // if ( (UINT)wParam == DBT_DEVNODES_CHANGED )
        //    g_WantUpdateHasGamepad = true;
        return 0;
    }
    return 0;
}

static HHOOK imgui_hook_wndproc;

LRESULT CALLBACK ImGuiMsgProc( int code, WPARAM wParam, LPARAM lParam )
{
    if ( code >= 0 )
    {
        auto msg = reinterpret_cast<LPMSG>( lParam );
        if ( ImGui_ImplWin32_WndProcHandler( msg->hwnd, msg->message,
                                             msg->wParam, msg->lParam ) )
        {
            return CallNextHookEx( imgui_hook_wndproc, code, wParam, lParam );
        }
    }
    return CallNextHookEx( imgui_hook_wndproc, code, wParam, lParam );
}

void rh::rw::engine::InstallWinProcHook()
{
    imgui_hook_wndproc = SetWindowsHookEx( WH_GETMESSAGE, ImGuiMsgProc, nullptr,
                                           GetCurrentThreadId() );
    assert( imgui_hook_wndproc != nullptr );
}

void rh::rw::engine::RemoveWinProcHook()
{
    assert( imgui_hook_wndproc != nullptr );
    auto unhook_result = UnhookWindowsHookEx( imgui_hook_wndproc );
    if ( !unhook_result )
        debug::DebugLogger::ErrorFmt( "Failed to uninstall window procedure "
                                      "hook required for imgui! ErrorCode %u",
                                      GetLastError() );
}
void rh::rw::engine::UpdateStateClient( ImGuiInputState &state )
{

    POINT pos;
    if ( HWND active_window = ::GetForegroundWindow() )
        if ( ::GetCursorPos( &pos ) && ::ScreenToClient( active_window, &pos ) )
        {
            state.MousePos[0] = (float)pos.x;
            state.MousePos[1] = (float)pos.y;
        }

    state.KeyCtrl  = ( ::GetKeyState( VK_CONTROL ) & 0x8000 ) != 0;
    state.KeyShift = ( ::GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
    state.KeyAlt   = ( ::GetKeyState( VK_MENU ) & 0x8000 ) != 0;
}
