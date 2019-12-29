#include "TestSample.h"

#include "../DebugUtils/DebugLogger.h"
#include "ImGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"
#include <memory>

rh::tests::TestSample::TestSample( rh::engine::RenderingAPI, HINSTANCE inst )
{
    hInst = inst;
}

rh::tests::TestSample::~TestSample() = default;

bool rh::tests::TestSample::Initialize( HWND )
{
    return CustomInitialize();
}

void rh::tests::TestSample::Update()
{
    using namespace std::chrono;
    high_resolution_clock::time_point t1;
    high_resolution_clock::time_point t2;
    long double currentFrameTime = 0.0L;

    while ( m_bUpdate ) {
        t1 = high_resolution_clock::now();
        CustomUpdate( m_fDeltaTime );
        Render();
        t2 = high_resolution_clock::now();
        m_fDeltaTime = duration_cast<duration<float>>( t2 - t1 ).count();
        currentFrameTime = duration_cast<duration<long double>>( t2 - t1 ).count();
        frameCount++;
        averageTimePerFrame = ( averageTimePerFrame * ( frameCount - 1 ) + currentFrameTime )
                              / frameCount;
        // RHDebug::DebugLogger::Log(std::to_wstring(averageTimePerFrame));
    }
    rh::debug::DebugLogger::Log(
        ToRHString( "avg. frametime:" + std::to_string( averageTimePerFrame * 1000 ) + " ms." ) );
    CustomShutdown();
}

void rh::tests::TestSample::Render()
{
    CustomRender();
}

void rh::tests::TestSample::Stop()
{
    m_bUpdate = false;
}

void rh::tests::TestSample::SetMouseInputDevicePtr( IDirectInputDevice8 *mouse )
{
    m_pMouse = mouse;
}

bool rh::tests::TestSample::CustomInitialize()
{
    return true;
}

void rh::tests::TestSample::CustomUpdate( float ) {}

void rh::tests::TestSample::CustomRender() {}

void rh::tests::TestSample::CustomShutdown() {}
