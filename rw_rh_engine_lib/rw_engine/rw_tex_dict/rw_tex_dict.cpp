#include "rw_tex_dict.h"
#include "../rw_macro_constexpr.h"
#include "../rw_stream/rw_stream.h"
#include "../rw_texture/rw_texture.h"
#include "rw_engine/system_funcs/rw_device_system_globals.h"
#include <fstream>
#include <rw_engine/system_funcs/rw_device_standards.h>
#include <rw_engine/test_heap_allocator.h>

struct _rwStreamTexDictionary
{
    uint16_t numTextures;
    uint16_t deviceId;
};

RwTexDictionary *rh::rw::engine::RwTexDictionaryCreate()
{
    RwTexDictionary *dict;

    dict = hAlloc<RwTexDictionary>( "TextureDict" );
    if ( !dict )
        return nullptr;

    constexpr auto rwTEXDICTIONARY = 6;
    rwObject::Initialize( dict, rwTEXDICTIONARY, 0 );
    rwLinkList::Initialize( &dict->texturesInDict );

    return dict;
}

int32_t rh::rw::engine::RwTexDictionaryDestroy( RwTexDictionary *dict )
{
    /* Destroy all the textures */
    rh::rw::engine::RwTexDictionaryForAllTextures(
        dict, reinterpret_cast<RwTextureCallBack>( RwTextureDestroy ),
        nullptr );

    /* De-initialize the plugin memory */

    /* Remove from the free list */
    // rwLinkListRemoveLLLink( &dict->lInInstance );

    hFree( dict );

    /* Success */
    return TRUE;
}

const RwTexDictionary *rh::rw::engine::RwTexDictionaryForAllTextures(
    const RwTexDictionary *dict, RwTextureCallBack fpCallBack, void *pData )
{
    RwLLLink *      cur, *next;
    const RwLLLink *end;

    end = rwLinkList::GetTerminator( &dict->texturesInDict );
    cur = rwLinkList::GetFirstLLLink( &dict->texturesInDict );

    while ( cur != end )
    {
        RwTexture *texture;

        next = rwLLLink::GetNext( cur );

        texture = rwLLLink::GetData<RwTexture>(
            cur, offsetof( RwTexture, lInDictionary ) );

        if ( !fpCallBack( texture, pData ) )
        {
            /* Early out */
            break;
        }

        cur = next;
    }

    /* All OK */
    return dict;
}

RwTexture *rh::rw::engine::RwTexDictionaryAddTexture( RwTexDictionary *dict,
                                                      RwTexture *      texture )
{
    if ( texture->dict )
    {
        rwLinkList::RemoveLLLink( &texture->lInDictionary );
    }

    texture->dict = dict;

    rwLinkList::AddLLLink( &dict->texturesInDict, &texture->lInDictionary );
    return texture;
}

RwTexDictionary *rh::rw::engine::RwTexDictionaryStreamRead( void *stream )
{
    uint32_t               lengthOut, versionOut;
    _rwStreamTexDictionary binDict{};
    RwTexDictionary *      result = RwTexDictionaryCreate();
    RwTexture *            texture;

    if ( !RwStreamFindChunk( stream, rwID_STRUCT, &lengthOut, &versionOut ) )
        return nullptr;

    RwStreamRead( stream, &binDict, sizeof( binDict ) );

    while ( binDict.numTextures-- )
    {
        uint32_t size, version;
        texture = nullptr;

        if ( !RwStreamFindChunk( stream, rwID_TEXTURENATIVE, &size, &version ) )
            return nullptr;

        if ( !gRwDeviceGlobals.Standards[rwSTANDARDNATIVETEXTUREREAD](
                 static_cast<void *>( stream ), &texture,
                 static_cast<int32_t>( size ) ) )

            return nullptr;
        rh::rw::engine::RwTexDictionaryAddTexture( result, texture );
    }

    return result;
}

RwTexDictionary *
rh::rw::engine::GTAReadTexDict( const rh::engine::String &fileName )
{
    std::ifstream stream( fileName, std::ios_base::in | std::ios_base::binary );

    if ( !stream )
        return RwTexDictionaryCreate();

    if ( !RwStreamFindChunk( &stream, 0x16, nullptr, nullptr ) )
        return RwTexDictionaryCreate();

    RwTexDictionary *result = RwTexDictionaryStreamRead( &stream );

    if ( !result )
        return RwTexDictionaryCreate();

    return result;
}
