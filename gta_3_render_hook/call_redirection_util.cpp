//
// Created by peter on 06.12.2020.
//
#include "call_redirection_util.h"
#include <DebugUtils/DebugLogger.h>

uint32_t GetAddressByGame( uint32_t v1_0, uint32_t v1_1,
                           uint32_t v_steam ) noexcept
{
    auto g_id = GetGameId();
    if ( g_id == Version_1_0_en )
        return v1_0;
    else if ( g_id == Version_1_1_en )
        return v1_1;
    else if ( g_id == Version_Steam_en )
        return v_steam;

    rh::debug::DebugLogger::Error( "Unknown GTA 3 version:Please use 1.0, 1.1 "
                                   "or steam version of the GTA 3!" );
    std::terminate();
}
uint32_t GetGameId() noexcept
{
    static uint32_t game_id = Version_unknown;
    if ( game_id != Version_unknown )
        return game_id;

    if ( *( (uint32_t *)0x5C1E70 ) == 0x53E58955 )
        return ( game_id = Version_1_0_en );
    if ( *( (uint32_t *)0x5C2130 ) == 0x53E58955 )
        return ( game_id = Version_1_1_en );
    if ( *( (uint32_t *)0x5C6FD0 ) == 0x53E58955 )
        return ( game_id = Version_Steam_en );
    std::terminate();
}
