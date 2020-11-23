#include "rw_func_ptrs.h"

int32_t RsCameraBeginUpdate( RwCamera *camera )
{
    static INT_PTR f_ptr = 0x619450;
    return reinterpret_cast<decltype( &RsCameraBeginUpdate )>( f_ptr )(
        camera );
}

int32_t RsCameraShowRaster( RwCamera *camera )
{
    static INT_PTR f_ptr = 0x619440;
    return reinterpret_cast<decltype( &RsCameraShowRaster )>( f_ptr )( camera );
}

RwCamera *rh::RwCameraBeginUpdate( RwCamera *camera )
{
    static INT_PTR f_ptr = 0x7EE190;
    return reinterpret_cast<decltype( &rh::RwCameraBeginUpdate )>( f_ptr )(
        camera );
}

RwCamera *rh::RwCameraEndUpdate( RwCamera *camera )
{
    static INT_PTR f_ptr = 0x7EE180;
    return reinterpret_cast<decltype( &rh::RwCameraEndUpdate )>( f_ptr )(
        camera );
}

RwCamera *rh::RwCameraClear( RwCamera *camera, RwRGBA *color,
                             int32_t clearMode )
{
    static INT_PTR f_ptr = 0x7EE340;
    return reinterpret_cast<decltype( &rh::RwCameraClear )>( f_ptr )(
        camera, color, clearMode );
}

void DoFade()
{
    static INT_PTR f_ptr = 0x53E600;
    return reinterpret_cast<decltype( &DoFade )>( f_ptr )();
}
