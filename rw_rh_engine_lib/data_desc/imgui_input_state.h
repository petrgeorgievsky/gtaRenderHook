//
// Created by peter on 24.02.2021.
//

#pragma once
namespace rh::rw::engine
{
struct ImGuiInputState
{
    float MousePos[2]; // Mouse position, in pixels. Set to ImVec2(-FLT_MAX,
    // -FLT_MAX) if mouse is unavailable (on another screen,
    // etc.)
    bool MouseDown[5]; // Mouse buttons: 0=left, 1=right, 2=middle + extras
    // (ImGuiMouseButton_COUNT == 5). Dear ImGui mostly uses
    // left and right buttons. Others buttons allows us to
    // track if the mouse is being used by your application +
    // available to user as a convenience via IsMouse** API.
    float
        MouseWheel; // Mouse wheel Vertical: 1 unit scrolls about 5 lines text.
    float MouseWheelH; // Mouse wheel Horizontal. Most users don't have a mouse
    // with an horizontal wheel, may not be filled by all
    // back-ends.
    bool KeyCtrl;       // Keyboard modifier pressed: Control
    bool KeyShift;      // Keyboard modifier pressed: Shift
    bool KeyAlt;        // Keyboard modifier pressed: Alt
    bool EnablePause;   // Enable Pause
    bool KeysDown[512]; // Keyboard keys that are pressed (ideally left in the
    // "native" order your engine has access to keyboard
    // keys, so you can use your own defines/enums for keys).

    //

    // helper
    bool IsAnyMouseDown()
    {
        for ( auto mouse : MouseDown )
            if ( mouse )
                return true;
        return false;
    }

    bool Reset()
    {
        for ( auto mouse : MouseDown )
            if ( mouse )
                return true;
        return false;
    }
};
} // namespace rh::rw::engine