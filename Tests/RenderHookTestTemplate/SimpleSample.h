#pragma once
#include <TestUtils\TestSample.h>

/**
* @brief Simple test sample for RenderHook, write your own code as you wish
*
*/
class SimpleSample : public RHTests::TestSample
{
public:
    SimpleSample( RHEngine::RHRenderingAPI api, HINSTANCE inst ) : RHTests::TestSample( api, inst ) { }
    void CustomRender() override;
};