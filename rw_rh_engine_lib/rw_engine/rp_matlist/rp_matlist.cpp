#include "rp_matlist.h"
#include "../rp_material/rp_material.h"
#include "../rw_macro_constexpr.h"
#include "../rw_stream/rw_stream.h"
#include <common_headers.h>

static uint32_t rwMATERIALLISTGRANULARITY = 20;

void rh::rw::engine::_rpMaterialListInitialize( RpMaterialList &matList )
{
    matList.space        = 0;
    matList.materials    = nullptr;
    matList.numMaterials = 0;
}

void rh::rw::engine::_rpMaterialListDeinitialize( RpMaterialList &matList )
{
    RpMaterial **materialArray;

    materialArray = matList.materials;
    if ( materialArray )
    {
        int32_t materialCount = matList.numMaterials;
        int32_t nI;

        for ( nI = 0; nI < materialCount; nI++ )
        {
            rh::rw::engine::RpMaterialDestroy( materialArray[nI] );
            materialArray[nI] = nullptr;
        }

        free( materialArray );
        materialArray     = nullptr;
        matList.materials = materialArray;
    }

    /* Reset the structure */
    matList.numMaterials = 0;
    matList.space        = 0;
}

bool rh::rw::engine::_rpMaterialListSetSize( RpMaterialList &matList,
                                             int32_t         size )
{
    if ( matList.space < size )
    {
        RpMaterial **materials;
        size_t memSize = sizeof( RpMaterial * ) * static_cast<size_t>( size );

        if ( matList.materials )
        {
            materials = static_cast<RpMaterial **>(
                realloc( matList.materials, memSize ) );
        }
        else
        {
            materials = static_cast<RpMaterial **>( malloc( memSize ) );
        }

        if ( !materials )
        {
            // RWERROR( ( E_RW_NOMEM, memSize ) );
            return false;
        }

        /* Shove in the new */
        matList.materials = materials;
        matList.space     = size;
    }

    return true;
}

int32_t rh::rw::engine::_rpMaterialListAppendMaterial( RpMaterialList &matList,
                                                       RpMaterial *material )
{
    RpMaterial **materials;
    uint32_t     count;
    size_t       memSize;

    /* See if there is a blank entry we can use */
    /* Add it to the list */
    if ( matList.space > matList.numMaterials )
    {
        materials = &( matList.materials )[matList.numMaterials];

        ( *materials ) = material;
        rpMaterial::AddRef( material );

        matList.numMaterials++;

        return ( matList.numMaterials - 1 );
    }

    /* Need to allocates some more space */
    count =
        ( static_cast<uint32_t>( matList.space ) + rwMATERIALLISTGRANULARITY );

    memSize = sizeof( RpMaterial * ) * count;

    if ( matList.materials )
    {
        materials =
            static_cast<RpMaterial **>( realloc( matList.materials, memSize ) );
    }
    else
    {
        materials = static_cast<RpMaterial **>( malloc(
            count * sizeof( RpMaterial * ) ) ); //_rpMaterialListAlloc( count );
    }

    if ( !materials )
    {
        return -1;
    }

    /* Shove in the new */
    matList.materials = materials;
    matList.space += rwMATERIALLISTGRANULARITY;

    materials[matList.numMaterials] = material;
    rpMaterial::AddRef( material );
    matList.numMaterials++;

    return ( matList.numMaterials - 1 );
}

bool rh::rw::engine::_rpMaterialListStreamRead( void *          stream,
                                                RpMaterialList &matList )
{
    int32_t  i, len;
    int32_t *matindex;
    uint32_t size, version;

    if ( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
    {
        return false;
    }
    int32_t status;

    status = ( NULL != RwStreamRead( stream, &len, sizeof( len ) ) );
    if ( !status )
        return false;

    matList.space        = 0;
    matList.materials    = nullptr;
    matList.numMaterials = 0;

    if ( len == 0 )
    {
        /* zero entry material list simply return the initialized
matList we've created */
        return true;
    }

    /* make the list as large as needed */
    if ( !_rpMaterialListSetSize( matList, len ) )
    {
        _rpMaterialListDeinitialize( matList );
        return false;
    }

    matindex = static_cast<int32_t *>(
        malloc( sizeof( int32_t ) * static_cast<uint32_t>( len ) ) );

    status = ( NULL != RwStreamRead( stream, matindex,
                                     sizeof( uint32_t ) *
                                         static_cast<uint32_t>( len ) ) );

    if ( !status )
    {
        free( matindex );
        _rpMaterialListDeinitialize( matList );
        return false;
    }

    for ( i = 0; i < len; i++ )
    {
        RpMaterial *material;

        /* new material */
        if ( matindex[i] < 0 )
        {
            if ( !RwStreamFindChunk( stream, rwID_MATERIAL, nullptr,
                                     &version ) )
            {
                free( matindex );
                _rpMaterialListDeinitialize( matList );
                return false;
            }
            material = RpMaterialStreamRead( stream );
            if ( !material )
            {
                free( matindex );
                _rpMaterialListDeinitialize( matList );
                return false;
            }
        }
        else
        {
            material = matList.materials[matindex[i]];
            //_rpMaterialListGetMaterial( matList, matindex[i] );
            rpMaterial::AddRef( material );
        }

        /* Add the material to the end of the list - this bumps up the
         * reference count
         */
        _rpMaterialListAppendMaterial( matList, material );

        /* We want to drop the reference count back down, so that only
         * the material list owns it (so it goes away when the material
         * list does.
         */
        rh::rw::engine::RpMaterialDestroy( material );
    }

    free( matindex );
    return true;
}
