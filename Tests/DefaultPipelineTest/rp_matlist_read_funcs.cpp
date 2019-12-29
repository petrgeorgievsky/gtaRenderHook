#include "rp_matlist_read_funcs.h"
#include "rp_matlist_funcs.h"
#include "rp_material_read_funcs.h"
#include "rp_material_funcs.h"

bool RH_RWAPI::_rpMaterialListStreamRead( void* stream, RpMaterialList& matList )
{
    RwInt32             i, len, * matindex;
    RwUInt32            size, version;

    if( !RwStreamFindChunk( stream, rwID_STRUCT, &size, &version ) )
    {
        return false;
    }
    RwBool status;

    status = ( NULL != RwStreamRead( stream, &len, sizeof( len ) ) );
    if( !status )
        return false;

    matList.space = 0;
    matList.materials = nullptr;
    matList.numMaterials = 0;

    if( len == 0 )
    {
        /* zero entry material list simply return the initialized
           matList we've created */
        return true;
    }

    /* make the list as large as needed */
    if( !_rpMaterialListSetSize( matList, len ) )
    {
        _rpMaterialListDeinitialize( matList );
        return false;
    }

    matindex = (RwInt32*)malloc( sizeof( RwInt32 ) * len );

    status = ( NULL !=
               RwStreamRead( stream, matindex, sizeof( RwInt32 ) * len ) );

    if( !status )
    {
        free( matindex );
        _rpMaterialListDeinitialize( matList );
        return false;
    }

    for( i = 0; i < len; i++ )
    {
        RpMaterial* material;

        /* new material */
        if( matindex[i] < 0 )
        {
            if( !RwStreamFindChunk( stream, (RwUInt32)rwID_MATERIAL,
                (RwUInt32*)NULL, &version ) )
            {
                free( matindex );
                _rpMaterialListDeinitialize( matList );
                return false;
            }
            if( !( material = _RpMaterialStreamRead( stream ) ) )
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
            RpMaterialAddRef( material );
        }

        /* Add the material to the end of the list - this bumps up the
         * reference count
         */
        _rpMaterialListAppendMaterial( matList, material );

        /* We want to drop the reference count back down, so that only
         * the material list owns it (so it goes away when the material
         * list does.
         */
        _RpMaterialDestroy( material );
    }

    free( matindex );
    return true;
}