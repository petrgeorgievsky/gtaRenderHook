#include "rp_material_read_funcs.h"
#include "rp_material_funcs.h"
RwTexture*
_RwTextureStreamRead( void* stream )
{
    RwUInt32    size, version;

    if( !RH_RWAPI::RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
    {
        return nullptr;
    }

    RwTexture* texture;
    RwChar                  textureName[rwTEXTUREBASENAMELENGTH * 4];
    RwChar                  textureMask[rwTEXTUREBASENAMELENGTH * 4];
    RwTextureFilterMode     filtering;
    RwTextureAddressMode    addressingU;
    RwTextureAddressMode    addressingV;
    struct _rwStreamTexture
    {
        RwUInt32            filterAndAddress;
    } texFiltAddr;
    RwBool                  mipmapState;
    RwBool                  autoMipmapState;
    RwTextureStreamFlags    flags;

    /* Read the filtering mode */
    memset( &texFiltAddr, 0, sizeof( texFiltAddr ) );
    if( RH_RWAPI::RwStreamRead( stream, &texFiltAddr, size ) != size )
    {
        return nullptr;
    }

    /* Extract filtering */
    filtering = (RwTextureFilterMode)
        ( texFiltAddr.filterAndAddress & rwTEXTUREFILTERMODEMASK );

    /* Extract addressing */
    addressingU = (RwTextureAddressMode)
        ( ( texFiltAddr.filterAndAddress >> 8 ) & 0x0F );

    addressingV = (RwTextureAddressMode)
        ( ( texFiltAddr.filterAndAddress >> 12 ) & 0x0F );

    /* Make sure addressingV is valid so files old than 3.04 still work */
    if( addressingV == rwTEXTUREADDRESSNATEXTUREADDRESS )
    {
        addressingV = addressingU;
        texFiltAddr.filterAndAddress |= ( ( addressingV & 0xF ) << 12 );
    }

    /* Extract user mipmap flags */
    flags = (RwTextureStreamFlags)( ( texFiltAddr.filterAndAddress >> 16 ) & 0xFF );

    //mipmapState = RwTextureGetMipmapping();
    //autoMipmapState = RwTextureGetAutoMipmapping();

    /* Use it */
    if( ( filtering == rwFILTERMIPNEAREST ) ||
        ( filtering == rwFILTERMIPLINEAR ) ||
        ( filtering == rwFILTERLINEARMIPNEAREST ) ||
        ( filtering == rwFILTERLINEARMIPLINEAR ) )
    {
        /* Lets mip map it */
        /*RwTextureSetMipmapping( TRUE );
        if( flags & rwTEXTURESTREAMFLAGSUSERMIPMAPS )
        {
            RwTextureSetAutoMipmapping( FALSE );
        }
        else
        {
            RwTextureSetAutoMipmapping( TRUE );
        }*/
    }
    else
    {
        /* Lets not */
        //RwTextureSetMipmapping( FALSE );
        //RwTextureSetAutoMipmapping( FALSE );
    }

    /* Search for a string or a unicode string */
    if( !RH_RWAPI::_rwStringStreamFindAndRead( textureName, stream ) )
    {
        //RwTextureSetMipmapping( mipmapState );
        //RwTextureSetAutoMipmapping( autoMipmapState );

        return nullptr;
    }

    /* Search for a string or a unicode string */
    if( !RH_RWAPI::_rwStringStreamFindAndRead( textureMask, stream ) )
    {
        //RwTextureSetMipmapping( mipmapState );
        //RwTextureSetAutoMipmapping( autoMipmapState );
        return nullptr;
    }

    /* Get the textures */
    if( !( texture = _RwTextureRead( textureName, textureMask ) ) )
    {
        RH_RWAPI::RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr );
        return nullptr;
    }
    //{
    //    /* Skip any extension chunks */
    //    _rwPluginRegistrySkipDataChunks( &textureTKList, stream );
    //
    //    RwTextureSetMipmapping( mipmapState );
    //    RwTextureSetAutoMipmapping( autoMipmapState );
    //
    //    RWRETURN( (RwTexture*)NULL );
    //}

    /* clean up */
    //RwTextureSetMipmapping( mipmapState );
    //RwTextureSetAutoMipmapping( autoMipmapState );

    //RWASSERT( 0 < texture->refCount );

    if( texture->refCount == 1 )
    {
        /* By testing the reference count here,
            * we can tell if we just loaded it!!! */

            /* Set the filtering and addressing */
        texture->filterAddressing = texFiltAddr.filterAndAddress &
            ( rwTEXTUREFILTERMODEMASK | rwTEXTUREADDRESSINGMASK );

        /* Read the extension chunks */
        /*if( !_rwPluginRegistryReadDataChunks( &textureTKList, stream, texture ) )
        {
            RWRETURN( (RwTexture*)NULL );
        }*/
    }
    else
    {
        /*if( !_rwPluginRegistrySkipDataChunks( &textureTKList, stream ) )
        {
            //RWRETURN( (RwTexture*)NULL );
        }*/
    }
    if( !RH_RWAPI::RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr ) )
        return nullptr;
    return ( texture );
}

RpMaterial* RH_RWAPI::_RpMaterialStreamRead( void* stream )
{
    RwUInt32            size;
    RwUInt32            version;

    if( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
    {
        return nullptr;
    }

    RpMaterial* material = nullptr;
    _rpMaterial         mat;

    memset( &mat, 0, sizeof( mat ) );
    if( RwStreamRead( stream, &mat, size ) != size )
    {
        return nullptr;
    }

    /* Create the material */
    material = _RpMaterialCreate();
    if( !material )
    {
        return nullptr;
    }

    /* We want the color */
    RpMaterialSetColor( material, &mat.color );


    /* set the surfaceProps */
    /*material->surfaceProps = mat.surfaceProps; */
    RpMaterialSetSurfaceProperties( material, &mat.surfaceProps );

    /* Check if it has a texture */
    material->texture = nullptr;

    if( mat.textured )
    {
        /* Read in the texture */
        if( !RwStreamFindChunk( stream, (RwUInt32)rwID_TEXTURE,
            (RwUInt32 *)NULL, &version ) )
        {
            _RpMaterialDestroy( material );
            return nullptr;
        }
        /* If we don't get the material, the polygons will just be the
        * color of the underlying material - usually white
        */
        material->texture = _RwTextureStreamRead( stream );
    }
    if( !RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr ) )
        return nullptr;
    return ( material );
}