#pragma once
#include <common.h>

namespace RH_RWAPI
{
    using RwStreamFindChunk_FN = bool (*)(void *, uint32_t, uint32_t *, uint32_t *);
    using RwStreamRead_FN = uint32_t (*)(void *, void *, uint32_t);

    using RwRasterCreate_FN = RwRaster *(*)(RwInt32, RwInt32, RwInt32, RwInt32);

    using RwRasterDestroy_FN = RwBool (*)(RwRaster *);

    using RwTextureCreate_FN = RwTexture *(*)(RwRaster *);
    using RwTextureSetName_FN = RwTexture *(*)(RwTexture *, const RwChar *);

    struct RwIOPointerTable
    {
        RwStreamFindChunk_FN fpFindChunk;
        RwStreamRead_FN fpRead;
    };
    struct RwRasterPointerTable
    {
        RwRasterCreate_FN fpCreateRaster;
        RwRasterDestroy_FN fpDestroyRaster;
    };
    struct RwTexturePointerTable
    {
        RwTextureCreate_FN fpCreateTexture;
        RwTextureSetName_FN fpTextureSetName;
        RwTextureSetName_FN fpTextureSetMaskName;
        //RwRasterDestroy_FN fpDestroyRaster;
    };
    struct RwGlobalPointerTable
    {
        RwSystemFunc fpDefaultSystem;
        uint32_t *opInternalRasterExtOffset;
    };
    extern RwIOPointerTable g_pIO_API;
    extern RwGlobalPointerTable g_pGlobal_API;
    extern RwRasterPointerTable g_pRaster_API;
    extern RwTexturePointerTable g_pTexture_API;
    extern uint32_t g_oInternalRasterExtOffset;
};