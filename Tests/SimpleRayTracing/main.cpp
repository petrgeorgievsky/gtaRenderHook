#include <TestUtils\WindowsSampleWrapper.h>
#include <DebugUtils\DebugLogger.h>
#include <Engine\Definitions.h>
#include "SimpleRayTracingSample.h"

int APIENTRY wWinMain( HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPWSTR    lpCmdLine,
                       int       nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
    // Window params initialization
    RHTests::WindowsSampleParams initParams;
    initParams.instance = hInstance;
    initParams.sampleTitle = TEXT( "SimpleRTSample" );
    initParams.windowClass = TEXT( "RHSimpleRTSAMPLE" );

    RHEngine::RHRenderingAPI renderingAPI = RHEngine::RHRenderingAPI::DX11;

    RHEngine::String logFilePath = ToRHString( "RHDebug.log" );

    RHDebug::DebugLogger::Init( logFilePath, RHDebug::LogLevel::Info );

    RHTests::WindowsSampleWrapper sample(
        initParams,
        std::unique_ptr<SimpleRayTracingSample>(
            new SimpleRayTracingSample( renderingAPI, hInstance ) ) );

    // Initialize test sample.
    if ( !sample.Init() ) return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}