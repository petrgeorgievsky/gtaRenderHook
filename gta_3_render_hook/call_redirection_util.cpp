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
    struct GameId
    {
        GameId()
        {
            constexpr auto game_start_bseq = 0x53E58955;
            id                             = Version_unknown;
            // Check game version by game's byte start sequence, borrowed from
            // pluginSDK
            if ( *( (uint32_t *)0x5C1E70 ) == game_start_bseq )
                id = Version_1_0_en;
            if ( *( (uint32_t *)0x5C2130 ) == game_start_bseq )
                id = Version_1_1_en;
            if ( *( (uint32_t *)0x5C6FD0 ) == game_start_bseq )
                id = Version_Steam_en;
        }
        uint32_t id;
    };
    static GameId game_id;
    return game_id.id;
}
