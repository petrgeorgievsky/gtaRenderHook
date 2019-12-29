#include "timer.h"

uint32_t CTimer::GetTimeMillisecondsFromStart()
{
    static uint32_t FPTR = 0x53BAD0;
    return reinterpret_cast<uint32_t( __cdecl * )()>( FPTR )();
}

void CTimer::Update()
{
    static uint32_t FPTR = 0x561B10;
    reinterpret_cast<void( __cdecl * )()>( FPTR )();
}
