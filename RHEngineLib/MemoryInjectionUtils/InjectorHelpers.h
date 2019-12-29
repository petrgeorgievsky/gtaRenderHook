#pragma once
#include "common.h"

static void Patch( INT_PTR address, void* data, SIZE_T size )
{
    auto adr_ptr = reinterpret_cast<void*>( address );

    unsigned long protect[2];
    VirtualProtect( adr_ptr, size, PAGE_EXECUTE_READWRITE, &protect[0] );
    memcpy( adr_ptr, data, size );
    VirtualProtect( adr_ptr, size, protect[0], &protect[1] );
}

/**
 * \brief
 * \param address
 * \param value
 */
static void SetPointer( INT_PTR address, void *value )
{
    Patch( address, &value, sizeof( void* ) );
}
inline static void SetInt( INT_PTR address, int value )
{
    Patch( address, &value, sizeof( void* ) );
}

inline static void RedirectCall( INT_PTR address, void *func )
{
    auto func_iptr = reinterpret_cast<INT_PTR>( func );

    INT_PTR temp = 0xE8;

    Patch( address, &temp, sizeof( unsigned char ) );
    temp = func_iptr - ( address + 5 );
    Patch( ( address + 1 ), &temp, sizeof( void* ) );
}

inline static void RedirectJump( INT_PTR address, void *func )
{
    auto func_iptr = reinterpret_cast<INT_PTR>( func );

    INT_PTR temp = 0xE9;

    Patch( address, &temp, sizeof( unsigned char ) );
    temp = func_iptr - ( address + 5 );
    Patch( ( address + 1 ), &temp, sizeof( void* ) );
}

inline static void Nop( INT_PTR address, SIZE_T size )
{
    auto adr_ptr = reinterpret_cast<void*>( address );

    unsigned long protect[2];
    VirtualProtect( adr_ptr, size, PAGE_EXECUTE_READWRITE, &protect[0] );
    memset( adr_ptr, 0x90, size );
    VirtualProtect( adr_ptr, size, protect[0], &protect[1] );
}
