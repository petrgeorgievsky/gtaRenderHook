//
// Created by peter on 18.02.2021.
//
#include "frame_info.h"
#include <ipc/MemoryReader.h>

namespace rh::rw::engine
{

FrameState FrameState::Deserialize( MemoryReader &reader )
{
    FrameState state{};
    state.Viewport      = reader.Read<MainViewportState>();
    state.Sky           = reader.Read<SkyState>();
    state.Lights        = AnalyticLightsState::Deserialize( reader );
    state.Im2D          = Im2DRenderState::Deserialize( reader );
    state.Im3D          = Im3DRenderState::Deserialize( reader );
    state.MeshInstances = MeshInstanceState::Deserialize( reader );
    state.SkinInstances = SkinInstanceState::Deserialize( reader );
    return state;
}
} // namespace rh::rw::engine