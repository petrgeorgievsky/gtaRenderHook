#include "game.h"
#include <DebugUtils/DebugLogger.h>
#include <iostream>
uint32_t &CGame::TimeMillisecondsFromStart = *reinterpret_cast<uint32_t *>( 0xB72CA8 );

void CGame::Process()
{
    static uint32_t FPTR = 0x53BEE0;
    reinterpret_cast<decltype( &CGame::Process )>( FPTR )();
}
