//
// Created by peter on 06.12.2020.
//

#pragma once
#include <cstdint>

class TxdStore
{
  public:
    static int32_t FindTxdSlot( const char *name );
    static void    PushCurrentTxd();
    static void    PopCurrentTxd();
    static void    SetCurrentTxd( int32_t id );

    // static void LoadTxd( int32_t id );
};