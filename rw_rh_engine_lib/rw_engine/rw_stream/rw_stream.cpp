#include "rw_stream.h"
#include <common_headers.h>

#define RWLIBRARYIDPACK( _version, _buildNum ) \
    ( ( ( ( (_version) -0x30000U ) & 0x3ff00U ) << 14 ) | ( ( (_version) &0x0003fU ) << 16 ) \
      | ( (_buildNum) &0xffffU ) )

#define RWLIBRARYIDUNPACKVERSION( _libraryID ) \
    ( ( ( ( ( _libraryID ) >> 14 ) & 0x3ff00U ) + 0x30000U ) \
      | ( ( ( _libraryID ) >> 16 ) & 0x0003fU ) )

#define RWLIBRARYIDUNPACKBUILDNUM( _libraryID ) ( (_libraryID) &0xffffU )

uint32_t rh::rw::engine::RwStreamRead( void *stream, void *buffer, uint32_t length )
{
    auto *stream_ = static_cast<std::istream *>( stream );

    stream_->read( static_cast<char *>( buffer ), length );

    return static_cast<uint32_t>( stream_->gcount() );
}

bool rh::rw::engine::RwStreamFindChunk( void *stream,
                                      uint32_t type,
                                      uint32_t *lengthOut,
                                      uint32_t *versionOut )
{
    uint32_t readType;
    uint32_t readLength;
    uint32_t readVersion;
    auto *stream_ = static_cast<std::istream *>( stream );

    while ( _rwStreamReadChunkHeader( stream_, &readType, &readLength, &readVersion, nullptr ) ) {
        if ( readType == type ) {
            if ( lengthOut ) {
                *lengthOut = readLength;
            }
            if ( versionOut ) {
                *versionOut = readVersion;
            }
            return true;
        }
        if ( readLength > 0 ) {
            stream_->seekg( readLength, std::ios_base::cur );
        }
    }

    return false;
}

static char *StringStreamRead( char *nativeString, void *stream, uint32_t length )
{
    char multiByteString[64];
    char *baseString;
    bool mallocced;
    if ( nativeString == nullptr ) {
        nativeString = static_cast<char *>( malloc( length * sizeof( char ) ) );
        if ( !nativeString ) {
            return nullptr;
        }
        mallocced = true;
    }

    baseString = nativeString;
    while ( length > 0 ) {
        uint32_t bytesToRead = ( length > 64 ) ? 64 : length;
        uint32_t i;

        if ( rh::rw::engine::RwStreamRead( stream, multiByteString, bytesToRead ) != bytesToRead ) {
            return nullptr;
        }

        /* Reduce by the amount we read */
        length -= bytesToRead;

        for ( i = 0; i < bytesToRead; i++ ) {
            baseString[i] = static_cast<char>( multiByteString[i] );
        }
        baseString += bytesToRead;
    }

    return ( nativeString );
}

char *rh::rw::engine::_rwStringStreamFindAndRead( char *string, void *stream )
{
    uint32_t readType;
    uint32_t readLength;
    uint32_t readVersion;
    auto *stream_ = static_cast<std::istream *>( stream );
    while ( _rwStreamReadChunkHeader( stream_, &readType, &readLength, &readVersion, nullptr ) ) {
        if ( readType == rwID_STRING ) {
            return ( StringStreamRead( string, stream, readLength ) );
        }

        if ( readLength > 0 ) {
            stream_->seekg( readLength, std::ios_base::cur );
        }
    }
    return nullptr;
}

struct __rwMark
{
    uint32_t type;
    uint32_t length;
    uint32_t libraryID;
};

bool rh::rw::engine::_rwStreamReadChunkHeader(
    std::istream *stream, uint32_t *type, uint32_t *length, uint32_t *version, uint32_t *buildNum )
{
    __rwMark mark{};
    RwChunkHeaderInfo chunkHdrInfo;

    if ( stream->eof() || stream->bad() )
        return false;

    stream->read( reinterpret_cast<char *>( &mark ), sizeof( mark ) );

    chunkHdrInfo.type = mark.type;
    chunkHdrInfo.length = mark.length;

    /* Check for old library ID */
    if ( !( mark.libraryID & 0xffff0000 ) ) {
        /* Just contains old-style version number */
        chunkHdrInfo.version = mark.libraryID << 8;
        chunkHdrInfo.buildNum = 0;
    } else {
        /* Unpack new library ID */
        chunkHdrInfo.version = RWLIBRARYIDUNPACKVERSION( mark.libraryID );
        chunkHdrInfo.buildNum = RWLIBRARYIDUNPACKBUILDNUM( mark.libraryID );
    }
    // TODO
    chunkHdrInfo.isComplex = true; // ChunkIsComplex( &chunkHdrInfo );

    if ( type ) {
        *type = chunkHdrInfo.type;
    }

    if ( length ) {
        *length = chunkHdrInfo.length;
    }

    if ( buildNum ) {
        *buildNum = chunkHdrInfo.buildNum;
    }

    if ( version ) {
        *version = chunkHdrInfo.version;
    }
    return true;
}
