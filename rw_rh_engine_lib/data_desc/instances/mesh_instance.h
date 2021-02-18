//
// Created by peter on 18.02.2021.
//
#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <span>
#include <vector>

namespace rh::rw::engine
{

struct DrawCallInfo
{
    uint64_t            MeshId;
    uint64_t            DrawCallId;
    uint64_t            PipelineId;
    uint64_t            LodId;
    uint64_t            MaterialListStart;
    uint64_t            MaterialListCount;
    DirectX::XMFLOAT4X3 WorldTransform;
};

class MemoryWriter;
class MemoryReader;

/**
 * Memory view into current frame im3d render state
 */
struct MeshInstanceState
{
    rh::engine::ArrayProxy<MaterialData> Materials;
    rh::engine::ArrayProxy<DrawCallInfo> DrawCalls;
    static MeshInstanceState             Deserialize( MemoryReader &reader );
};

} // namespace rh::rw::engine
