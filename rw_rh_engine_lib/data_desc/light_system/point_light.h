//
// Created by peter on 16.02.2021.
//

#pragma once

namespace rh::rw::engine
{
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