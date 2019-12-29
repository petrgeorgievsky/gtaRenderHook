#pragma once
#include <common_headers.h>
namespace rh::engine {
enum class ImageBufferFormat : uint8_t;
enum class SamplerAddressing : uint8_t;
enum class BlendOp : uint8_t;
enum class CullMode : uint8_t;
} // namespace rh::engine

namespace rh::rw::engine {
rh::engine::ImageBufferFormat RwFormatToRHImageBufferFormat( RwRasterFormat format );
rh::engine::ImageBufferFormat RwNativeFormatToRHImageBufferFormat( D3DFORMAT format,
                                                                   RwRasterFormat rwFormat,
                                                                   RwPlatformID platform,
                                                                   uint32_t dxt_compression );

rh::engine::SamplerAddressing RwTextureAddressModeToRHSamplerAddressing( RwTextureAddressMode mode );

rh::engine::BlendOp RwBlendFunctionToRHBlendOp( RwBlendFunction func );

rh::engine::CullMode RwCullModeToRHCullMode( RwCullMode mode );
} // namespace rw_rh_engine
