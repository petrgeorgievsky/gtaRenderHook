#include <TestUtils\WindowsSampleWrapper.h>
#include "ClearScreenSample.h"

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
    initParams.sampleTitle = L"ClearScreenSample";
    initParams.windowClass = L"CLEARSCREENSAMPLE";

    RHEngine::RHRenderingAPI renderingAPI = RHEngine::RHRenderingAPI::Vulkan;

    RHTests::WindowsSampleWrapper sample(
        initParams,
        std::unique_ptr<ClearScreenSample>(
            new ClearScreenSample(renderingAPI, hInstance)));

    // Initialize test sample.
    if (!sample.Init()) return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}