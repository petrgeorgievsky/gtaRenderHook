#include "rp_matlist_funcs.h"
#include "rp_material_funcs.h"

void RH_RWAPI::_rpMaterialListInitialize( RpMaterialList& matList )
{
    matList.space = 0;
    matList.materials = nullptr;
    matList.numMaterials = 0;
}

void RH_RWAPI::_rpMaterialListDeinitialize( RpMaterialList& matList )

{
    RpMaterial** materialArray;

    materialArray = matList.materials;
    if( materialArray )
    {
        RwInt32             materialCount = matList.numMaterials;
        RwInt32             nI;

        for( nI = 0; nI < materialCount; nI++ )
        {
            _RpMaterialDestroy( materialArray[nI] );
            materialArray[nI] = nullptr;
        }

        free( materialArray );
        materialArray = nullptr;
        matList.materials = materialArray;
    }

    /* Reset the structure */
    matList.numMaterials = 0;
    matList.space = 0;
}

bool RH_RWAPI::_rpMaterialListSetSize( RpMaterialList& matList, RwInt32 size )
{

    if( matList.space < size )
    {
        RpMaterial** materials;
        size_t memSize = sizeof( RpMaterial* ) * size;

        if( matList.materials )
        {
            materials = (RpMaterial * *)realloc( matList.materials, memSize );
        }
        else
        {
            materials = (RpMaterial * *)malloc( memSize );
        }

        if( !materials )
        {
            //RWERROR( ( E_RW_NOMEM, memSize ) );
            return false;
        }

        /* Shove in the new */
        matList.materials = materials;
        matList.space = size;
    }

    return true;
}

RwInt32 RH_RWAPI::_rpMaterialListAppendMaterial( RpMaterialList& matList, RpMaterial* material )
{
    RpMaterial** materials;
    RwUInt32            count;
    size_t              memSize;

    /* See if there is a blank entry we can use */
    /* Add it to the list */
    if( matList.space > matList.numMaterials )
    {
        materials = &( matList.materials )[matList.numMaterials];

        ( *materials ) = material;
        RpMaterialAddRef( material );

        matList.numMaterials++;

        return ( matList.numMaterials - 1 );
    }

    /* Need to allocates some more space */
    count = ( matList.space +
              rwMATERIALLISTGRANULARITY );

    memSize = sizeof( RpMaterial* ) * count;

    if( matList.materials )
    {
        materials =
            (RpMaterial * *)realloc( matList.materials, memSize );
    }
    else
    {
        materials = (RpMaterial * *)malloc( count * sizeof( RpMaterial* ) );//_rpMaterialListAlloc( count );
    }

    if( !materials )
    {
        return -1;
    }

    /* Shove in the new */
    matList.materials = materials;
    matList.space += rwMATERIALLISTGRANULARITY;

    materials[matList.numMaterials] = material;
    RpMaterialAddRef( material );
    matList.numMaterials++;

    return ( matList.numMaterials - 1 );
}