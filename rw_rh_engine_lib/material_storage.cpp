//
// Created by peter on 29.11.2020.
//

#include "material_storage.h"
#include "common_headers.h"
#include <DebugUtils/DebugLogger.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

namespace rh::rw::engine
{

MaterialExtensionSystem &MaterialExtensionSystem::GetInstance()
{
    static MaterialExtensionSystem m;
    return m;
}

std::optional<MaterialDescription>
MaterialExtensionSystem::GetMatDesc( const std::string_view &name )
{
    auto x = mMaterials.find( name );
    return x != mMaterials.end() ? x->second
                                 : std::optional<MaterialDescription>{};
}

MaterialExtensionSystem::MaterialExtensionSystem()
{
    namespace fs  = std::filesystem;
    auto dir_path = fs::current_path() / "materials";
    if ( !fs::exists( dir_path ) )
        return;
    for ( auto &p : fs::directory_iterator( dir_path ) )
    {
        const fs::path &file_path = p.path();
        if ( file_path.extension() != ".mat" )
            continue;
        ParseMaterialDesc( file_path );
    }
}

void MaterialExtensionSystem::ParseMaterialDesc(
    const std::filesystem::path &mat_desc )
{
    std::ifstream filestream( mat_desc );
    if ( !filestream.is_open() )
    {
        rh::debug::DebugLogger::ErrorFmt(
            "Failed to open material description at %s", mat_desc.c_str() );
        return;
    }
    auto &desc = mMaterials[mat_desc.stem().generic_string()];
    try
    {
        nlohmann::json desc_json{};
        filestream >> desc_json;

        auto tex_name = desc_json.contains( "spec_tex" )
                            ? desc_json["spec_tex"].get<std::string>()
                            : "";

        desc.mSpecularTextureName.fill( 0 );
        std::copy( tex_name.begin(), tex_name.end(),
                   desc.mSpecularTextureName.begin() );

        desc.mTextureDictName =
            desc_json.contains( "tex_dict_slot_name" )
                ? desc_json["tex_dict_slot_name"].get<std::string>()
                : "";
        desc.IsEmissive    = desc_json.contains( "emissive" )
                                 ? desc_json["emissive"].get<bool>()
                                 : false;
        desc.EmissionValue = desc_json.contains( "emission" )
                                 ? desc_json["emission"].get<float>()
                                 : 1.0f;
    }
    catch ( const std::exception &ex )
    {
        rh::debug::DebugLogger::ErrorFmt(
            "Failed to parse material description %s", ex.what() );
    }
}

RwTexture *MaterialExtensionSystem::ReadTexture( std::string_view dictionary,
                                                 std::string_view name )
{
    return mReadTextureCallback ? mReadTextureCallback( dictionary, name )
                                : nullptr;
}
void MaterialExtensionSystem::RegisterReadTextureCallback(
    MaterialExtensionSystem::ReadTextureCallback cb )
{
    mReadTextureCallback = std::move( cb );
}

} // namespace rh::rw::engine