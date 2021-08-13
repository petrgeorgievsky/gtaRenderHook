//
// Created by peter on 16.02.2021.
//

#include "light_state_recorder.h"
#include <ipc/MemoryReader.h>
#include <ipc/MemoryWriter.h>
namespace rh::rw::engine
{
constexpr size_t gLightBufferSize = 1024;

LightStateRecorder::LightStateRecorder() noexcept
{
    PointLights.resize( gLightBufferSize );
    PointLightCount = 0;
}

LightStateRecorder::~LightStateRecorder() noexcept = default;

uint64_t LightStateRecorder::Serialize( MemoryWriter &writer )
{
    uint64_t point_light_count = PointLightCount;
    writer.Write( &point_light_count );
    if ( point_light_count > 0 )
        writer.Write( PointLights.data(), point_light_count );
    return 0;
}

void LightStateRecorder::Flush() { PointLightCount = 0; }

void LightStateRecorder::RecordPointLight( PointLight &&light )
{
    if ( PointLightCount >= gLightBufferSize )
        return;
    PointLights[PointLightCount] = light;
    PointLightCount++;
}

} // namespace rh::rw::engine