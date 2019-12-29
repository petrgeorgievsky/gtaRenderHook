#include "rp_clump_funcs.h"

namespace RH_RWAPI {

RpClump *_RpClumpCreate( void ) noexcept
{
    RpClump *clump;

    clump = (RpClump *) malloc( sizeof( RpClump ) );

    if ( !clump ) {
        return ( clump );
    }

    rwObjectInitialize( clump, rpCLUMP, 0 );
    //_RpClumpSetFrame( clump, NULL );

    /* Contains nothing */
    rwLinkListInitialize( &clump->atomicList );
    rwLinkListInitialize( &clump->lightList );
    rwLinkListInitialize( &clump->cameraList );

    /* Its not in the world */
    rwLLLinkInitialize( &clump->inWorldLink );

    /* Set the callback */
    // RpClumpSetCallBack( clump, (RpClumpCallBack)NULL );

    /* Initialize memory allocated to toolkits */
    // rwPluginRegistryInitObject( &clumpTKList, clump );

    /* All Done */
    return ( clump );
}

RwBool _RpClumpDestroy( RpClump *clump )
{
    // RwFrame* frame = nullptr;

    /* De-init the clump plugin registered memory */
    // rwPluginRegistryDeInitObject( &clumpTKList, clump );

    // RpClumpForAllAtomics( clump, DestroyClumpAtomic, NULL );
    // RpClumpForAllLights( clump, DestroyClumpLight, NULL );
    // RpClumpForAllCameras( clump, DestroyClumpCamera, NULL );

    /* Destroy the frame hierarchy if one exists */
    // frame = RpClumpGetFrame( clump );
    /*if( frame )
  {
      RwFrameDestroyHierarchy( frame );
  }*/

    /* Destroy the clump */
    free( clump );

    return ( TRUE );
}

RpClump *_RpClumpAddAtomic( RpClump *clump, RpAtomic *atomic ) noexcept
{
    /* It is assumed the atomic is NOT in the world although the
   * clump might be
   */

    rwLinkListAddLLLink( &clump->atomicList, &atomic->inClumpLink );
    atomic->clump = clump;

    return ( clump );
}

} // namespace RH_RWAPI
