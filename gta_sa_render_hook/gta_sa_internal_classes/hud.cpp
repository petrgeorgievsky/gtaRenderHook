#include "hud.h"

void CHud::DrawAfterFade()
{
    static auto f_ptr = 0x58D490;
    return reinterpret_cast<decltype( &CHud::DrawAfterFade )>( f_ptr )();
}
