//
// Created by peter on 28.10.2020.
//

#include "PointLights.h"
#include <scene_graph.h>

int32_t PointLights::AddLight( char a1, float x, float y, float z, float dx,
                               int dy, int dz, float rad, float r, float g,
                               float b, char fogtype, char extrashadows )
{
    auto &frame_info = rh::rw::engine::GetCurrentSceneGraph()->mFrameInfo;
    if ( frame_info.mLightCount > 1024 )
        return 0;

    auto &l     = frame_info.mFirst4PointLights[frame_info.mLightCount];
    l.mPos[0]   = x;
    l.mPos[1]   = y;
    l.mPos[2]   = z;
    l.mRadius   = rad;
    l.mColor[0] = r;
    l.mColor[1] = g;
    l.mColor[2] = b;
    l.mColor[3] = 1;
    frame_info.mLightCount++;
    return 1;
}
