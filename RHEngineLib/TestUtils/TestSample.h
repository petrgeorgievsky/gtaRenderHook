#pragma once
#include "../Engine/Definitions.h"
#include <common.h>

namespace rh::tests {
class TestSample
{
public:
    TestSample( rh::engine::RenderingAPI api, HINSTANCE inst );
    virtual ~TestSample();
    virtual bool Initialize( HWND wnd );
    void Update();
    virtual void Render();
    void Stop();
    void SetMouseInputDevicePtr( IDirectInputDevice8 *mouse );
    virtual bool CustomInitialize();
    virtual void CustomUpdate( float dt );
    virtual void CustomRender();
    virtual void CustomShutdown();

protected:
    bool m_bRenderGUI = true;
    IDirectInputDevice8 *m_pMouse{};

private:
    float m_fDeltaTime = 0;
    long double averageTimePerFrame = 0;
    unsigned long frameCount = 0;
    bool m_bUpdate = true;
    HINSTANCE hInst;
};
}; // namespace RHTests
