//
// Created by peter on 10.06.2026.
//

#pragma once

namespace rh::rw::engine::restir
{
struct Reservoir
{
    float totalWeight;
    float selectedLightWeight;
    int   visitedSampleCount;
    int   selectedLightId;
};
} // namespace rh::rw::engine::restir