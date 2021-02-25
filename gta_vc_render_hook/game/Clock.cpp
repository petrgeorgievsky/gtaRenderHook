//
// Created by peter on 08.12.2020.
//

#include "Clock.h"
#include <injection_utils/InjectorHelpers.h>

bool Clock::GetIsTimeInRange( unsigned char from, unsigned char to )
{
    return InMemoryFuncCall<bool>( 0x4870F0, from, to );
}
