//
// Created by peter on 17.02.2021.
//
#pragma once
#include <data_desc/instances/mesh_instance.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <span>
#include <vector>

namespace rh::rw::engine
{
class MemoryWriter;
class MemoryReader;

class MeshInstanceStateRecorder
{
  public:
    MeshInstanceStateRecorder();

    std::span<MaterialData> AllocateDrawCallMaterials( uint64_t count );

    void     RecordDrawCall( const DrawCallInfo &info );
    uint64_t Serialize( MemoryWriter &memory_writer );
    void     Flush();

  private:
    std::vector<DrawCallInfo> MeshData;
    std::vector<MaterialData> MaterialsData;
    uint64_t                  DrawCallCount;
    uint64_t                  MaterialCount;
};
} // namespace rh::rw::engine