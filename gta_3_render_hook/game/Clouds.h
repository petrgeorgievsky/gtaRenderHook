//
// Created by peter on 18.01.2021.
//
#pragma once

#include <cstdint>
class Clouds
{
  public:
    static int32_t RenderBackground( int16_t topred, int16_t topgreen,
                                     int16_t topblue, int16_t botred,
                                     int16_t botgreen, int16_t botblue,
                                     int16_t alpha );
    /// Removes unnecessary 2D sky quad.
    static void Patch();
};