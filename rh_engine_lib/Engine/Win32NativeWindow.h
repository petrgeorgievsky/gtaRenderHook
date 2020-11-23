#pragma once
#include "Common\INativeWindow.h"
#ifndef HWND
using HWND = struct HWND__ *;
#endif
struct Win32NativeWindowInitParams
{
    HWND mWindowHandle;
};

class Win32NativeWindow : INativeWindow
{
  public:
    Win32NativeWindow( const Win32NativeWindowInitParams & init_params);
    virtual ~Win32NativeWindow() override;
};