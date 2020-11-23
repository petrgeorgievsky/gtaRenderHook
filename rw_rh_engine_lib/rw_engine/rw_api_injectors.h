#pragma once
#include <cstdint>
struct RwRaster;
struct RwTexture;
struct RwResEntry;

namespace rh::rw::engine
{
using RwResEntryDestroyNotify = void ( * )( RwResEntry *resEntry );

using RwStreamFindChunk_FN = bool ( * )( void *, uint32_t, uint32_t *,
                                         uint32_t * );
using RwStreamRead_FN      = uint32_t ( * )( void *, void *, uint32_t );

using RwRasterCreate_FN = RwRaster *(*)( int32_t, int32_t, int32_t, int32_t );

using RwRasterDestroy_FN = int32_t ( * )( RwRaster * );

using RwTextureCreate_FN  = RwTexture *(*)( RwRaster * );
using RwTextureSetName_FN = RwTexture *(*)( RwTexture *, const char * );
using RwResourcesAllocateResEntry_FN =
    RwResEntry *(*)( void *, RwResEntry **, int32_t, RwResEntryDestroyNotify );

struct RwIOPointerTable
{
    RwStreamFindChunk_FN fpFindChunk;
    RwStreamRead_FN      fpRead;
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
    // RwRasterDestroy_FN fpDestroyRaster;
};
/*
struct RwGlobalPointerTable
{
    // RwSystemFunc                   fpDefaultSystem;
    uint32_t *                     opInternalRasterExtOffset;
    RwResourcesAllocateResEntry_FN fpResourcesAllocateResEntry;
};*/

extern RwIOPointerTable g_pIO_API;
// extern RwGlobalPointerTable  g_pGlobal_API;
extern RwRasterPointerTable  g_pRaster_API;
extern RwTexturePointerTable g_pTexture_API;
extern uint32_t              g_oInternalRasterExtOffset;

} // namespace rh::rw::engine
