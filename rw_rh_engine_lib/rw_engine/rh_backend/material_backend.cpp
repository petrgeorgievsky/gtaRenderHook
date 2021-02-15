//
// Created by peter on 08.05.2020.
//

#include "material_backend.h"
#include "raster_backend.h"
#include <material_storage.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
namespace rh::rw::engine
{

MaterialData ConvertMaterialData( RpMaterial *material )
{
    int32_t tex_id      = BackendRasterPlugin::NullRasterId;
    int32_t spec_tex_id = BackendRasterPlugin::NullRasterId;
    if ( material == nullptr )
        return MaterialData{ tex_id, RwRGBA{ 255, 0, 0, 255 }, spec_tex_id, 0 };

    auto &m_b = BackendMaterialPlugin::GetData( material );

    if ( material->texture && material->texture->raster )
    {
        auto &raster =
            BackendRasterPlugin::GetData( material->texture->raster );
        tex_id = raster.mImageId;
    }
    auto spec_tex = m_b.mSpecTex;
    if ( spec_tex && spec_tex->raster )
    {
        auto &raster = BackendRasterPlugin::GetData( spec_tex->raster );
        spec_tex_id  = raster.mImageId;
    }
    return MaterialData{ tex_id, material->color, spec_tex_id,
                         material->surfaceProps.specular };
}

int32_t BackendMaterialPlugin::Offset = -1;

BackendMaterialPlugin::BackendMaterialPlugin( const PluginPtrTable &plugin_cb )
{
    auto ctor = []( void *object, int32_t offsetInObject,
                    int32_t sizeInObject ) {
        auto ext = GetAddress( static_cast<RpMaterial *>( object ) );
        new ( ext ) BackendMaterialExt;
        return object;
    };
    auto dtor = []( void *object, int32_t offsetInObject,
                    int32_t sizeInObject ) {
        auto &ext = GetData( static_cast<RpMaterial *>( object ) );
        ext.~BackendMaterialExt();
        return object;
    };
    // Register plugin
    Offset = plugin_cb.MaterialRegisterPlugin( sizeof( BackendMaterialExt ),
                                               rwID_MATERIAL_BACKEND, ctor,
                                               dtor, nullptr );
    assert( Offset > 0 );

    // Register stream callbacks
    if ( plugin_cb.MaterialSetStreamAlwaysCallBack )
    {
        plugin_cb.MaterialSetStreamAlwaysCallBack( rwID_MATERIAL_BACKEND,
                                                   StreamAlwaysCallback );
    }
}

uint8_t *BackendMaterialPlugin::GetAddress( RpMaterial *material )
{
    assert( material && Offset > 0 );
    return ( reinterpret_cast<uint8_t *>( material ) ) + Offset;
}

BackendMaterialExt &BackendMaterialPlugin::GetData( RpMaterial *material )
{
    return *reinterpret_cast<BackendMaterialExt *>( GetAddress( material ) );
}

int32_t BackendMaterialPlugin::StreamAlwaysCallback(
    void *object, [[maybe_unused]] int32_t offsetInObject,
    [[maybe_unused]] int32_t sizeInObject )
{
    auto *material = static_cast<RpMaterial *>( object );
    auto &ext      = GetData( material );

    if ( !material->texture )
        return 1;

    // TODO: probably extension system should live on render driver side, need
    // to investigate performance
    auto &m_ext_sys = MaterialExtensionSystem::GetInstance();

    auto texture_name = std::string_view( material->texture->name );
    auto mat_desc     = m_ext_sys.GetMatDesc( texture_name );
    if ( !mat_desc )
        return 1;

    ext.mSpecTex = m_ext_sys.ReadTexture(
        mat_desc->mTextureDictName,
        std::string_view( mat_desc->mSpecularTextureName.data() ) );
    return 1;
}

int32_t BackendMaterialPlugin::CallAlwaysCb( RpMaterial *material )
{
    return StreamAlwaysCallback( material, Offset,
                                 sizeof( BackendMaterialExt ) );
}

} // namespace rh::rw::engine