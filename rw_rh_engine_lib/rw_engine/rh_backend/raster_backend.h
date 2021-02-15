#pragma once
#include <Engine/ResourcePool.h>
#include <cstdint>
#include <vector>

struct RwRaster;

namespace rh
{

namespace engine
{
class IWindow;
class ISyncPrimitive;
class IImageBuffer;
class IImageView;
} // namespace engine

namespace rw::engine
{

struct BackendRasterExt
{
    uint64_t mImageId;
};

constexpr uint64_t gNullRasterId = 0xBADF00D;
extern int32_t     gBackendRasterExtOffset;

/* Plugin Attach */
int32_t BackendRasterPluginAttach();
void *BackendRasterCtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject );
void *BackendRasterDtor( void *object, [[maybe_unused]] int32_t offsetInObject,
                         [[maybe_unused]] int32_t sizeInObject );
/* Open/Close */
// void BackendRasterOpen();
// void BackendRasterClose();

BackendRasterExt *GetBackendRasterExt( RwRaster *raster );

struct RasterHeader
{
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mDepth;
    uint32_t mFormat;
    uint32_t mMipLevelCount;
};

struct MipLevelHeader
{
    uint32_t mSize;
    uint32_t mStride;
};

} // namespace rw::engine
} // namespace rh
