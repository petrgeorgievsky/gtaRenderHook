//
// Created by peter on 29.11.2020.
//
#pragma once
#include <array>
#include <filesystem>
#include <map>
#include <optional>
#include <string>

namespace rh::rw::engine
{

struct MaterialDescription
{
    std::string          mTextureDictName;
    std::array<char, 32> mSpecularTextureName{};
};

class MaterialExtensionSystem
{
  public:
    static MaterialExtensionSystem &GetInstance();

    std::optional<MaterialDescription> GetMatDesc( const std::string &name );

  private:
    MaterialExtensionSystem();
    void ParseMaterialDesc( const std::filesystem::path &mat_desc );

  private:
    std::map<std::string, MaterialDescription> mMaterials;
};

}; // namespace rh::rw::engine