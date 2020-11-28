//
// Created by peter on 27.11.2020.
//

#include "Serializable.h"
#include <nlohmann/json.hpp>

namespace rh::engine
{

Serializable::~Serializable() = default;
Serializable::Serializable( const nlohmann::json &impl ) : mImpl( impl ) {}

template <> float Serializable::Get( const std::string &name )
{
    return mImpl->at( name ).get<float>();
}

template <> bool Serializable::Get( const std::string &name )
{
    return mImpl->at( name ).get<bool>();
}
template <> uint32_t Serializable::Get( const std::string &name )
{
    return mImpl->at( name ).get<uint32_t>();
}

template <> void Serializable::Set( const std::string &name, float v )
{
    mImpl->at( name ) = v;
}
template <> void Serializable::Set( const std::string &name, bool v )
{
    mImpl->at( name ) = v;
}
template <> void Serializable::Set( const std::string &name, uint32_t v )
{
    mImpl->at( name ) = v;
}

} // namespace rh::engine