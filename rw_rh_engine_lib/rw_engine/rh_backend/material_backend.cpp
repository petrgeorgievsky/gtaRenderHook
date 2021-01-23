//
// Created by peter on 08.05.2020.
//

#include "material_backend.h"
#include "raster_backend.h"
#include <material_storage.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
namespace rh::rw::engine
{
int32_t gBackendMaterialExtOffset = 0;
int32_t engine::BackendMaterialPluginAttach()
{
    gBackendMaterialExtOffset =
        DeviceGlobals::PluginFuncs.MaterialRegisterPlugin(
            sizeof( BackendMaterialExt ), rwID_MATERIAL_BACKEND,
            BackendMaterialCtor, BackendMaterialDtor, nullptr );

    if ( DeviceGlobals::PluginFuncs.MaterialSetStreamAlwaysCallBack )
        DeviceGlobals::PluginFuncs.MaterialSetStreamAlwaysCallBack(
            rwID_MATERIAL_BACKEND, BackendMaterialStreamAlwaysCallback );
    return gBackendMaterialExtOffset > 0;
}

void *BackendMaterialCtor( void *object, int32_t offsetInObject,
                           int32_t sizeInObject )
{
    auto ext = GetBackendMaterialExt( static_cast<RpMaterial *>( object ) );
    ext->mMaterialId = 0xBADF00D;
    ext->mSpecTex    = nullptr;
    return object;
}

void *BackendMaterialDtor( void *object, int32_t offsetInObject,
                           int32_t sizeInObject )
{
    auto ext = GetBackendMaterialExt( static_cast<RpMaterial *>( object ) );
    if ( ext->mMaterialId == 0xBADF00D )
        return object;
    /* gRenderClient->GetTaskQueue().ExecuteTask(
         SharedMemoryTaskType::MATERIAL_DELETE,
         [&ext]( MemoryWriter &&memory_writer ) {
             // serialize
             memory_writer.Write( &ext->mMaterialId );
         },
         []( MemoryReader &&memory_reader ) {
             // deserialize
         } );*/
    ext->mMaterialId = 0xBADF00D;
    ext->mSpecTex    = nullptr;
    return object;
}

BackendMaterialExt *GetBackendMaterialExt( RpMaterial *material )
{
    assert( material );
    auto *ext = reinterpret_cast<BackendMaterialExt *>(
        ( reinterpret_cast<uint8_t *>( material ) ) +
        gBackendMaterialExtOffset );

    return ext;
}
uint64_t CreateMaterialData( RpMaterial *material )
{
    auto             ext = GetBackendMaterialExt( material );
    MaterialInitData initData{};

    initData.mTexture = 0xBADF00D;
    if ( material->texture && material->texture->raster )
    {
        auto raster       = GetBackendRasterExt( material->texture->raster );
        initData.mTexture = raster->mImageId;
    }
    initData.mSpecTexture = 0xBADF00D;
    if ( ext->mSpecTex && ext->mSpecTex->raster )
    {
        auto raster           = GetBackendRasterExt( ext->mSpecTex->raster );
        initData.mSpecTexture = raster->mImageId;
    }
    initData.specular    = material->surfaceProps.specular;
    initData.diffuse     = material->surfaceProps.diffuse;
    initData.ambient     = material->surfaceProps.ambient;
    initData.mColor      = material->color;
    uint64_t material_id = 0xBADF00D;
    /*gRenderClient->GetTaskQueue().ExecuteTask(
        SharedMemoryTaskType::MATERIAL_LOAD,
        [&initData]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &initData );
        },
        [&material_id]( MemoryReader &&memory_reader ) {
            // deserialize
            material_id = *memory_reader.Read<uint64_t>();
        } );*/
    return material_id;
}

int32_t BackendMaterialStreamAlwaysCallback( void *object, int32_t, int32_t )
{
    auto *material = static_cast<RpMaterial *>( object );
    auto *ext      = GetBackendMaterialExt( material );

    if ( !material->texture )
        return 1;

    auto &m_ext_sys = MaterialExtensionSystem::GetInstance();

    auto texture_name = std::string_view( material->texture->name );
    auto mat_desc     = m_ext_sys.GetMatDesc( texture_name );
    if ( !mat_desc )
        return 1;

    ext->mSpecTex = m_ext_sys.ReadTexture(
        mat_desc->mTextureDictName,
        std::string_view( mat_desc->mSpecularTextureName.data() ) );
    return 1;
}

MaterialData ConvertMaterialData( RpMaterial *material )
{
    int32_t tex_id      = 0xBADF00D;
    int32_t spec_tex_id = 0xBADF00D;
    if ( material == nullptr )
        return MaterialData{ tex_id, RwRGBA{ 255, 0, 0, 255 }, spec_tex_id, 0 };

    auto m_b = GetBackendMaterialExt( material );

    if ( material->texture && material->texture->raster )
    {
        auto raster = GetBackendRasterExt( material->texture->raster );
        tex_id      = raster->mImageId;
    }
    auto spec_tex = m_b->mSpecTex;
    if ( spec_tex && spec_tex->raster )
    {
        auto raster = GetBackendRasterExt( spec_tex->raster );
        spec_tex_id = raster->mImageId;
    }
    return MaterialData{ tex_id, material->color, spec_tex_id,
                         material->surfaceProps.specular };
}

} // namespace rh::rw::engine