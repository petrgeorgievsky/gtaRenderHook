#pragma once
#include <stdint.h>
class CGame
{
public:
    static void Process();
    static uint32_t &TimeMillisecondsFromStart;
};
