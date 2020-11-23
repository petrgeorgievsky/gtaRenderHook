//
// Created by peter on 23.05.2020.
//

#pragma once

#include <cstdint>

class Streaming
{
  public:
    static void      RequestModel( int32_t model, uint32_t flags );
    static uint32_t &mNumModelsRequested;
};
