#include <TestUtils\WindowsSampleWrapper.h>
#include "Im2DTest.h"

int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // Window params initialization
    RHTests::WindowsSampleParams initParams;
    initParams.instance = hInstance;
    initParams.sampleTitle = TEXT("Im2DSample");
    initParams.windowClass = TEXT("IM2DSAMPLE");

    RHEngine::RHRenderingAPI renderingAPI = RHEngine::RHRenderingAPI::DX11;

    RHTests::WindowsSampleWrapper sample(
        initParams,
        std::unique_ptr<Im2DTest>(
            new Im2DTest( renderingAPI, hInstance ) ) );

    // Initialize test sample.
    if (!sample.Init())
        return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}