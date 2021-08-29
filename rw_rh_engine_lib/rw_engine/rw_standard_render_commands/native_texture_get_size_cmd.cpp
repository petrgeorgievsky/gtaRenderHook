//
// Created by peter on 29.08.2021.
//

#include "native_texture_get_size_cmd.h"

namespace rh::rw::engine
{
RwNativeTextureGetSizeCmd::RwNativeTextureGetSizeCmd( RwTexture *texture )
    : Texture( texture )
{
}
bool RwNativeTextureGetSizeCmd::Execute( uint32_t &size )
{
    int32_t numMipLevels;

    /* Platform specific ID, filter & addressing modes texture and Mask names */
    uint32_t t_size = rwCHUNKHEADERSIZE + sizeof( rwNativeTexture );

    auto raster = Texture->raster;
    if ( !raster )
    {
        size = t_size;
        return true;
    }

    /* Native raster data structure */
    t_size += sizeof( rwD3D9NativeRaster );

    /* Size of the palette if palletized */
    uint32_t rasFormat = RwRasterGetFormatMacro( raster );
    if ( rasFormat & rwRASTERFORMATPAL4 )
    {
        assert( false );
    }
    else if ( rasFormat & rwRASTERFORMATPAL8 )
    {
        assert( false );
    }

    auto &raster_data = BackendRasterPlugin::GetData( raster );

    /* Size of pixel data for all mip levels */
    /*if ( raster_data.IsCube )
    {
        RwUInt32 cubeSize = 0;
        numMipLevels      = raster_data.MipCount;
        if ( rasExt->automipmapgen )
        {
            numMipLevels = 1;
        }
        else
        {
            numMipLevels = IDirect3DCubeTexture9_GetLevelCount( cubeTexture );
        }

        while ( numMipLevels-- )
        {
            D3DSURFACE_DESC surfaceDesc;
            D3DLOCKED_RECT  lockedRect;
            RwUInt32        mipmapsize;

            IDirect3DCubeTexture9_GetLevelDesc( cubeTexture, numMipLevels,
                                                &surfaceDesc );

            cubeTexture->LockRect( D3DCUBEMAP_FACE_POSITIVE_X, numMipLevels,
                                   &lockedRect, NULL,
                                   D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY );
            cubeTexture->UnlockRect( D3DCUBEMAP_FACE_POSITIVE_X, numMipLevels );

            if ( rasExt->compressed )
            {
                if ( surfaceDesc.Height < 4 )
                {
                    mipmapsize = lockedRect.Pitch;
                }
                else
                {
                    mipmapsize = ( surfaceDesc.Height / 4 ) * lockedRect.Pitch;
                }
            }
            else
            {
                mipmapsize = surfaceDesc.Height * lockedRect.Pitch;
            }

            cubeSize += sizeof( RwUInt32 ) + mipmapsize;
        }

        size += 6 * cubeSize;
    }*/
    // else
    {
        numMipLevels             = raster_data.MipCount;
        auto     bytes_per_block = raster_data.BytesPerBlock;
        auto     block_size      = raster_data.BlockSize;
        uint32_t mip_w           = raster->width;
        uint32_t mip_h           = raster->height;
        uint32_t stride;

        while ( numMipLevels-- )
        {
            stride = bytes_per_block * ( ( mip_w + 3 ) / block_size );

            t_size += sizeof( uint32_t ) + ( stride * mip_h );
            mip_h = mip_h << 1;
            mip_w = mip_w << 1;
        }
    }

    size = t_size;

    return true;
}
} // namespace rh::rw::engine