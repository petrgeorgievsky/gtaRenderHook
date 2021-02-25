//
// Created by peter on 08.12.2020.
//

#include "Clock.h"
#include "../call_redirection_util.h"
#include <injection_utils/InjectorHelpers.h>
bool Clock::GetIsTimeInRange( unsigned char from, unsigned char to )
{
    return InMemoryFuncCall<bool>(
        GetAddressByGame( 0x473420, 0x473420, 0x473420 ), from, to );
}
