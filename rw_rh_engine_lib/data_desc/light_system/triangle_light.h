//
// Created by peter on 07.08.2021.
//

#pragma once
#include <cstdint>

namespace rh::rw::engine
{

struct TriangleLight
{
    float                  V0[3];
    int32_t                InstanceId;
    float                  V1[3];
    float                  Intensity;
    float                  V2[3];
    [[maybe_unused]] float Padd;
};

} // namespace rh::rw::engine