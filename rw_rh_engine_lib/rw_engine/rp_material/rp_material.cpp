#include "rp_material.h"
#include "../rw_macro_constexpr.h"
#include "../rw_stream/rw_stream.h"
#include "../rw_texture/rw_texture.h"
#include <common_headers.h>
#include <rw_engine/rh_backend/material_backend.h>
namespace rh::rw::engine
{
struct RpMaterialChunkInfo;

struct RpMaterialChunkInfo
{
    int32_t flags;                    /* Material flags - unused currently -
                                       * for future expansion */
    RwRGBA              color;        /* Color of material. */
    int32_t             unused;       /* Not used */
    int32_t             textured;     /* Are we textured? */
    RwSurfaceProperties surfaceProps; /* Surface properties */
};

RpMaterial *RpMaterialStreamRead( void *stream )
{
    uint32_t size;
    uint32_t version;

    if ( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
    {
        return nullptr;
    }

    RpMaterial *        material = nullptr;
    RpMaterialChunkInfo mat{};
    if ( RwStreamRead( stream, &mat, size ) != size )
    {
        return nullptr;
    }

    /* Create the material */
    material = RpMaterialCreate();
    if ( !material )
    {
        return nullptr;
    }

    /* We want the color */
    rpMaterial::SetColor( material, mat.color );

    /* set the surfaceProps */
    rpMaterial::SetSurfaceProperties( material, mat.surfaceProps );

    /* Check if it has a texture */
    material->texture = nullptr;

    if ( mat.textured )
    {
        /* Read in the texture */
        if ( !RwStreamFindChunk( stream, rwID_TEXTURE, nullptr, &version ) )
        {
            rh::rw::engine::RpMaterialDestroy( material );
            return nullptr;
        }
        /* If we don't get the material, the polygons will just be the
         * color of the underlying material - usually white
         */
        material->texture = RwTextureStreamRead( stream );
    }
    if ( !RwStreamFindChunk( stream, rwID_EXTENSION, nullptr, nullptr ) )
        return nullptr;
    return ( material );
}

RpMaterial *RpMaterialCreate()
{
    RpMaterial *material;
    RwRGBA      color{};

    material = hAlloc<RpMaterial>(
        "Material", sizeof( RpMaterial ) + sizeof( BackendMaterialExt ) );
    if ( !material )
        return nullptr;

    material->refCount = 1;

    /*
     *  White is an appropriate material color (especially for textured
     * materials)
     */
    color.red   = 0xff;
    color.green = 0xff;
    color.blue  = 0xff;
    color.alpha = 0xff;

    rpMaterial::SetColor( material, color );
    material->texture = nullptr; /* Non textured */

    /* use the default material pipeline */
    material->pipeline     = nullptr;
    material->surfaceProps = { 1.0F, 1.0F, 1.0F };
    auto &mat_ext          = BackendMaterialPlugin::GetData( material );
    mat_ext.mSpecTex       = nullptr;
    // RpMaterialSetSurfaceProperties( material, &defaultSurfaceProperties );

    /* Initialize memory allocated to toolkits */
    // rwPluginRegistryInitObject( &materialTKList, material );

    // RWASSERT( 0 < material->refCount );

    return ( material );
}

int32_t RpMaterialDestroy( RpMaterial *material )
{
    if ( 1 == material->refCount )
    {
        /* De-initialize memory allocated to toolkits */
        // rwPluginRegistryDeInitObject( &materialTKList, material );

        /* Decreases the reference count on the texture too */
        // RpMaterialSetTexture( material, (RwTexture *)NULL );

        hFree( material ); // RwFreeListFree( RWMATERIALGLOBAL( matFreeList ),
                           // material
                           // );
    }
    else
    {
        /* RWCRTCHECKMEMORY(); */
        --material->refCount;
        /* RWCRTCHECKMEMORY(); */
    }

    return ( TRUE );
}

} // namespace rh::rw::engine
