#pragma once
#include "../Engine/Definitions.h"

namespace rh::tests {
class TestSample
{
public:
    TestSample( engine::RenderingAPI api, void* inst );
    virtual ~TestSample();
    virtual bool Initialize( void* wnd );
    void Update();
    virtual void Render();
    void Stop();
    void SetMouseInputDevicePtr( void *mouse );
    virtual bool CustomInitialize();
    virtual void CustomUpdate( float dt );
    virtual void CustomRender();
    virtual void CustomShutdown();

protected:
    bool m_bRenderGUI = true;
    void *m_pMouse{};

private:
    float m_fDeltaTime = 0;
    long double averageTimePerFrame = 0;
    unsigned long frameCount = 0;
    bool m_bUpdate = true;
    void* hInst;
};
} // namespace RHTests
