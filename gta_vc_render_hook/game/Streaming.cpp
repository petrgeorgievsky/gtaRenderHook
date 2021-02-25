//
// Created by peter on 23.05.2020.
//

#include "Streaming.h"
#include <injection_utils/InjectorHelpers.h>

uint32_t &Streaming::mNumModelsRequested =
    *reinterpret_cast<uint32_t *>( 0x975354 );

void Streaming::RequestModel( int32_t model, uint32_t flags )
{

    InMemoryFuncCall<void>( 0x40E310, model, flags );
}
