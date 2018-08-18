#include "stdafx.h"
#include "RHEngineTests.h"
#include <DebugUtils\DebugLogger.h>
#include <TestUtils\TestSample.h>
#include <TestUtils\WindowsSampleWrapper.h>

/**
 * @brief Simple sample that clears framebuffer with color varying from blue to pink 
 * by each frame incrementing/decrementing red color value
 * 
 */
class ClearScreenSample: public RHTests::TestSample
{
public:
    ClearScreenSample(RHEngine::RHRenderingAPI api, HINSTANCE inst) : RHTests::TestSample(api, inst) { }
    void CustomRender() override;
private:
    unsigned char m_nCurrentRed = 0;
    char m_nCurrentRedIncr = 1;
};

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
    initParams.sampleTitle = L"TestSample";
    initParams.windowClass = L"TESTSAMPLE";

    RHEngine::RHRenderingAPI renderingAPI = RHEngine::RHRenderingAPI::DX11;

    RHTests::WindowsSampleWrapper sample (
        initParams, 
        std::unique_ptr<ClearScreenSample> ( 
            new ClearScreenSample(renderingAPI, hInstance) ) );

    // Initialize test sample.
    if (!sample.Init()) return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}

void ClearScreenSample::CustomRender()
{
    m_nCurrentRed = m_nCurrentRed + m_nCurrentRedIncr;
    // If we reached 100% pink or 100% blue we'll change increment direction
    if (m_nCurrentRed % 0xFF == 0)
        m_nCurrentRedIncr *= -1;
    RwRGBA clearColor = { m_nCurrentRed, 0, 255, 255 };
    RHEngine::g_pRWRenderEngine->CameraClear(m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE);
}
