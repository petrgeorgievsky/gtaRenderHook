//
// Created by peter on 12.06.2020.
//

#include "ConfigurationManager.h"
#include <nlohmann/json.hpp>

#include <fstream>

namespace rh::engine
{

ConfigurationManager &ConfigurationManager::Instance()
{
    static ConfigurationManager config_mgr{};
    return config_mgr;
}

bool ConfigurationManager::LoadFromFile( const std::string &path )
{
    using namespace nlohmann;
    json          settings;
    std::ifstream file( path );
    if ( !file.is_open() )
        return false;
    file >> settings;
    for ( auto &cfg_block : mConfigBlocks )
    {
        if ( settings.contains( cfg_block->Name() ) )
        {
            Serializable s( std::make_unique<JsonSerializer>(
                settings[cfg_block->Name()] ) );
            cfg_block->Deserialize( &s );
        }
    }
    return true;
}

void ConfigurationManager::SaveToFile( const std::string &path )
{
    using namespace nlohmann;

    json settings;
    for ( const auto &cfg_block : mConfigBlocks )
    {
        Serializable s(
            std::make_unique<JsonSerializer>( settings[cfg_block->Name()] ) );
        cfg_block->Serialize( &s );
    }
    std::ofstream file( path );
    file << settings;
}

class JsonSerializer
{
  public:
    explicit JsonSerializer( nlohmann::json &data ) : mImpl( data ) {}
    friend class Serializable;

  private:
    nlohmann::json &mImpl;
};

void ConfigurationManager::AddConfigBlock( ConfigBlock *block )
{
    mConfigBlocks.push_back( block );
}

Serializable::~Serializable() = default;
Serializable::Serializable( std::unique_ptr<JsonSerializer> impl )
    : mImpl( std::move( impl ) )
{
}

template <> float Serializable::Get( const std::string &name )
{
    return mImpl->mImpl[name].get<float>();
}

template <> bool Serializable::Get( const std::string &name )
{
    return mImpl->mImpl[name].get<bool>();
}
template <> uint32_t Serializable::Get( const std::string &name )
{
    return mImpl->mImpl[name].get<uint32_t>();
}

template <> void Serializable::Set( const std::string &name, float v )
{
    mImpl->mImpl[name] = v;
}
template <> void Serializable::Set( const std::string &name, bool v )
{
    mImpl->mImpl[name] = v;
}
template <> void Serializable::Set( const std::string &name, uint32_t v )
{
    mImpl->mImpl[name] = v;
}

} // namespace rh::engine