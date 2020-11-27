//
// Created by peter on 12.06.2020.
//
#pragma once
#include <cassert>
#include <memory>
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
class JsonSerializer;
class Serializable
{
  public:
    Serializable( std::unique_ptr<JsonSerializer> impl );
    ~Serializable();

    template <typename T> T    Get( const std::string &name );
    template <typename T> void Set( const std::string &name, T );

  private:
    std::unique_ptr<JsonSerializer> mImpl;
};

class ConfigBlock
{

  public:
    virtual std::string Name() { return "Uncategorized"; }
    virtual void        Serialize( Serializable *serializable )   = 0;
    virtual void        Deserialize( Serializable *serializable ) = 0;
};

template <> float    Serializable::Get( const std::string &name );
template <> bool     Serializable::Get( const std::string &name );
template <> uint32_t Serializable::Get( const std::string &name );

template <> void Serializable::Set( const std::string &name, float );
template <> void Serializable::Set( const std::string &name, bool );
template <> void Serializable::Set( const std::string &name, uint32_t );

} // namespace rh::engine