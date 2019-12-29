#pragma once
#include <common.h>

int32_t RsCameraBeginUpdate( RwCamera *camera );
int32_t RsCameraShowRaster( RwCamera *camera );
void DoFade();
namespace rh {
RwCamera *RwCameraBeginUpdate( RwCamera *camera );                             // 0x7EE190
RwCamera *RwCameraEndUpdate( RwCamera *camera );                               // 0x7EE180
RwCamera *RwCameraClear( RwCamera *camera, RwRGBA *color, RwInt32 clearMode ); // 0x7EE340
} // namespace rh
