#pragma once
#include <rwtestsample.h>

/**
* @brief Simple sample that clears framebuffer with color varying from blue to pink
* by each frame incrementing/decrementing red color value
*
*/
class ClearScreenSample : public rh::rw::engine::RwTestSample
{
public:
    ClearScreenSample( rh::engine::RenderingAPI api, HINSTANCE inst )
        : rh::rw::engine::RwTestSample( api, inst )
    {}
    void CustomRender() override;
private:
    unsigned char m_nCurrentRed = 0;
    char m_nCurrentRedIncr = 1;
};
