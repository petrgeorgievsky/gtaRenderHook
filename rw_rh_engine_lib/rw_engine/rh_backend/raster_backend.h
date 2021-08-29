#pragma once
#include <cstdint>
#include <vector>

struct RwRaster;

namespace rh
{
namespace rw::engine
{

struct BackendRasterExt;
struct PluginPtrTable;

/**
 *
 */
class BackendRasterPlugin
{
  public:
    BackendRasterPlugin( const PluginPtrTable &plugin_cb );
    static uint8_t *         GetAddress( RwRaster *raster );
    static BackendRasterExt &GetData( RwRaster *raster );

    static constexpr uint64_t NullRasterId = 0xBADF00D;

  private:
    static int32_t Offset;
};

struct BackendRasterExt
{
    uint64_t mImageId;
    uint32_t OriginalFormat;
    uint8_t  MipCount;
    uint8_t  BytesPerBlock;
    uint8_t  BlockSize;
    bool     HasAlpha;
    bool     Compressed;

    ~BackendRasterExt();
    BackendRasterExt();
    BackendRasterExt( const BackendRasterExt & )          = delete;
    BackendRasterExt( BackendRasterExt &&other ) noexcept = delete;
    BackendRasterExt &operator=( const BackendRasterExt &other ) = delete;
    BackendRasterExt &operator=( BackendRasterExt &&other ) = delete;
};

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
