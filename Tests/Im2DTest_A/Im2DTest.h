#pragma once
#include <TestUtils\TestSample.h>

/**
* @brief Immediate 2D mode test, draws a few different primitives using Immediate 2D mode
*
*/
class Im2DTest : public RHTests::TestSample
{
public:
    Im2DTest(RHEngine::RHRenderingAPI api, HINSTANCE inst) : RHTests::TestSample(api, inst) { }

    void CustomRender() override;
    bool CustomInitialize() override;

    void InitTriangleArrayVerticies();
    void InitLineCircleVerticies();
    void InitPointListVerticies();

    void DrawTriangle(float x, float y, float scale);

    std::vector<RwIm2DVertex> GetTriangle(float x, float y, float scale);
    std::vector<RwIm2DVertex> GetLine(float x0, float y0, float x1, float y1);
    std::vector<RwIm2DVertex> GetPoint(float x, float y);
private:
    std::vector<RwIm2DVertex> m_vTriangleList;
    std::vector<RwIm2DVertex> m_vLineList;
    std::vector<RwIm2DVertex> m_vPointList;
};