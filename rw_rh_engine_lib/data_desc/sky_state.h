//
// Created by peter on 17.02.2021.
//
#pragma once

namespace rh::rw::engine
{
struct SkyState
{
    float mSkyTopColor[4];
    float mSkyBottomColor[4];
    float mSunDir[4];
    float mAmbientColor[4];
};
} // namespace rh::rw::engine