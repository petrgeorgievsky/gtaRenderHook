//
// Created by peter on 18.01.2021.
//

#include "rwd3d8_patches.h"
#include "../call_redirection_util.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <rw_game_hooks.h>

/// Old renderware videomode struct, lacks format and refresh-rate
struct Rw35VideoMode
{
    int32_t  width;
    int32_t  height;
    int32_t  depth;
    uint32_t flags;
};

int32_t rwD3D8FindCorrectRasterFormat( RwRasterType type, uint32_t flags )
{
    uint32_t format = flags & rwRASTERFORMATMASK;
    switch ( type )
    {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            /* Check if we are requesting a default pixel format palette texture
             */
            if ( format & rwRASTERFORMATPAL8 )
            {
                format |= rwRASTERFORMAT8888;
            }
            if ( ( format & rwRASTERFORMATPAL8 ) == 0 )
            {
                format |= rwRASTERFORMAT8888;
            }
            else
            {
                format = rwRASTERFORMAT8888;
            }
        }
        else
        {
            /* No support for 4 bits palettes */
            if ( format & rwRASTERFORMATPAL4 )
            {
                /* Change it to a 8 bits palette */
                format &= static_cast<uint32_t>( ~rwRASTERFORMATPAL4 );

                format |= rwRASTERFORMATPAL8;
            }
            if ( format & rwRASTERFORMATPAL8 )
            {
                /* Change it to a 8 bits palette */
                format &= static_cast<uint32_t>( ~rwRASTERFORMATPAL8 );

                // format = rwRASTERFORMATPAL8;
            }
        }
        break;
    case rwRASTERTYPECAMERATEXTURE:
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format |= rwRASTERFORMAT888;
        }
        break;

    case rwRASTERTYPECAMERA:
        /* Always force default */
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format = rwRASTERFORMAT8888;
        }
        break;

    case rwRASTERTYPEZBUFFER:
        /* Always force default */
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format = rwRASTERFORMAT32;
        }
        break;
    default: break;
    }
    return static_cast<int32_t>( format );
}

char **_psGetVideoModeList()
{
    int32_t                    numModes;
    int32_t                    i;
    static std::vector<char *> videomode_list;
    if ( !videomode_list.empty() )
    {
        return videomode_list.data();
    }
    if ( !rh::rw::engine::SystemHandler( rwDEVICESYSTEMGETNUMMODES, &numModes,
                                         nullptr, 0 ) )
        return videomode_list.data();

    videomode_list.resize( numModes, nullptr );

    for ( i = 0; i < numModes; i++ )
    {
        RwVideoMode vm{};
        if ( !rh::rw::engine::SystemHandler( rwDEVICESYSTEMGETMODEINFO, &vm,
                                             nullptr, i ) )
        {
            videomode_list[i] = nullptr;
            continue;
        }
        videomode_list[i] = new char[4096];
        sprintf( videomode_list[i], "%d X %d X %d HZ", vm.width, vm.height,
                 vm.refRate );
    }

    return videomode_list.data();
}

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
    // GTA 3 stores some of the supported formats in binary file, and checks
    // whether it needs to convert it using rwD3D8 function, for now replace
    // it with our own impl
    RedirectJump( GetAddressByGame( 0x59A350, 0x59A610, 0x59B7E0 ),
                  reinterpret_cast<void *>( rwD3D8FindCorrectRasterFormat ) );

    RedirectJump( GetAddressByGame( 0x5A0F00, 0x5A11C0, 0x5A1840 ),
                  reinterpret_cast<void *>( RwEngineGetVideoModeInfoFix ) );
}
