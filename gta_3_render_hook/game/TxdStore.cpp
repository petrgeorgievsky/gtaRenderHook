//
// Created by peter on 06.12.2020.
//

#include "TxdStore.h"
#include "../call_redirection_util.h"
int32_t TxdStore::FindTxdSlot( const char *name )
{
    return InMemoryFuncCall<int32_t>(
        GetAddressByGame( 0x5275D0, 0x527810, 0x5277A0 ), name );
}
void TxdStore::PushCurrentTxd()
{
    return InMemoryFuncCall<void>(
        GetAddressByGame( 0x527900, 0x527B40, 0x527AD0 ) );
}
void TxdStore::PopCurrentTxd()
{
    return InMemoryFuncCall<void>(
        GetAddressByGame( 0x527910, 0x527B50, 0x527AE0 ) );
}
void TxdStore::SetCurrentTxd( int32_t id )
{
    return InMemoryFuncCall<void>(
        GetAddressByGame( 0x5278C0, 0x527B00, 0x527A90 ), id );
}
