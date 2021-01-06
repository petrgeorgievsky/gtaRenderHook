//
// Created by peter on 29.11.2020.
//
#pragma once
#include <array>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>

struct RwTexture;
namespace rh::rw::engine
{

struct MaterialDescription
{
    std::string          mTextureDictName;
    std::array<char, 32> mSpecularTextureName{};
};

class MaterialExtensionSystem
{
    using ReadTextureCallback = std::function<RwTexture *(
        std::string_view dict, std::string_view name )>;

  public:
    static MaterialExtensionSystem &GetInstance();

    std::optional<MaterialDescription>
    GetMatDesc( const std::string_view &name );

    RwTexture *ReadTexture( std::string_view dictionary,
                            std::string_view name );

    void RegisterReadTextureCallback( ReadTextureCallback cb );

  private:
    MaterialExtensionSystem();
    void ParseMaterialDesc( const std::filesystem::path &mat_desc );

  private:
    std::map<std::string, MaterialDescription, std::less<>> mMaterials;
    ReadTextureCallback mReadTextureCallback;
};

}; // namespace rh::rw::engine