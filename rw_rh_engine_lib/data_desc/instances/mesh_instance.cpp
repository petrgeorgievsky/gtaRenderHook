//
// Created by peter on 18.02.2021.
//

#include "mesh_instance.h"
#include <ipc/MemoryReader.h>

namespace rh::rw::engine
{
MeshInstanceState MeshInstanceState::Deserialize( MemoryReader &reader )
{
    MeshInstanceState result{};
    auto              dc_count = *reader.Read<uint64_t>();
    if ( dc_count <= 0 )
        return result;

    auto material_count = *reader.Read<uint64_t>();
    result.Materials    = rh::engine::ArrayProxy(
        reader.Read<MaterialData>( material_count ), material_count );
    result.DrawCalls = rh::engine::ArrayProxy(
        reader.Read<DrawCallInfo>( dc_count ), dc_count );
    return result;
}
} // namespace rh::rw::engine