#include "rw_texture.h"
#include "../rw_macro_constexpr.h"
#include <Engine/Common/types/comparison_func.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/sampler.h>
#include <Engine/Common/types/sampler_addressing.h>
#include <Engine/Common/types/sampler_filter.h>
#include <Engine/Common/types/shader_stage.h>
#include <array>
#include <common_headers.h>
#include <rw_engine/rw_raster/rw_raster.h>
#include <rw_engine/rw_rh_convert_funcs.h>
#include <rw_engine/rw_stream/rw_stream.h>

RwTexture *rh::rw::engine::RwTextureCreate( RwRaster *raster )
{
    auto *texture = hAlloc<RwTexture>( "Texture" );

    if ( texture == nullptr )
        return nullptr;

    texture->dict = nullptr;

    texture->name[0] = '\0';
    texture->mask[0] = '\0';

    texture->raster = raster; /* Set the raster */

    texture->refCount = 1; /* One reference so far */

    texture->filterAddressing = 0;

    /* Default addressing */
    rwTexture::SetAddressing( texture, rwTEXTUREADDRESSWRAP );

    /* Default sampling */
    rwTexture::SetFilterMode( texture, rwFILTERNEAREST );

    return texture;
}

int32_t TextureAnnihilate( RwTexture *texture )
{
    /* Temporarily bump up reference count to avoid assertion failures */
    texture->refCount++;

    if ( texture->dict )
    {
        rh::rw::engine::rwLinkList::RemoveLLLink( &texture->lInDictionary );
    }

    if ( texture->raster )
    {
        /* We still have the raster to destroy */
        rh::rw::engine::RwRasterDestroy( texture->raster );
        texture->raster = nullptr;
    }

    /* Reinstate reference count */
    --texture->refCount;

    return TRUE;
}

int32_t rh::rw::engine::RwTextureDestroy( RwTexture *texture )
{
    int32_t result = TRUE;

    --texture->refCount;

    if ( texture->refCount <= 0 )
        result = TextureAnnihilate( texture );

    /* All done */
    return result;
}
enum RwTextureStreamFlags
{
    rwNATEXTURESTREAMFLAG           = 0x00,
    rwTEXTURESTREAMFLAGSUSERMIPMAPS = 0x01
};
RwTexture *rh::rw::engine::RwTextureStreamRead( void *stream )
{
    uint32_t size, version;

    if ( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
    {
        return nullptr;
    }

    RwTexture *                                   texture;
    std::array<char, rwTEXTUREBASENAMELENGTH * 4> textureName;
    std::array<char, rwTEXTUREBASENAMELENGTH * 4> textureMask;
    RwTextureFilterMode                           filtering;
    RwTextureAddressMode                          addressingU;
    RwTextureAddressMode                          addressingV;
    struct _rwStreamTexture
    {
        uint32_t filterAndAddress;
    } texFiltAddr{};
    // RwBool mipmapState;
    // RwBool autoMipmapState;
    RwTextureStreamFlags flags;

    /* Read the filtering mode */
    memset( &texFiltAddr, 0, sizeof( texFiltAddr ) );
    if ( RwStreamRead( stream, &texFiltAddr, size ) != size )
    {
        return nullptr;
    }

    /* Extract filtering */
    filtering = static_cast<RwTextureFilterMode>(
        texFiltAddr.filterAndAddress & rwTexture::rwTEXTUREFILTERMODEMASK );

    /* Extract addressing */
    addressingU = static_cast<RwTextureAddressMode>(
        ( texFiltAddr.filterAndAddress >> 8 ) & 0x0F );

    addressingV = static_cast<RwTextureAddressMode>(
        ( texFiltAddr.filterAndAddress >> 12 ) & 0x0F );

    /* Make sure addressingV is valid so files old than 3.04 still work */
    if ( addressingV == rwTEXTUREADDRESSNATEXTUREADDRESS )
    {
        addressingV = addressingU;
        texFiltAddr.filterAndAddress |=
            ( ( static_cast<uint32_t>( addressingV ) & 0xF ) << 12 );
    }

    /* Extract user mipmap flags */
    flags = static_cast<RwTextureStreamFlags>(
        ( texFiltAddr.filterAndAddress >> 16 ) & 0xFF );

    // mipmapState = RwTextureGetMipmapping();
    // autoMipmapState = RwTextureGetAutoMipmapping();

    /* Use it */
    if ( ( filtering == rwFILTERMIPNEAREST ) ||
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
        // RwTextureSetMipmapping( FALSE );
        // RwTextureSetAutoMipmapping( FALSE );
    }

    /* Search for a string or a unicode string */
    if ( !_rwStringStreamFindAndRead( textureName.data(), stream ) )
    {
        // RwTextureSetMipmapping( mipmapState );
        // RwTextureSetAutoMipmapping( autoMipmapState );

        return nullptr;
    }

    /* Search for a string or a unicode string */
    if ( !_rwStringStreamFindAndRead( textureMask.data(), stream ) )
    {
        // RwTextureSetMipmapping( mipmapState );
        // RwTextureSetAutoMipmapping( autoMipmapState );
        return nullptr;
    }

    /* Get the textures */
    // if ( !( texture = nullptr /*RwTextureRead( textureName, textureMask )*/ )
    // )
    //{
    RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr );
    return nullptr;
    //}
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
    // RwTextureSetMipmapping( mipmapState );
    // RwTextureSetAutoMipmapping( autoMipmapState );

    // RWASSERT( 0 < texture->refCount );

    if ( texture->refCount == 1 )
    {
        /* By testing the reference count here,
         * we can tell if we just loaded it!!! */

        /* Set the filtering and addressing */
        texture->filterAddressing = texFiltAddr.filterAndAddress &
                                    ( rwTexture::rwTEXTUREFILTERMODEMASK |
                                      rwTexture::rwTEXTUREADDRESSINGMASK );

        /* Read the extension chunks */
        /*if( !_rwPluginRegistryReadDataChunks( &textureTKList, stream, texture
) )
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
    if ( !RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr ) )
        return nullptr;
    return ( texture );
}
/*
int32_t rh::rw::engine::RwD3D11SetTexture( void *context,
                                         RwTexture *texture,
                                         uint32_t reg_id,
                                         uint32_t stage )
{
    using namespace rh::engine;
    if ( reg_id > D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT )
        return false;
    auto context_impl = static_cast<IRenderingContext *>(
        context != nullptr ? context : g_pRHRenderer->GetCurrentContext() );

    ImageBindType bindType;
    ShaderStage shaderStage;
    switch ( static_cast<RwRenderShaderStage>( stage ) ) {
    case RwRenderShaderStage::VertexShader:
        bindType = ImageBindType::VSResource;
        shaderStage = ShaderStage::Vertex;
        break;
    case RwRenderShaderStage::GeometryShader:
        bindType = ImageBindType::GSResource;
        shaderStage = ShaderStage::Geometry;
        break;
    case RwRenderShaderStage::HullShader:
        bindType = ImageBindType::HSResource;
        shaderStage = ShaderStage::Hull;
        break;
    case RwRenderShaderStage::DomainShader:
        bindType = ImageBindType::DSResource;
        shaderStage = ShaderStage::Domain;
        break;
    case RwRenderShaderStage::PixelShader:
        bindType = ImageBindType::PSResource;
        shaderStage = ShaderStage::Pixel;
        break;
    case RwRenderShaderStage::ComputeShader:
        bindType = ImageBindType::CSResource;
        shaderStage = ShaderStage::Compute;
        break;
    }

    if ( !texture )
        return context_impl->BindImageBuffers( bindType, {{reg_id, nullptr}} );

    RwTextureAddressMode addressingU = RwTextureGetAddressingU( texture );
    RwTextureAddressMode addressingV = RwTextureGetAddressingV( texture );
    RwTextureAddressMode addressingW = RwTextureGetAddressing( texture );
    //RwTextureFilterMode filtering = RwTextureGetFilterMode( texture );

    if ( !texture->raster )
        return false;
    auto raster = texture->raster;

    auto *internalTexture = GetInternalRaster( raster );

    if ( !internalTexture )
        return false;
    Sampler sampler{};
    sampler.adressU = RwTextureAddressModeToRHSamplerAddressing( addressingU );
    sampler.adressV = RwTextureAddressModeToRHSamplerAddressing( addressingV );
    sampler.adressW = RwTextureAddressModeToRHSamplerAddressing( addressingW );
    sampler.filtering = SamplerFilter::Anisotropic;
    sampler.comparison = ComparisonFunc::Never;
    sampler.borderColor = {255, 255, 255, 255};
    context_impl->BindSamplers( {{reg_id, &sampler}}, shaderStage );
    return context_impl->BindImageBuffers( bindType, {{reg_id, internalTexture}}
);
}
*/