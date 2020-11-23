#include "audioengine.h"

CAudioEngine *gAudioEngine = reinterpret_cast<CAudioEngine *>( 0xB6BC90 );

void CAudioEngine::Service()
{
    static auto f_ptr = 0x507750;
    return reinterpret_cast<void( __thiscall * )( CAudioEngine * )>( f_ptr )( this );
}
