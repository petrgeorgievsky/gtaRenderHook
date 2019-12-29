#include "rp_material_funcs.h"

RpMaterial* RH_RWAPI::_RpMaterialCreate( void )
{
    RpMaterial* material;
    RwRGBA              color;

    material = (RpMaterial*)malloc( sizeof( RpMaterial ) );
    if( !material )
        return nullptr;

    material->refCount = 1;

    /* White is an appropriate material color (especially for textured materials) */
    color.red = 0xff;
    color.green = 0xff;
    color.blue = 0xff;
    color.alpha = 0xff;

    RpMaterialSetColor( material, &color );
    material->texture = (RwTexture*)NULL;  /* Non textured */

    /* use the default material pipeline */
    material->pipeline = (RxPipeline*)NULL;
    material->surfaceProps = { (RwReal)( 1.0 ), (RwReal)( 1.0 ), (RwReal)( 1.0 ) };
    //RpMaterialSetSurfaceProperties( material, &defaultSurfaceProperties );

    /* Initialize memory allocated to toolkits */
    //rwPluginRegistryInitObject( &materialTKList, material );

    //RWASSERT( 0 < material->refCount );

    return ( material );
}
RwBool RH_RWAPI::_RpMaterialDestroy( RpMaterial* material )
{
    if( 1 == material->refCount )
    {
        /* De-initialize memory allocated to toolkits */
        //rwPluginRegistryDeInitObject( &materialTKList, material );

        /* Decreases the reference count on the texture too */
        //RpMaterialSetTexture( material, (RwTexture *)NULL );

        free( material );//RwFreeListFree( RWMATERIALGLOBAL( matFreeList ), material );
    }
    else
    {
        /* RWCRTCHECKMEMORY(); */
        --material->refCount;
        /* RWCRTCHECKMEMORY(); */
    }

    return ( TRUE );
}
