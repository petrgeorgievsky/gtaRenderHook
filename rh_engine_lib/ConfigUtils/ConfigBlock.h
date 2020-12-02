//
// Created by peter on 12.06.2020.
//
#pragma once
#include <string>
namespace rh::engine
{
class Serializable;
class ConfigBlock
{

  public:
    ConfigBlock()                       = default;
    ~ConfigBlock()                      = default;
    ConfigBlock( const ConfigBlock &c ) = delete;
    virtual std::string Name() { return "Uncategorized"; }
    virtual void        Serialize( Serializable *serializable )   = 0;
    virtual void        Deserialize( Serializable *serializable ) = 0;
};
} // namespace rh::engine