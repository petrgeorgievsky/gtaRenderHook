#pragma once
#include <stdint.h>
struct RwCamera;
struct RwRGBA;
namespace rh::rw::engine {

RwCamera *RwCameraCreate();

void RwCameraDestroy(RwCamera *camera);

RwCamera *RwCameraClear(RwCamera *camera, RwRGBA *color, int32_t clearMode);
} // namespace rw_rh_engine
