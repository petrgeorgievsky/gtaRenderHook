//
// Created by peter on 10.06.2026.
//

#pragma once
#include <cstdint>

namespace rh::rw::engine::restir
{
struct Reservoir
{
    float totalWeight;
    float selectedLightWeight;
    int   visitedSampleCount;
    int   selectedLightId;
    uint32_t   selectedLightSeed{0};
    uint32_t   pad[3]{};
};
} // namespace rh::rw::engine::restir