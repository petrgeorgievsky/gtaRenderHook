#include "camera_backend.h"
#include "../system_funcs/rw_device_system_globals.h"
#include "common_headers.h"
#include <Engine/Common/IDeviceState.h>
#include <cassert>

namespace rh::rw::engine
{

int32_t gBackendCameraExtOffset = 0;

void *BackendCameraCtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject )
{
    [[maybe_unused]] auto *camExt =
        GetBackendCameraExt( static_cast<RwCamera *>( object ) );

    /*
        for ( int i = 0; i < gFrameResourceCacheSize; i++ )
        {
            camExt->mFrameResourceCache[i].mCmdBuffer =
                DeviceGlobals::RenderHookDevice->CreateCommandBuffer();
            camExt->mFrameResourceCache[i].mImageAquire =
                DeviceGlobals::RenderHookDevice->CreateSyncPrimitive(
                    rh::engine::SyncPrimitiveType::GPU );
            camExt->mFrameResourceCache[i].mRenderExecute =
                DeviceGlobals::RenderHookDevice->CreateSyncPrimitive(
                    rh::engine::SyncPrimitiveType::GPU );
            camExt->mFrameResourceCache[i].mBufferIsRecorded = false;
        }
        for ( int i = 0; i < gFramebufferCacheSize; i++ )
            camExt->mFramebufferCache[i] = nullptr;
        camExt->mRenderPass           = nullptr;
        camExt->mSwapchain            = nullptr;
        camExt->mClearFlags           = 0;
        camExt->mCurrentFrameId       = 0;
        camExt->mCurrentFramebufferId = 0;
    */

    return ( object );
}

void *BackendCameraDtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject )
{
    [[maybe_unused]] auto *camExt =
        GetBackendCameraExt( static_cast<RwCamera *>( object ) );
    /*
        for ( int i = 0; i < gFrameResourceCacheSize; i++ )
        {
            delete camExt->mFrameResourceCache[i].mCmdBuffer;
            camExt->mFrameResourceCache[i].mCmdBuffer = nullptr;
            delete camExt->mFrameResourceCache[i].mImageAquire;
            camExt->mFrameResourceCache[i].mImageAquire = nullptr;
            delete camExt->mFrameResourceCache[i].mRenderExecute;
            camExt->mFrameResourceCache[i].mRenderExecute    = nullptr;
            camExt->mFrameResourceCache[i].mBufferIsRecorded = false;
        }*/
    /* Phew! */
    return ( object );
}

int32_t BackendCameraPluginAttach()
{
    gBackendCameraExtOffset = gRwDeviceGlobals.PluginFuncs.CameraRegisterPlugin(
        sizeof( BackendCameraExt ), rwID_CAMERA_BACKEND, BackendCameraCtor,
        BackendCameraDtor, nullptr );
    return gBackendCameraExtOffset > 0;
}

BackendCameraExt *GetBackendCameraExt( RwCamera *camera )
{
    assert( camera );
    auto *ext = reinterpret_cast<BackendCameraExt *>(
        reinterpret_cast<uint8_t *>( camera ) + gBackendCameraExtOffset );

    return ext;
}
} // namespace rh::rw::engine
