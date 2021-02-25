//
// Created by peter on 23.02.2021.
//
#include "rwd3d8_patches.h"

#include <injection_utils/InjectorHelpers.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
#include <vector>

/// Old renderware videomode struct, lacks format and refresh-rate
struct Rw35VideoMode
{
    int32_t  width;
    int32_t  height;
    int32_t  depth;
    uint32_t flags;
};

Rw35VideoMode *RwEngineGetVideoModeInfoFix( Rw35VideoMode *modeinfo,
                                            int32_t        modeIndex )
{

    RwVideoMode videoMode{};
    if ( !rh::rw::engine::SystemHandler( rwDEVICESYSTEMGETMODEINFO, &videoMode,
                                         nullptr, modeIndex ) )
        return nullptr;
    modeinfo->width  = videoMode.width;
    modeinfo->height = videoMode.height;
    modeinfo->depth  = videoMode.depth;
    modeinfo->flags  = videoMode.flags;
    return modeinfo;
}

void RwD3D8Patches::Patch()
{
    RedirectJump( 0x642B70,
                  reinterpret_cast<void *>( RwEngineGetVideoModeInfoFix ) );
}
