#include "rw_image_funcs.h"
#include <common_headers.h>

uint32_t rh::rw::engine::InternalImageFindFormat( RwImage *image )
{
    RwInt32 depth;
    RwUInt32 format;

    depth = image->depth;

    if ( ( 4 == depth ) || ( 8 == depth ) ) {
        const RwInt32 width = image->width;
        const RwInt32 height = image->height;
        const RwUInt8 *cpIn = image->cpPixels;
        const RwRGBA *rpPal = image->palette;
        RwInt32 y;
        RwBool paletteHasAlpha;

        /*
     * 4 & 8bit palettized images always have 32bit palettes, X888 or 8888
     */

        /* First: check palette for transparent colors */
        paletteHasAlpha = FALSE;
        if ( 4 == depth ) {
            for ( y = 0; y < 16; y++ ) {
                if ( 0xFF != rpPal[y].alpha ) {
                    paletteHasAlpha = TRUE;
                    break;
                }
            }
        } else {
            for ( y = 0; y < 256; y++ ) {
                if ( 0xFF != rpPal[y].alpha ) {
                    paletteHasAlpha = TRUE;
                    break;
                }
            }
        }

        if ( paletteHasAlpha ) {
            for ( y = 0; y < height; y++ ) {
                const RwUInt8 *cpInCur = cpIn;
                RwInt32 x;

                for ( x = 0; x < width; x++ ) {
                    /* Is there any alpha */
                    if ( 0xFF != rpPal[*cpInCur].alpha ) {
                        if ( 4 == depth )
                            format = rwRASTERFORMAT8888 | rwRASTERFORMATPAL4;
                        else
                            format = rwRASTERFORMAT8888 | rwRASTERFORMATPAL8;

                        return format;
                    }

                    /* Next pixel */
                    cpInCur++;
                }

                cpIn += image->stride;
            }
        }

        if ( 4 == depth )
            format = rwRASTERFORMAT888 | rwRASTERFORMATPAL4;
        else
            format = rwRASTERFORMAT888 | rwRASTERFORMATPAL8;
    } else {
        const RwInt32 width = image->width;
        const RwInt32 height = image->height;
        const RwUInt8 *cpIn = image->cpPixels;
        RwInt32 y;
        RwUInt32 alphaBits = 0;

        for ( y = 0; y < height; y++ ) {
            const auto *rpInCur = reinterpret_cast<const RwRGBA *>( cpIn );
            RwInt32 x;

            for ( x = 0; x < width; x++ ) {
                if ( rpInCur->alpha < 0xff ) {
                    /* lower 4 bits of the alpha channel are discarded in 4444 */
                    if ( rpInCur->alpha > 0xf ) {
                        alphaBits = 8;
                        break;
                    }
                    alphaBits = 1;
                }

                /* Next pixel */
                rpInCur++;
            }

            if ( alphaBits >= 8 ) {
                break;
            }

            cpIn += image->stride;
        }

        if ( alphaBits >= 8 )
            format = rwRASTERFORMAT8888;
        else if ( alphaBits )
            format = rwRASTERFORMAT8888;
        else
            format = rwRASTERFORMAT888;
    }

    return format;
}
