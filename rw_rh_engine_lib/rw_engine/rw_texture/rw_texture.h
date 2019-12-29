#pragma once
#include <cstdint>
struct RwTexture;
struct RwRaster;

namespace rh::rw::engine {

enum class RwRenderShaderStage : uint32_t {
    VertexShader,
    GeometryShader,
    HullShader,
    DomainShader,
    PixelShader,
    ComputeShader
};

RwTexture *RwTextureStreamRead( void *stream );

RwTexture *RwTextureCreate( RwRaster *raster );

int32_t RwTextureDestroy( RwTexture *texture );

int32_t RwD3D11SetTexture( void *context, RwTexture *texture, uint32_t reg_id, uint32_t stage );

} // namespace rw_rh_engine
