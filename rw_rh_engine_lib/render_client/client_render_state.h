//
// Created by peter on 10.02.2021.
//
#pragma once
#include "data_desc/immediate_mode/im_state.h"
#include "data_desc/sky_state.h"
#include "data_desc/viewport_state.h"
#include "im2d_state_recorder.h"
#include "im3d_state_recorder.h"
#include "light_state_recorder.h"
#include "mesh_instance_state_recorder.h"
#include "skin_instance_state_recorder.h"

namespace rh::rw::engine
{

class ClientRenderState
{
  public:
    MainViewportState         ViewportState;
    SkyState                  SkyState;
    ImmediateState            ImState;
    Im2DStateRecorder         Im2D{ ImState };
    Im3DStateRecorder         Im3D{ ImState };
    MeshInstanceStateRecorder MeshDrawCalls{};
    SkinInstanceStateRecorder SkinMeshDrawCalls{};
    LightStateRecorder        Lights{};
};

} // namespace rh::rw::engine