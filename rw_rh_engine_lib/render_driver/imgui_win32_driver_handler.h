//
// Created by peter on 24.02.2021.
//

#pragma once

namespace rh::rw::engine
{
struct ImGuiInputState;
class ImGuiWin32DriverHandler
{
  public:
    bool Init( void *hwnd );

    void Shutdown();

    void UpdateMousePos();

    bool UpdateMouseCursor();

    void NewFrame( const ImGuiInputState &state );
};
} // namespace rh::rw::engine