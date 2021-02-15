//
// Created by peter on 10.02.2021.
//
#pragma once
#include <common_headers.h>
#include <rw_engine/rh_backend/im2d_backend.h>
#include <rw_engine/rh_backend/im3d_backend.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rh_backend/skinned_mesh_backend.h>

namespace rh::rw::engine
{

struct ImmediateState
{
    uint64_t Raster = gNullRasterId;

    uint8_t ColorBlendSrc;
    uint8_t ColorBlendDst;
    uint8_t ColorBlendOp;
    uint8_t BlendEnable;

    uint8_t ZTestEnable;
    uint8_t ZWriteEnable;
    uint8_t StencilEnable;

    void Update( RwRenderState nState, void *pParam );
};

class ClientRenderState
{
  public:
    ImmediateState        ImState;
    Im2DClientGlobals     Im2D{ ImState };
    Im3DClient            Im3D{ ImState };
    BackendRendererClient MeshDrawCalls{};
    SkinRendererClient    SkinMeshDrawCalls{};
};

} // namespace rh::rw::engine