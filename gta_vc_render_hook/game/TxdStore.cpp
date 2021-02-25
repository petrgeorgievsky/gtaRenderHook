#include "TxdStore.h"
//#include "../call_redirection_util.h"
#include <injection_utils/InjectorHelpers.h>

int32_t TxdStore::FindTxdSlot( const char *name )
{
    return InMemoryFuncCall<int32_t>( 0x580D70, name );
}
void TxdStore::PushCurrentTxd() { return InMemoryFuncCall<void>( 0x580AC0 ); }
void TxdStore::PopCurrentTxd() { return InMemoryFuncCall<void>( 0x580AA0 ); }
void TxdStore::SetCurrentTxd( int32_t id )
{
    return InMemoryFuncCall<void>( 0x580AD0, id );
}
