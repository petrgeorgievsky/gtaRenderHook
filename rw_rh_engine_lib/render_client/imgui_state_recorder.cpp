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
LRESULT ImGuiImplWin32WndProcHandler( HWND hwnd, UINT msg, WPARAM w_param,
                                      [[maybe_unused]] LPARAM l_param )
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
        int button;
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
            button = ( GET_XBUTTON_WPARAM( w_param ) == XBUTTON1 ) ? 3 : 4;
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
        int button;
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
            button = ( GET_XBUTTON_WPARAM( w_param ) == XBUTTON1 ) ? 3 : 4;
        }
        input_state.MouseDown[button] = false;
        if ( !input_state.IsAnyMouseDown() && ::GetCapture() == hwnd )
            ::ReleaseCapture();
        return 0;
    }
    case WM_MOUSEWHEEL:
        input_state.MouseWheel +=
            (float)GET_WHEEL_DELTA_WPARAM( w_param ) / (float)WHEEL_DELTA;
        return 0;
    case WM_MOUSEHWHEEL:
        input_state.MouseWheelH +=
            (float)GET_WHEEL_DELTA_WPARAM( w_param ) / (float)WHEEL_DELTA;
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if ( w_param < 256 )
            input_state.KeysDown[w_param] = true;
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if ( w_param < 256 )
            input_state.KeysDown[w_param] = false;
        return 0;
    default: return 0;
    }
}

static HHOOK gImguiHookWndproc;

LRESULT CALLBACK ImGuiMsgProc( int code, WPARAM w_param, LPARAM l_param )
{
    if ( code >= 0 )
    {
        auto msg = reinterpret_cast<LPMSG>( l_param );
        if ( ImGuiImplWin32WndProcHandler( msg->hwnd, msg->message, msg->wParam,
                                           msg->lParam ) )
        {
            return CallNextHookEx( gImguiHookWndproc, code, w_param, l_param );
        }
    }
    return CallNextHookEx( gImguiHookWndproc, code, w_param, l_param );
}

void rh::rw::engine::InstallWinProcHook()
{
    gImguiHookWndproc = SetWindowsHookEx( WH_GETMESSAGE, ImGuiMsgProc, nullptr,
                                         GetCurrentThreadId() );
    assert( gImguiHookWndproc != nullptr );
}

void rh::rw::engine::RemoveWinProcHook()
{
    assert( gImguiHookWndproc != nullptr );
    auto unhook_result = UnhookWindowsHookEx( gImguiHookWndproc );
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
