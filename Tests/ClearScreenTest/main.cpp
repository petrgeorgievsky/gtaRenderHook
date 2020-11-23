#include <TestUtils\WindowsSampleWrapper.h>

#include "ClearScreenSample.h"
#include <DebugUtils/DebugLogger.h>
#include <Windows.h>
#include <memory>

int APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
    UNREFERENCED_PARAMETER( nCmdShow );

    using namespace rh;

    // Window params initialization
    tests::WindowsSampleParams initParams{
        .instance    = hInstance,
        .sampleTitle = "ClearScreenSample",
        .windowClass = "CLEARSCREENSAMPLE",
    };

    engine::RenderingAPI renderingAPI = engine::RenderingAPI::DX11;

    debug::DebugLogger::Init( "log_rh.log", debug::LogLevel::Info );

    auto sample_impl =
        std::make_unique<ClearScreenSample>( renderingAPI, hInstance );

    tests::WindowsSampleWrapper sample( initParams, std::move( sample_impl ) );

    // Initialize test sample.
    if ( !sample.Init() )
        return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}
