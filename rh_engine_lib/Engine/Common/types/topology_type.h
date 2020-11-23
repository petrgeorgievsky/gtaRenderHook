//
// Created by peter on 02.05.2020.
//
#pragma once
#include <cstdint>

namespace rh::engine
{

enum class Topology : uint32_t
{
    TriangleList = 0,
    LineList,
    PointList
};
}