//
// Created by peter on 16.02.2021.
//

#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <data_desc/imgui_input_state.h>
#include <data_desc/light_system/lighting_state.h>
#include <data_desc/sky_state.h>
#include <data_desc/viewport_state.h>
#include <render_client/im2d_state_recorder.h>
#include <render_client/im3d_state_recorder.h>
#include <render_client/mesh_instance_state_recorder.h>
#include <render_client/skin_instance_state_recorder.h>

namespace rh::rw::engine
{

struct FrameState
{
    MainViewportState * Viewport;
    SkyState *          Sky;
    ImGuiInputState *   ImGuiInput;
    AnalyticLightsState Lights;
    Im2DRenderState     Im2D;
    Im3DRenderState     Im3D;
    MeshInstanceState   MeshInstances;
    SkinInstanceState   SkinInstances;
    static FrameState   Deserialize( MemoryReader &reader );
};

} // namespace rh::rw::engine