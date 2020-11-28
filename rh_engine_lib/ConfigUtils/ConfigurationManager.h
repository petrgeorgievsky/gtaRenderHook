//
// Created by peter on 12.06.2020.
//
#pragma once
#include <cassert>
#include <string>
#include <vector>

namespace rh::engine
{
class ConfigBlock;

class ConfigurationManager
{
  public:
    static ConfigurationManager &Instance();

    bool LoadFromFile( const std::string &path );
    void SaveToFile( const std::string &path );
    void AddConfigBlock( ConfigBlock *block );

  private:
    std::vector<ConfigBlock *> mConfigBlocks;
};

} // namespace rh::engine