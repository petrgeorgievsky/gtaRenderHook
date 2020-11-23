#pragma once
#include <stdint.h>
class CTimer
{
public:
    static uint32_t GetTimeMillisecondsFromStart();
    static void Update();
};
