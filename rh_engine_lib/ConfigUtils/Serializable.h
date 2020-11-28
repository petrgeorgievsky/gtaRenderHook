//
// Created by peter on 27.11.2020.
//
#pragma once
#include <Engine/FastPimpl.h>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace rh::engine
{

class Serializable
{
  public:
    Serializable( const nlohmann::json &impl );
    ~Serializable();

    template <typename T> T    Get( const std::string &name );
    template <typename T> void Set( const std::string &name, T ) {}

  private:
    static constexpr auto JsonSerializerS = 16;
    static constexpr auto JsonSerializerA = 8;
    FastPimpl<nlohmann::json, JsonSerializerA, JsonSerializerS> mImpl;
};

template <> float    Serializable::Get( const std::string &name );
template <> bool     Serializable::Get( const std::string &name );
template <> uint32_t Serializable::Get( const std::string &name );

template <> void Serializable::Set( const std::string &name, float );
template <> void Serializable::Set( const std::string &name, bool );
template <> void Serializable::Set( const std::string &name, uint32_t );
} // namespace rh::engine