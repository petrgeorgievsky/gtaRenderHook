//
// Created by peter on 23.08.2021.
//

#include "native_texture_write_cmd.h"
#include "native_texture_get_size_cmd.h"
#include "rasterlockcmd.h"
#include "rasterunlockcmd.h"

#include <common_headers.h>
#include <format>

#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_raster/rw_raster_macros_wrappers.h>

namespace rh::rw::engine
{
namespace
{
constexpr auto rwCHUNKHEADERSIZE = ( sizeof( uint32_t ) * 3 );
struct rwNativeTexture
{
    friend std::ostream &operator<<( std::ostream          &os,
                                     const rwNativeTexture &texture )
    {
        os << "NativeTexture data block:\n\tid: " << texture.id
           << "\n\tfilterAndAddress: " << texture.filterAndAddress
           << "\n\tname: " << texture.name << "\n\tmask: " << texture.mask;
        return os;
    }
    int32_t id;               /* RwPlatformID,(rwID_D3D9) defined in batype.h */
    int32_t filterAndAddress; /* Same as in babintex.c */
    char    name[rwTEXTUREBASENAMELENGTH]; /* Texture name */
    char    mask[rwTEXTUREBASENAMELENGTH]; /* Texture mask name */
};

struct rwD3D9NativeRaster
{
    friend std::ostream &operator<<( std::ostream             &os,
                                     const rwD3D9NativeRaster &raster )
    {
        os << "D3D9NativeRaster data block:\n\tformat: " << raster.format
           << " d3dFormat: " << raster.d3dFormat << " width: " << raster.width
           << " height: " << raster.height
           << " depth: " << (uint32_t)raster.depth
           << " numMipLevels: " << (uint32_t)raster.numMipLevels
           << " type: " << (uint32_t)raster.type
           << " flags: " << (uint32_t)raster.flags;
        return os;
    }
    uint32_t  format;       /* Raster format flags */
    D3DFORMAT d3dFormat;    /* D3D pixel format */
    uint16_t  width;        /* Raster width */
    uint16_t  height;       /* Raster height */
    uint8_t   depth;        /* Raster depth */
    uint8_t   numMipLevels; /* The number of mip levels to load */
    uint8_t   type;         /* The raster type */
    uint8_t flags; /* This raster has an alpha component, automipmapgen, etc */
};
struct rwD3D8NativeRaster
{
    friend std::ostream &operator<<( std::ostream             &os,
                                     const rwD3D8NativeRaster &raster )
    {
        os << "D3D8NativeRaster data block:\n\tformat: " << raster.format
           << " alpha: " << raster.alpha << " width: " << raster.width
           << " height: " << raster.height
           << " depth: " << (uint32_t)raster.depth
           << " numMipLevels: " << (uint32_t)raster.numMipLevels
           << " type: " << (uint32_t)raster.type
           << " dxtFormat: " << (uint32_t)raster.dxtFormat;
        return os;
    }
    uint32_t format;       /* Raster format flags */
    int32_t  alpha;        /* This raster has an alpha component */
    uint16_t width;        /* Raster width */
    uint16_t height;       /* Raster height */
    uint8_t  depth;        /* Raster depth */
    uint8_t  numMipLevels; /* The number of mip levels to load */
    uint8_t  type;         /* The raster type */
    uint8_t  dxtFormat;    /* 1-5 for DXT format 0 for normal */
};
} // namespace

RwNativeTextureWriteCmd::RwNativeTextureWriteCmd( RwStream  *stream,
                                                  RwTexture *texture )
    : Stream( stream ), Texture( texture )
{
}

constexpr auto rwLIBRARYCURRENTRWVERSION      = 3;
constexpr auto rwLIBRARYCURRENTMAJORREVISION  = 3;
constexpr auto rwLIBRARYCURRENTMINORREVISION  = 0;
constexpr auto rwLIBRARYCURRENTBINARYREVISION = 2;
constexpr auto rwLIBRARYCURRENTVERSION =
    ( ( rwLIBRARYCURRENTRWVERSION << 16 ) |
      ( rwLIBRARYCURRENTMAJORREVISION << 12 ) |
      ( rwLIBRARYCURRENTMINORREVISION << 8 ) |
      ( rwLIBRARYCURRENTBINARYREVISION ) );

constexpr auto HAS_ALPHA         = ( 1 << 0 );
constexpr auto IS_CUBE           = ( 1 << 1 );
constexpr auto USE_AUTOMIPMAPGEN = ( 1 << 2 );
constexpr auto IS_COMPRESSED     = ( 1 << 3 );

bool RwNativeTextureWriteCmd::Execute()
{
    auto &io = g_pIO_API;

    RwRaster          *raster;
    rwNativeTexture    nativeTexture{};
    rwD3D8NativeRaster nativeRaster{};
    uint32_t           bytesLeftToWrite;
    int32_t            i, face;

    /* Calc the amount of data to write, excluding the chunk header*/
    RwNativeTextureGetSizeCmd{ Texture }.Execute( bytesLeftToWrite );
    bytesLeftToWrite -= rwCHUNKHEADERSIZE;

    /* Struct header for _rwD3DNativeTexture structure */
    constexpr auto RWBUILDNUMBER = 0xFFFF;
    if ( !io.fpWriteChunkHeader( Stream, rwID_STRUCT, bytesLeftToWrite,
                                 rwLIBRARYCURRENTVERSION, RWBUILDNUMBER ) )
        return false;

    /*
     * Don't have to worry about endianness as this is platform specific
     */

    /* ID, filter & addressing modes */
    nativeTexture.id = rwID_PCD3D8;
    nativeTexture.filterAndAddress =
        ( ( (int32_t)rwTexture::GetFilterMode( Texture ) ) & 0xFF ) |
        ( ( ( (int32_t)rwTexture::GetAddressingU( Texture ) ) << 8 ) &
          0x0F00 ) |
        ( ( ( (int32_t)rwTexture::GetAddressingV( Texture ) ) << 12 ) &
          0xF000 );

    /* Texture name */
    memcpy( nativeTexture.name, Texture->name,
            sizeof( char ) * rwTEXTUREBASENAMELENGTH );

    /* Mask name */
    memcpy( nativeTexture.mask, Texture->mask,
            sizeof( char ) * rwTEXTUREBASENAMELENGTH );

    if ( !io.fpWrite( Stream, (void *)&nativeTexture,
                      sizeof( rwNativeTexture ) ) )
        return false;

    bytesLeftToWrite -= sizeof( rwNativeTexture );

    /*
     * Write the rasters
     */
    raster            = Texture->raster;
    auto &raster_data = BackendRasterPlugin::GetData( raster );

    nativeRaster.width  = (uint16_t)raster->width;
    nativeRaster.height = (uint16_t)raster->height;
    assert( raster->cFormat > 0 );
    nativeRaster.format       = ( raster->cFormat << 8 ) & rwRASTERFORMATMASK;
    nativeRaster.depth        = 32;
    nativeRaster.numMipLevels = (uint8_t)raster_data.MipCount;
    nativeRaster.type         = (uint8_t)raster->cType;

    if ( raster_data.HasAlpha )
        nativeRaster.alpha = 1;

    if ( raster_data.Compressed )
    {
        if ( raster_data.OriginalFormat == D3DFMT_DXT1 )
            nativeRaster.dxtFormat = 1;
        else if ( raster_data.OriginalFormat == D3DFMT_DXT2 )
            nativeRaster.dxtFormat = 2;
        else if ( raster_data.OriginalFormat == D3DFMT_DXT3 )
            nativeRaster.dxtFormat = 3;
        else if ( raster_data.OriginalFormat == D3DFMT_DXT4 )
            nativeRaster.dxtFormat = 4;
        else if ( raster_data.OriginalFormat == D3DFMT_DXT5 )
            nativeRaster.dxtFormat = 5;
    }

    // nativeRaster.d3dFormat =
    //     static_cast<D3DFORMAT>( raster_data.OriginalFormat );
    static_assert( 0x48 == sizeof( rwNativeTexture ) );
    static_assert( 0x10 == sizeof( rwD3D8NativeRaster ) );
    if ( !io.fpWrite( Stream, &nativeRaster, sizeof( rwD3D9NativeRaster ) ) )
        return ( false );

    bytesLeftToWrite -= sizeof( rwD3D9NativeRaster );

    /// PALETTE Formats will be removed to remove loading overhead
    if ( nativeRaster.format & rwRASTERFORMATPAL4 )
        assert( false );
    else if ( nativeRaster.format & rwRASTERFORMATPAL8 )
        assert( false );

    /*if ( rasExt->cube )
    {
        face = 6;
    }
    else
    {
        face = 1;
    }*/

    // rasExt->face = 0;

    uint32_t size;
    for ( i = 0; i < nativeRaster.numMipLevels; i++ )
    {
        void           *pixels = nullptr;
        RwRasterLockCmd lock_cmd( raster,
                                  rwRASTERLOCKREAD + ( ( (uint8_t)i ) << 8 ) );

        if ( !lock_cmd.Execute( pixels ) )
            return false;

        size = raster->height * raster->width * raster_data.BytesPerBlock /
               raster_data.BlockSize;

        if ( !io.fpWrite( Stream, (void *)&size, sizeof( uint32_t ) ) )
            return false;

        bytesLeftToWrite -= sizeof( uint32_t );

        if ( !io.fpWrite( Stream, (void *)pixels, size ) )
            return false;
        bytesLeftToWrite -= size;
        RwRasterUnlockCmd unlock_cmd( raster );
        unlock_cmd.Execute();
    }

    assert( 0 == bytesLeftToWrite );

    return true;
}

} // namespace rh::rw::engine