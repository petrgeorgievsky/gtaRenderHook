#pragma once
#include <iostream>

namespace rh::rw::engine {

bool _rwStreamReadChunkHeader(
    std::istream *stream, uint32_t *type, uint32_t *length, uint32_t *version, uint32_t *buildNum );

char *_rwStringStreamFindAndRead( char *string, void *stream );

bool RwStreamFindChunk( void *stream, uint32_t type, uint32_t *lengthOut, uint32_t *versionOut );

uint32_t RwStreamRead( void *stream, void *buffer, uint32_t length );

} // namespace rw_rh_engine
