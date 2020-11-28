//
// Created by peter on 12.06.2020.
//

#include "ConfigurationManager.h"
#include "ConfigBlock.h"
#include "Serializable.h"
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
        if ( !settings.contains( cfg_block->Name() ) )
            continue;
        Serializable s( settings[cfg_block->Name()] );
        cfg_block->Deserialize( &s );
    }
    return true;
}

void ConfigurationManager::SaveToFile( const std::string &path )
{
    using namespace nlohmann;

    json settings;
    for ( const auto &cfg_block : mConfigBlocks )
    {
        Serializable s( settings[cfg_block->Name()] );
        cfg_block->Serialize( &s );
    }
    std::ofstream file( path );
    file << settings;
}

void ConfigurationManager::AddConfigBlock( ConfigBlock *block )
{
    mConfigBlocks.push_back( block );
}

} // namespace rh::engine