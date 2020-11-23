#include "global_definitions.h"
#include "rw_api_injectors.h"
void *&rh::rw::engine::GetInternalRaster( RwRaster *raster )
{
    void **internalRaster = reinterpret_cast<void **>( ( reinterpret_cast<uint8_t *>( raster ) )
                                                       + *g_pGlobal_API.opInternalRasterExtOffset );

    return *internalRaster;
}
