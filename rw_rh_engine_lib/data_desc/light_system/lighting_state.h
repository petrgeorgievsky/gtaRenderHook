//
// Created by peter on 16.02.2021.
//
#pragma once
#include "point_light.h"
#include <Engine/Common/ArrayProxy.h>
namespace rh::rw::engine
{
class MemoryReader;
struct AnalyticLightsState
{
    rh::engine::ArrayProxy<PointLight> PointLights;
    static AnalyticLightsState         Deserialize( MemoryReader &reader );
};
} // namespace rh::rw::engine