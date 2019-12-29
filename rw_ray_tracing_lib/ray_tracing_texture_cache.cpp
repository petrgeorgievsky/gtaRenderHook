#include "ray_tracing_texture_cache.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/D3D11Impl/ImageBuffers/D3D11Texture2D.h>
#include <Engine/IRenderer.h>

using namespace rh::engine;
using namespace rh::debug;

rw_raytracing_lib::RayTracingTextureCache::RayTracingTextureCache()
{
    IGPUAllocator *allocator = g_pRHRenderer->GetGPUAllocator();
    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::DynamicTextureArrayBuffer;
    createInfo_.width = 1024;
    createInfo_.height = 1024;
    createInfo_.depth = 512;
    createInfo_.mipLevels = 9;
    createInfo_.format = ImageBufferFormat::BC1;
    allocator->AllocateImageBuffer( createInfo_, m_pTexturePool );
    for ( auto i = 0; i < 9; i++ )
        mTextureIdCounters[i] = 0;
}

bool rw_raytracing_lib::RayTracingTextureCache::PushTexture( void *ptr )
{
    DebugLogger::Log( "RayTracingTextureCache::PushTexture begin" );
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    D3D11Texture2D *texture_ptr = static_cast<D3D11Texture2D *>( ptr );
    auto texture_info = texture_ptr->GetTextureInfo();
    if ( texture_info.format != DXGI_FORMAT_BC1_UNORM )
        return false;
    uint32_t max_size = max( texture_info.width, texture_info.height );
    if ( max_size > 1024 )
        return false;
    uint8_t mip_slice = 10 - static_cast<uint8_t>( log2( max_size ) );
    uint8_t scale_x = max_size / texture_info.width;
    uint8_t scale_y = max_size / texture_info.height;
    uint32_t texture_id = mTextureIdCounters[mip_slice] + 1;
    context->CopyImageBuffer( m_pTexturePool, texture_ptr, texture_id * 9 + mip_slice, 0, 0, 0 );
    mTextureIdCounters[mip_slice]++;
    m_pTextureAddressCache[ptr] = {texture_id, mip_slice, scale_x, scale_y, 0};
    DebugLogger::Log( "TextureSize=" + std::to_string( texture_info.width ) + "x"
                      + std::to_string( texture_info.height ) + "\n" );
    return true;
}

rw_raytracing_lib::TextureAddress *rw_raytracing_lib::RayTracingTextureCache::GetTextureAddress(
    void *ptr )
{
    if ( ptr == nullptr )
        return nullptr;

    if ( m_pTextureAddressCache.find( ptr ) == m_pTextureAddressCache.end() )
        if ( !PushTexture( ptr ) )
            return nullptr;

    return &m_pTextureAddressCache[ptr];
}
