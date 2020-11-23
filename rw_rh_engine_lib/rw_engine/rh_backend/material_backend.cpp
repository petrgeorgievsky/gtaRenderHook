//
// Created by peter on 08.05.2020.
//

#include "material_backend.h"
#include "raster_backend.h"
#include <rw_engine/system_funcs/rw_device_system_globals.h>
namespace rh::rw::engine
{
rh::engine::ResourcePool<MaterialData> *MaterialGlobals::SceneMaterialPool =
    nullptr;
int32_t gBackendMaterialExtOffset = 0;
int32_t engine::BackendMaterialPluginAttach()
{
    gBackendMaterialExtOffset =
        DeviceGlobals::PluginFuncs.MaterialRegisterPlugin(
            sizeof( BackendMaterialExt ), rwID_MATERIAL_BACKEND,
            BackendMaterialCtor, BackendMaterialDtor, nullptr );
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
    DeviceGlobals::SharedMemoryTaskQueue->ExecuteTask(
        SharedMemoryTaskType::MATERIAL_DELETE,
        [&ext]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &ext->mMaterialId );
        },
        []( MemoryReader &&memory_reader ) {
            // deserialize
        } );
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
    DeviceGlobals::SharedMemoryTaskQueue->ExecuteTask(
        SharedMemoryTaskType::MATERIAL_LOAD,
        [&initData]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &initData );
        },
        [&material_id]( MemoryReader &&memory_reader ) {
            // deserialize
            material_id = *memory_reader.Read<uint64_t>();
        } );
    return material_id;
}

} // namespace rh::rw::engine