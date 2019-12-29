#pragma once
#include <Engine/Common/types/string_typedefs.h>
#include <cstdint>

struct RwTexDictionary;
struct RwTexture;
using RwTextureCallBack = RwTexture *(*) ( RwTexture *texture, void *pData );

namespace rh::rw::engine {
RwTexDictionary *RwTexDictionaryCreate();

int32_t RwTexDictionaryDestroy( RwTexDictionary *dict );

const RwTexDictionary *RwTexDictionaryForAllTextures( const RwTexDictionary *dict,
                                                      RwTextureCallBack fpCallBack,
                                                      void *pData );
RwTexture *RwTexDictionaryAddTexture( RwTexDictionary *dict, RwTexture *texture );

RwTexDictionary *RwTexDictionaryStreamRead( void *stream );

RwTexDictionary *GTAReadTexDict( const rh::engine::String &fileName );
} // namespace rw_rh_engine
