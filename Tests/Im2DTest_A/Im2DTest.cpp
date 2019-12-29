#include "Im2DTest.h"
#include <RWUtils\RwAPI.h>

void Im2DTest::CustomRender()
{
    RwRGBA clearColor = { 0, 0, 128, 255 };

    RH_RWAPI::RwCameraClear( m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE );

    RH_RWAPI::RwIm2DRenderPrimitive( rwPRIMTYPETRILIST, m_vTriangleList.data(), m_vTriangleList.size() );
    RH_RWAPI::RwIm2DRenderPrimitive( rwPRIMTYPEPOLYLINE, m_vLineList.data(), m_vLineList.size() );
    RH_RWAPI::RwIm2DRenderPrimitive( rwPRIMTYPEPOINTLIST, m_vPointList.data(), m_vPointList.size() );
}

bool Im2DTest::CustomInitialize()
{
    RHTests::TestSample::CustomInitialize();
    InitTriangleArrayVerticies();
    InitLineCircleVerticies();
    InitPointListVerticies();
    return true;
}

void Im2DTest::InitTriangleArrayVerticies()
{
    // Constructs a list of 40x20 triangles
    const int x_count = 4;
    const int y_count = 4;
    const float spacing = 64;
    const float scale = 64;
    const float c_x = 50;
    const float c_y = 50;

    for ( int x = 1; x <= x_count; x++ )
    {
        for ( int y = 1; y <= y_count; y++ )
        {
            auto triangle = GetTriangle( x * spacing + c_x, y * spacing + c_y, scale );
            m_vTriangleList.insert(
                m_vTriangleList.end(),
                std::make_move_iterator( triangle.begin() ),
                std::make_move_iterator( triangle.end() )
            );
        }
    }
}

void Im2DTest::InitLineCircleVerticies()
{
    // Constructs a circle using lines
    const int line_count = 16;
    const float c_x = 200;
    const float c_y = 200;
    const float r = 40;
    const float PI = 3.14f;
    const float angular_scale = PI * 2.0f / (float)line_count;
    float prev_x = sinf( 0 ) * r + c_x;
    float prev_y = cosf( 0 ) * r + c_y;
    float curr_x, curr_y;

    for ( int i = 0; i < line_count; i++ )
    {
        curr_x = sinf( ( i + 1 )*angular_scale ) * r + c_x;
        curr_y = cosf( ( i + 1 )*angular_scale ) * r + c_y;
        auto line = GetLine( prev_x, prev_y, curr_x, curr_y );
        prev_x = curr_x;
        prev_y = curr_y;
        m_vLineList.insert(
            m_vLineList.end(),
            std::make_move_iterator( line.begin() ),
            std::make_move_iterator( line.end() )
        );
    }
}

void Im2DTest::InitPointListVerticies()
{
    m_vPointList.reserve( 16 );

    RwIm2DVertex point = { 0,0,0,1,0xFFFFFFFF,0,0 };

    // Construct point list
    for ( int i = 0; i < 16; i++ )
    {
        point.x = (float)( rand() % 120 ) + 200.0f;
        point.y = (float)( rand() % 120 ) + 200.0f;
        m_vPointList.emplace_back( point );
    }
}

void Im2DTest::DrawTriangle( float x, float y, float scale )
{
    std::vector<RwIm2DVertex> triangle = GetTriangle( x, y, scale );
    RHEngine::g_pRWRenderEngine->Im2DRenderPrimitive( rwPRIMTYPETRILIST, triangle.data(), triangle.size() );
}

std::vector<RwIm2DVertex> Im2DTest::GetTriangle( float x, float y, float scale )
{
    return { {x - scale * 0.5f,y,0,1,0xFFFFFF00,0,0},{x + scale * 0.5f,y,0,1,0xFFFF00FF,0,0},{x,y + scale,0,1,0xFF00FFFF,0,0} };
}

std::vector<RwIm2DVertex> Im2DTest::GetLine( float x0, float y0, float x1, float y1 )
{
    return { {x0,y0,0,1,0xFFFFFFFF,0,0},{x1,y1,0,1,0xFFFFFFFF,0,0} };
}

std::vector<RwIm2DVertex> Im2DTest::GetPoint( float x0, float y0 )
{
    return { {x0,y0,0,1,0xFFFFFFFF,0,0} };
}
