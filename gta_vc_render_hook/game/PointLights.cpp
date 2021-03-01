//
// Created by peter on 01.03.2021.
//

#include "PointLights.h"
#include <injection_utils/InjectorHelpers.h>
#include <render_client/render_client.h>

int32_t PointLights::AddLight( char a1, float x, float y, float z, float dx,
                               float dy, float dz, float rad, float r, float g,
                               float b, char fogtype, char extrashadows )
{
    using namespace rh::rw::engine;
    assert( gRenderClient );
    auto &light_state = gRenderClient->RenderState.Lights;

    PointLight l{};
    l.mPos[0] = x;
    l.mPos[1] = y;
    l.mPos[2] = z;
    l.mRadius = rad;
    l.mDir[0] = dx;
    l.mDir[1] = dy;
    l.mDir[2] = dz;
    if ( a1 == 1 )
        l.mSpotCutoff = 0.7f;
    else
        l.mSpotCutoff = -0.5f;
    l.mColor[0] = r;
    l.mColor[1] = g;
    l.mColor[2] = b;
    l.mColor[3] = 1;

    light_state.RecordPointLight( std::move( l ) );
    return 1;
}

int32_t AddBikeHeadLight( char a1, float x, float y, float z, float dx,
                          float dy, float dz, float rad, float r, float g,
                          float b, char fogtype, char extrashadows )
{
    // Move light along direction to fix bike headlights
    return PointLights::AddLight( a1, x + dx, y + dy, z + dz, dx, dy, dz, 40.0f,
                                  r, g, b, fogtype, extrashadows );
}

void PointLights::Patch()
{
    RedirectJump( 0x567700, reinterpret_cast<void *>( PointLights::AddLight ) );
    RedirectCall( 0x60BE6E, reinterpret_cast<void *>( AddBikeHeadLight ) );
}
