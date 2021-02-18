//
// Created by peter on 18.02.2021.
//
#include "lighting_state.h"
#include <ipc/MemoryReader.h>

namespace rh::rw::engine
{
AnalyticLightsState AnalyticLightsState::Deserialize( MemoryReader &reader )
{
    AnalyticLightsState result{};
    auto                point_light_count = *reader.Read<uint64_t>();
    if ( point_light_count <= 0 )
        return result;
    result.PointLights = rh::engine::ArrayProxy(
        reader.Read<PointLight>( point_light_count ), point_light_count );
    return result;
}
} // namespace rh::rw::engine