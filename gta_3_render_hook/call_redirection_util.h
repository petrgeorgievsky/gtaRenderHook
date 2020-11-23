//
// Created by peter on 30.10.2020.
//
#pragma once
#include <cstdint>
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

template <typename RetType, uint32_t address, typename... Args>
auto InMemoryFuncCall( Args... args )
{
    using return_type = RetType( __cdecl * )( Args... );
    if constexpr ( std::is_void_v<RetType> )
        reinterpret_cast<return_type>( address )( args... );
    else
        return reinterpret_cast<return_type>( address )( args... );
}