#include "per_pixel_gi_pass.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/constant_buffer_info.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/Common/types/shader_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/IRenderer.h>
#include <rw_engine/rw_macro_constexpr.h>

#include <fstream>

using namespace rw_raytracing_lib;
using namespace rh::engine;

rh::engine::IGPUResource *ReadBMP( const std::string &path )
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();

    std::ifstream file( path );
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER info_header;
    file.read( reinterpret_cast<char *>( &header ), sizeof( header ) );
    file.read( reinterpret_cast<char *>( &info_header ), sizeof( info_header ) );
    std::size_t tex_size = static_cast<std::size_t>(
        info_header.biBitCount / 8 * info_header.biWidth
        * info_header.biHeight ); // info_header.biSizeImage;
    file.seekg( 0, std::ios_base::end );

    std::size_t fsize = static_cast<std::size_t>( file.tellg() );
    char *texture_full = static_cast<char *>( malloc( fsize ) );
    file.close();
    // std::vector<char> texture_buffer( tex_size, '\0' );

    char *texture_buff = static_cast<char *>( malloc( tex_size ) );

    FILE *fhandle = nullptr;
    fopen_s( &fhandle, path.c_str(), "rb" );
    // file.open( path );
    // std::streamsize currg = file.tellg();
    // assert( currg == header.bfOffBits );
    // file.read( texture_full, fsize );
    fread( texture_full, sizeof( char ), fsize, fhandle );
    std::streamsize read_gc = 0;
    fgetpos( fhandle, &read_gc );

    assert( read_gc == fsize );
    fclose( fhandle );
    memcpy( texture_buff, texture_full + header.bfOffBits, tex_size );
    // assert( !file.failbit );
    // assert( !file.badbit );
    // assert( !file.eofbit );
    std::vector<uint32_t> texture_buffer_cvt( static_cast<uint32_t>( info_header.biWidth
                                                                     * info_header.biHeight ),
                                              0 );
    for ( size_t px = 0; px < texture_buffer_cvt.size(); px++ ) {
        texture_buffer_cvt[px]
            = rh::rw::engine::rwRGBA::Long( static_cast<uint8_t>( texture_buff[px * 3] ),
                                          static_cast<uint8_t>( texture_buff[px * 3 + 1] ),
                                          static_cast<uint8_t>( texture_buff[px * 3 + 2] ),
                                          0xFF );
    }
    free( texture_buff );
    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = static_cast<uint32_t>( info_header.biWidth );
    createInfo_.height = static_cast<uint32_t>( info_header.biHeight );
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RGBA8;
    createInfo_.initialDataVec = {{createInfo_.width * 4, texture_buffer_cvt.data()}};

    rh::engine::IGPUResource *res;
    allocator->AllocateImageBuffer( createInfo_, res );

    return res;
}

RTPerPixelGIPass::~RTPerPixelGIPass()
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    allocator->FreeImageBuffer( mDispatchParams.target, ImageBufferType::RenderTargetBuffer );
    allocator->FreeImageBuffer( mSHCoCgBuff, ImageBufferType::RenderTargetBuffer );
    allocator->FreeImageBuffer( mBlueNoise, ImageBufferType::TextureBuffer );
}

RTPerPixelGIPass::RTPerPixelGIPass( size_t w, size_t h, float scale )
{
    mBlueNoise = ReadBMP( "rh_resources/blue_noise.bmp" );
    mGIParamsData.fRenderingScale = scale;
    mGIParamsData.fNoiseScale = 64;
    mGIParamsData.fRaytracingDistance = 2000.0f;
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    mDispatchParams.dispatchThreadsX = static_cast<uint32_t>( w * scale / mThreadGroupSize );
    mDispatchParams.dispatchThreadsY = static_cast<uint32_t>( h * scale / mThreadGroupSize );
    mDispatchParams.dispatchThreadsZ = 1;
    allocator->AllocateShader( {"shaders/d3d11/per_pixel_gi.hlsl",
                                "TraceRays",
                                ShaderStage::Compute},
                               mDispatchParams.shader );
    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = static_cast<uint32_t>( w * scale );
    createInfo_.height = static_cast<uint32_t>( h * scale );
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RGBA16;
    allocator->AllocateImageBuffer( createInfo_, mDispatchParams.target );

    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = static_cast<uint32_t>( w * scale );
    createInfo_.height = static_cast<uint32_t>( h * scale );
    createInfo_.mipLevels = 1;
    createInfo_.format = ImageBufferFormat::RG16;
    allocator->AllocateImageBuffer( createInfo_, mSHCoCgBuff );

    ConstantBufferInfo cb_info{};
    cb_info.size = sizeof( PerPixelGIParams );
    allocator->AllocateConstantBuffer( cb_info, mGIParamsBuffer );

    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    context->UpdateBuffer( mGIParamsBuffer, reinterpret_cast<void *>( &mGIParamsData ), -1 );
}

void *RTPerPixelGIPass::Execute( RayTracer *ray_tracer, void *gbPos, void *gbNormals )
{
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    context->BindConstantBuffers( ShaderStage::Compute, {{1, mGIParamsBuffer}} );
    context->BindImageBuffers( ImageBindType::CSResource,
                               {{4, gbPos}, {5, gbNormals}, {6, mBlueNoise}} );
    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{1, mSHCoCgBuff}} );
    ray_tracer->TraceRays( mDispatchParams );
    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{1, nullptr}} );
    context->BindImageBuffers( ImageBindType::CSResource,
                               {{4, nullptr}, {5, nullptr}, {6, nullptr}} );
    // context->BindConstantBuffers( RHShaderStage::Compute, {{1, nullptr}} );
    context->FlushCache();
    return mDispatchParams.target;
}

void *RTPerPixelGIPass::GetSHCoCg()
{
    return mSHCoCgBuff;
}
