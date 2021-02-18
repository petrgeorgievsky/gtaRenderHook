#include "cameraclearcmd.h"
#include <render_client/render_client.h>

using namespace rh::rw::engine;

RwCameraClearCmd::RwCameraClearCmd( RwCamera *camera, RwRGBA *color,
                                    int32_t clear_mode )
    : m_pCamera( camera ), m_nClearMode( clear_mode )
{
    m_aClearColor = color;
}
RwCameraClearCmd::~RwCameraClearCmd() = default;

bool RwCameraClearCmd::Execute()
{
    assert( gRenderClient );
    auto &frame      = gRenderClient->RenderState.ViewportState;
    frame.ClearColor = *m_aClearColor;
    frame.ClearFlags = m_nClearMode;
    frame.ClearDepth = 1.0f;
    return true;
}
