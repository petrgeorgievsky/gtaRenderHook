//
// Created by peter on 28.10.2020.
//

#include "PointLights.h"
#include "../call_redirection_util.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <render_client/render_client.h>

int32_t PointLights::AddLight( char a1, float x, float y, float z, float dx,
                               int dy, int dz, float rad, float r, float g,
                               float b, char fogtype, char extrashadows )
{
    using namespace rh::rw::engine;
    assert( gRenderClient );
    auto &light_state = gRenderClient->RenderState.Lights;

    PointLight l{};
    l.mPos[0]   = x;
    l.mPos[1]   = y;
    l.mPos[2]   = z;
    l.mRadius   = rad;
    l.mColor[0] = r;
    l.mColor[1] = g;
    l.mColor[2] = b;
    l.mColor[3] = 1;

    light_state.RecordPointLight( std::move( l ) );
    return 1;
}

void PointLights::Patch()
{
    RedirectJump( GetAddressByGame( 0x510790, 0x510980, 0x510910 ),
                  reinterpret_cast<void *>( PointLights::AddLight ) );
}
