//
// Created by peter on 30.10.2020.
//
#pragma once
#include <cstdint>
#include <system_error>
#include <type_traits>
template <typename func_type, uint32_t address> class InMemoryFunc
{
  public:
    auto operator()() noexcept
    {
        return reinterpret_cast<func_type>( address );
    }
};

template <typename RetType, uint32_t vtable_id, typename C, typename... Args>
auto InMemoryVirtualFunc( C _this, Args... args )
{
    using return_type = RetType( __thiscall * )( C, Args... );
    auto address      = ( *reinterpret_cast<void ***>( _this ) )[vtable_id];
    if constexpr ( std::is_void_v<RetType> )
        reinterpret_cast<return_type>( address )( _this, args... );
    else
        return reinterpret_cast<return_type>( address )( _this, args... );
}

template <typename RetType, typename... Args>
auto InMemoryFuncCall( uint32_t address, Args... args )
{
    using return_type = RetType( __cdecl * )( Args... );
    if constexpr ( std::is_void_v<RetType> )
        reinterpret_cast<return_type>( address )( args... );
    else
        return reinterpret_cast<return_type>( address )( args... );
}
template <typename RetType, typename... Args>
auto InMemoryThisCall( uint32_t address, Args... args )
{
    using return_type = RetType( __thiscall * )( Args... );
    if constexpr ( std::is_void_v<RetType> )
        reinterpret_cast<return_type>( address )( args... );
    else
        return reinterpret_cast<return_type>( address )( args... );
}

constexpr uint32_t Version_unknown  = 0xF;
constexpr uint32_t Version_1_0_en   = 0;
constexpr uint32_t Version_1_1_en   = 1;
constexpr uint32_t Version_Steam_en = 2;

uint32_t GetGameId() noexcept;

uint32_t GetAddressByGame( uint32_t v1_0, uint32_t v1_1,
                           uint32_t v_steam ) noexcept;