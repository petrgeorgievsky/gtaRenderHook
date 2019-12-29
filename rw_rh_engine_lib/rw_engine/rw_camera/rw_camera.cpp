#include "rw_camera.h"
#include "../global_definitions.h"
#include "../rw_macro_constexpr.h"
#include <common_headers.h>
namespace rh::rw::engine {

RwCamera *RwCameraCreate()
{
    auto *camera = static_cast<RwCamera *>( malloc( sizeof( RwCamera ) ) );

    if ( camera == nullptr )
        return nullptr;
    rwObject::HasFrameInitialize( camera, rwCAMERA, 0, nullptr );

    /* Set up the defaults for the camera */
    camera->viewWindow.x = camera->viewWindow.y = 1.0f;
    camera->recipViewWindow.x = camera->recipViewWindow.y = 1.0f;
    camera->viewOffset.x = camera->viewOffset.y = 0.0f;

    /* Clipping planes */
    camera->nearPlane = 0.05f;
    camera->farPlane = 8000.0f;
    camera->fogPlane = 0.1f;

    /* Render destination rasters */
    camera->frameBuffer = nullptr;
    camera->zBuffer = nullptr;

    /* Set up projection type */
    camera->projectionType = rwPERSPECTIVE;

    return camera;
}

void RwCameraDestroy( RwCamera *camera )
{
    if ( camera )
        free( camera );
}

RwCamera *RwCameraClear( RwCamera *camera, RwRGBA *color, int32_t clearMode )
{
    RwStandardFunc CameraClearFunc = g_pRwEngineInstance->stdFunc[rwSTANDARDCAMERACLEAR];

    if ( CameraClearFunc( camera, color, clearMode ) ) {
        return camera;
    }

    /* Device error */
    return nullptr;
}

} // namespace rw_rh_engine
