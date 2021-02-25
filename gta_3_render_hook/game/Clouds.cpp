//
// Created by peter on 18.01.2021.
//

#include "Clouds.h"
#include "../call_redirection_util.h"
#include <injection_utils/InjectorHelpers.h>

int32_t Clouds::RenderBackground( int16_t, int16_t, int16_t, int16_t, int16_t,
                                  int16_t, int16_t )
{
    return 1;
}

void Clouds::Patch()
{

    // Patch default sky
    RedirectJump( GetAddressByGame( 0x4F7F00, 0x4F7FE0, 0x4F7F70 ),
                  reinterpret_cast<void *>( Clouds::RenderBackground ) );
}
