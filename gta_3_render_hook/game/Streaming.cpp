//
// Created by peter on 23.05.2020.
//

#include "Streaming.h"
#include "../call_redirection_util.h"
uint32_t &Streaming::mNumModelsRequested =
    *reinterpret_cast<uint32_t *>( 0x08E2CC4 );

void Streaming::RequestModel( int32_t model, uint32_t flags )
{
    InMemoryFuncCall<void, 0x407EA0>( model, flags );
}
