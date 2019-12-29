#include <TestUtils\WindowsSampleWrapper.h>

#include "ClearScreenSample.h"
#include <memory>

int APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
    UNREFERENCED_PARAMETER( nCmdShow );

    // Window params initialization
    rh::tests::WindowsSampleParams initParams;
    initParams.instance = hInstance;
    initParams.sampleTitle = TEXT( "ClearScreenSample" );
    initParams.windowClass = TEXT( "CLEARSCREENSAMPLE" );

    rh::engine::RenderingAPI renderingAPI = rh::engine::RenderingAPI::DX11;

    auto sample_impl = std::make_unique<ClearScreenSample>( renderingAPI, hInstance );

    rh::tests::WindowsSampleWrapper sample( initParams, std::move( sample_impl ) );

    // Initialize test sample.
    if ( !sample.Init() )
        return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}
