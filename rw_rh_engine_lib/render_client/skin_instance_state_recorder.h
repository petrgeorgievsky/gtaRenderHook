//
// Created by peter on 17.02.2021.
//
#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <span>
#include <vector>

namespace rh::rw::engine
{

struct SkinDrawCallInfo
{
    uint64_t            MeshId;
    uint64_t            DrawCallId;
    uint64_t            MaterialListStart;
    uint64_t            MaterialListCount;
    DirectX::XMFLOAT4X3 WorldTransform;
    DirectX::XMFLOAT4X3 BoneTransform[256];
};

class MemoryWriter;
class MemoryReader;

/**
 * Memory view into current frame's mesh instance list state
 */
struct SkinInstanceState
{
    rh::engine::ArrayProxy<MaterialData>     Materials;
    rh::engine::ArrayProxy<SkinDrawCallInfo> DrawCalls;
    static SkinInstanceState Deserialize( MemoryReader &reader );
};

class SkinInstanceStateRecorder
{
  public:
    SkinInstanceStateRecorder();

    std::span<MaterialData> AllocateDrawCallMaterials( uint64_t count );

    void RecordDrawCall( const SkinDrawCallInfo &info );

    uint64_t Serialize( MemoryWriter &writer ) const;
    void     Flush();

  private:
    std::vector<MaterialData>     MaterialsData;
    std::vector<SkinDrawCallInfo> MeshData;
    uint64_t                      DrawCallCount;
    uint64_t                      MaterialCount;
};

} // namespace rh::rw::engine