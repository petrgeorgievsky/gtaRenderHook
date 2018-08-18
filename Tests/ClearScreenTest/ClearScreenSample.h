#pragma once
#include <TestUtils\TestSample.h>

/**
* @brief Simple sample that clears framebuffer with color varying from blue to pink
* by each frame incrementing/decrementing red color value
*
*/
class ClearScreenSample : public RHTests::TestSample
{
public:
    ClearScreenSample(RHEngine::RHRenderingAPI api, HINSTANCE inst) : RHTests::TestSample(api, inst) { }
    void CustomRender() override;
private:
    unsigned char m_nCurrentRed = 0;
    char m_nCurrentRedIncr = 1;
};