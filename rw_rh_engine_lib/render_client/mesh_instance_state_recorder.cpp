//
// Created by peter on 17.02.2021.
//

#include "mesh_instance_state_recorder.h"

#include <ipc/MemoryWriter.h>

namespace rh::rw::engine
{

MeshInstanceStateRecorder::MeshInstanceStateRecorder()
{
    MeshData.resize( 10000 );
    DrawCallCount = 0;
    MaterialsData.resize( 20000 );
    MaterialCount = 0;
}

std::span<MaterialData>
MeshInstanceStateRecorder::AllocateDrawCallMaterials( uint64_t count )
{
    if ( MaterialsData.size() < MaterialCount + count )
        MaterialsData.resize( MaterialCount + count );

    MeshData[DrawCallCount].MaterialListStart = MaterialCount;
    MeshData[DrawCallCount].MaterialListCount = count;
    return std::span<MaterialData>( &MaterialsData[MaterialCount], count );
}

void MeshInstanceStateRecorder::RecordDrawCall( const DrawCallInfo &info )
{
    auto count              = MeshData[DrawCallCount].MaterialListCount;
    MeshData[DrawCallCount] = info;
    MeshData[DrawCallCount].MaterialListStart = MaterialCount;
    MeshData[DrawCallCount].MaterialListCount = count;
    MaterialCount += MeshData[DrawCallCount].MaterialListCount;
    DrawCallCount++;
}

void MeshInstanceStateRecorder::Flush()
{
    DrawCallCount = 0;
    MaterialCount = 0;
}

uint64_t MeshInstanceStateRecorder::Serialize( MemoryWriter &memory_writer )
{
    /// Serialize schema:
    /// uint64 scene_materials_count
    /// GeometryMaterial scene_materials
    /// uint64 frame_draw_call_count
    /// Im2DDrawCall frame_drawcalls[frame_draw_call_count]

    // serialize drawcalls

    memory_writer.Write( &DrawCallCount );
    if ( DrawCallCount <= 0 )
        return memory_writer.Pos();

    memory_writer.Write( &MaterialCount );
    memory_writer.Write( MaterialsData.data(), MaterialCount );
    memory_writer.Write( MeshData.data(), DrawCallCount );

    return memory_writer.Pos();
}

} // namespace rh::rw::engine