//
// Created by peter on 23.05.2020.
//

#include "Streaming.h"
#include "../call_redirection_util.h"
uint32_t &Streaming::mNumModelsRequested = *reinterpret_cast<uint32_t *>(
    GetAddressByGame( 0x8E2C10, 0x8E2CC4, 0x8F2E04 ) );

void Streaming::RequestModel( int32_t model, uint32_t flags )
{

    InMemoryFuncCall<void>( GetAddressByGame( 0x407EA0, 0x407EA0, 0x407EA0 ),
                            model, flags );
}
