//
// Created by peter on 17.02.2021.
//

#include "skin_instance_state_recorder.h"

#include <ipc/MemoryReader.h>
#include <ipc/MemoryWriter.h>

namespace rh::rw::engine
{

constexpr size_t gSkinDrawCallLimit = 100;
constexpr size_t gSkinMaterialLimit = 2000;

SkinInstanceStateRecorder::SkinInstanceStateRecorder()
{
    MeshData.resize( gSkinDrawCallLimit );
    MaterialsData.resize( gSkinMaterialLimit );
    DrawCallCount = 0;
    MaterialCount = 0;
}

std::span<MaterialData>
SkinInstanceStateRecorder::AllocateDrawCallMaterials( uint64_t count )
{
    if ( MaterialsData.size() < MaterialCount + count )
        MaterialsData.resize( MaterialCount + count );

    MeshData[DrawCallCount].MaterialListStart = MaterialCount;
    MeshData[DrawCallCount].MaterialListCount = count;

    return std::span<MaterialData>( &MaterialsData[MaterialCount], count );
}

void SkinInstanceStateRecorder::RecordDrawCall( const SkinDrawCallInfo &info )
{
    assert( DrawCallCount < 100 );
    auto mat_count          = MeshData[DrawCallCount].MaterialListCount;
    MeshData[DrawCallCount] = info;
    MeshData[DrawCallCount].MaterialListStart = MaterialCount;
    MeshData[DrawCallCount].MaterialListCount = mat_count;
    MaterialCount += mat_count;
    DrawCallCount++;
}

uint64_t SkinInstanceStateRecorder::Serialize( MemoryWriter &writer ) const
{
    /// Serialize schema:
    /// uint64 skip_offset
    /// uint64 frame_draw_call_count
    /// uint64 material_count
    /// uint64 materials
    /// Im2DDrawCall frame_drawcalls[frame_draw_call_count]

    // serialize drawcalls
    writer.Write( &DrawCallCount );

    if ( DrawCallCount <= 0 )
    {
        return writer.Pos();
    }
    writer.Write( &MaterialCount );
    writer.Write( MaterialsData.data(), MaterialCount );
    writer.Write( MeshData.data(), DrawCallCount );

    return writer.Pos();
}

SkinInstanceState SkinInstanceState::Deserialize( MemoryReader &reader )
{
    SkinInstanceState result{};
    auto              dc_count = *reader.Read<uint64_t>();
    if ( dc_count <= 0 )
        return result;

    auto material_count = *reader.Read<uint64_t>();
    result.Materials    = rh::engine::ArrayProxy(
           reader.Read<MaterialData>( material_count ), material_count );
    result.DrawCalls = rh::engine::ArrayProxy(
        reader.Read<SkinDrawCallInfo>( dc_count ), dc_count );
    return result;
}

void SkinInstanceStateRecorder::Flush()
{
    DrawCallCount = 0;
    MaterialCount = 0;
}
} // namespace rh::rw::engine