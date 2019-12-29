#pragma once
static void Patch( void* address, void* data, SIZE_T size )
{
    unsigned long protect[2];
    VirtualProtect( address, size, PAGE_EXECUTE_READWRITE, &protect[0] );
    memcpy( address, data, size );
    VirtualProtect( address, size, protect[0], &protect[1] );
}

/**
 * \brief
 * \param address
 * \param value
 */
static void SetPointer( INT_PTR address, void *value )
{
    Patch( (void *)address, &value, sizeof( void* ) );
}
inline static void SetInt( INT_PTR address, int value )
{
    Patch( (void *)address, &value, sizeof( void* ) );
}
inline static void RedirectCall( INT_PTR address, void *func )
{
    INT_PTR temp = 0xE8;
    Patch( (void *)address, &temp, sizeof( unsigned char ) );
    temp = (INT_PTR)func - ( (INT_PTR)address + 5 );
    Patch( (void *)( (INT_PTR)address + 1 ), &temp, sizeof( void* ) );
}
inline static void RedirectJump( INT_PTR address, void *func )
{
    INT_PTR temp = 0xE9;
    Patch( (void *)address, &temp, sizeof( unsigned char ) );
    temp = (INT_PTR)func - ( (INT_PTR)address + 5 );
    Patch( (void *)( (INT_PTR)address + 1 ), &temp, sizeof( void* ) );
}
inline static void Nop( INT_PTR address, SIZE_T size )
{
    unsigned long protect[2];
    VirtualProtect( (void *)address, size, PAGE_EXECUTE_READWRITE, &protect[0] );
    memset( (void *)address, 0x90, size );
    VirtualProtect( (void *)address, size, protect[0], &protect[1] );
}