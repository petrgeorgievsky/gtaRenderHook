//
// Created by peter on 16.02.2021.
//

#pragma once

#include <cstdint>

namespace rh::rw::engine
{

// Capacity of the GPU point-light buffers. Shaders index them with the light
// count coming from the scene, so every consumer has to clamp it.
constexpr uint32_t MaxPointLights = 1024;

// TODO: Try separating spot and point lights, may improve perf
struct PointLight
{
    float mPos[3];
    float mRadius;
    float mDir[3];
    float mSpotCutoff;
    float mColor[4];
};
} // namespace rh::rw::engine