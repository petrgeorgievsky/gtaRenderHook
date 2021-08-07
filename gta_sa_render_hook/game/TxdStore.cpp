//
// Created by peter on 07.08.2021.
//

#include "TxdStore.h"
#include <injection_utils/InjectorHelpers.h>

int32_t TxdStore::FindTxdSlot( const char *name )
{
    return InMemoryFuncCall<int32_t>( 0x731850, name );
}
void TxdStore::PushCurrentTxd() { return InMemoryFuncCall<void>( 0x7316A0 ); }
void TxdStore::PopCurrentTxd() { return InMemoryFuncCall<void>( 0x7316B0 ); }
void TxdStore::SetCurrentTxd( int32_t id )
{
    return InMemoryFuncCall<void>( 0x7319C0, id );
}
