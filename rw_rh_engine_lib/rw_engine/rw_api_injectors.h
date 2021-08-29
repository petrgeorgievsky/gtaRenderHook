#pragma once
#include <cstdint>
struct RwRaster;
struct RwTexture;
struct RwResEntry;

namespace rh::rw::engine
{
using RwStreamFindChunk_FN = bool ( * )( void *, uint32_t, uint32_t *,
                                         uint32_t * );
using RwStreamRead_FN      = uint32_t (      *)( void *, void *, uint32_t );

using RwStreamWriteVersionedChunkHeader_FN = void *(*)( void *, int32_t,
                                                        int32_t, uint32_t,
                                                        uint32_t );
using RwStreamWrite_FN = void *(*)( void *, const void *, uint32_t );

using RwRasterCreate_FN = RwRaster *(*)( int32_t, int32_t, int32_t, int32_t );

using RwRasterDestroy_FN = int32_t ( * )( RwRaster * );

using RwTextureCreate_FN  = RwTexture *(*)( RwRaster  *);
using RwTextureSetName_FN = RwTexture *(*)( RwTexture *, const char * );

struct RwIOPointerTable
{
    RwStreamFindChunk_FN                 fpFindChunk;
    RwStreamRead_FN                      fpRead;
    RwStreamWriteVersionedChunkHeader_FN fpWriteChunkHeader;
    RwStreamWrite_FN                     fpWrite;
};

struct RwRasterPointerTable
{
    RwRasterCreate_FN  fpCreateRaster;
    RwRasterDestroy_FN fpDestroyRaster;
};

struct RwTexturePointerTable
{
    RwTextureCreate_FN  fpCreateTexture;
    RwTextureSetName_FN fpTextureSetName;
    RwTextureSetName_FN fpTextureSetMaskName;
};

extern RwIOPointerTable      g_pIO_API;
extern RwRasterPointerTable  g_pRaster_API;
extern RwTexturePointerTable g_pTexture_API;

} // namespace rh::rw::engine
