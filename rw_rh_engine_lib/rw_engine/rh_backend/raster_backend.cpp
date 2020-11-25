#include "raster_backend.h"
#include "../system_funcs/rw_device_system_globals.h"
#include "Engine/Common/IImageBuffer.h"
#include "Engine/Common/IImageView.h"
#include "common_headers.h"
#include "material_backend.h"

#include <cassert>

namespace rh::rw::engine
{

rh::engine::ResourcePool<RasterData> *RasterGlobals::SceneRasterPool = nullptr;

void RasterGlobals::Init()
{
    if ( SceneRasterPool != nullptr )
        return;

    SceneRasterPool = new rh::engine::ResourcePool<RasterData>(
        11000, []( RasterData &obj, uint64_t ) {
            delete obj.mImageView;
            delete obj.mImageBuffer;
        } );
}

int32_t gBackendRasterExtOffset = 0;

void *BackendRasterCtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject )
{
    auto *rasExt = GetBackendRasterExt( static_cast<RwRaster *>( object ) );

    /* These are used to detect when we are using a camera for the first time */
    // rasExt->mWindow              = nullptr;
    // rasExt->mCurrentBackBufferId = 0;
    rasExt->mImageId = 0xBADF00D;
    return ( object );
}

void *BackendRasterDtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject )
{
    auto *rasExt = GetBackendRasterExt( static_cast<RwRaster *>( object ) );
    auto  img_id = rasExt->mImageId;
    if ( rasExt->mImageId == 0xBADF00D )
        return ( object );

    DeviceGlobals::SharedMemoryTaskQueue->ExecuteTask(
        SharedMemoryTaskType::DESTROY_RASTER,
        [&img_id]( MemoryWriter &&memory_writer ) {
            // serialize
            memory_writer.Write( &img_id );
        },
        []( MemoryReader && ) {} );

    rasExt->mImageId = 0xBADF00D;

    /* Phew! */
    return ( object );
}

int32_t BackendRasterPluginAttach()
{
    gBackendRasterExtOffset = DeviceGlobals::PluginFuncs.RasterRegisterPlugin(
        sizeof( BackendRasterExt ), rwID_RASTER_BACKEND, BackendRasterCtor,
        BackendRasterDtor, nullptr );
    return gBackendRasterExtOffset > 0;
}

BackendRasterExt *GetBackendRasterExt( RwRaster *raster )
{
    assert( raster );
    auto *internalRaster = reinterpret_cast<BackendRasterExt *>(
        ( reinterpret_cast<uint8_t *>( raster ) ) + gBackendRasterExtOffset );

    return internalRaster;
}

} // namespace rh::rw::engine
