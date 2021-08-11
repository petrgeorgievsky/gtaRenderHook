//
// Created by peter on 11.08.2021.
//

#pragma once
#include "point_light.h"
#include "triangle_light.h"

namespace rh::rw::engine
{

struct PackedLight
{
    union
    {
        PointLight    Point{};
        TriangleLight Triangle;
    };
};
} // namespace rh::rw::engine
