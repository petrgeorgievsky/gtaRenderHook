#include "ray_tracer.h"
#include "rw_engine/rw_macro_constexpr.h"
#include <Engine/Common/IGPUAllocator.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/D3D11Impl/Buffers/D3D11StructuredBuffer.h>
#include <Engine/D3D11Impl/ImageBuffers/D3D11Texture2D.h>
#include <Engine/D3D11Impl/ImageBuffers/ImageBuffer.h>
#include <Engine/D3D11Impl/Shaders/D3D11Shader.h>
#include <Engine/IRenderer.h>

void *ReadBMP( const std::string &path )
{
    auto allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();

    std::ifstream file( path );
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER info_header;
    file.read( reinterpret_cast<char *>( &header ), sizeof( header ) );
    file.read( reinterpret_cast<char *>( &info_header ), sizeof( info_header ) );
    size_t tex_size = info_header.biSizeImage;
    std::vector<uint8_t> texture_buffer( tex_size, '\0' );
    file.seekg( header.bfOffBits, std::ios_base::beg );
    file.read( reinterpret_cast<char *>( texture_buffer.data() ), texture_buffer.size() );
    std::vector<uint32_t> texture_buffer_cvt( static_cast<uint32_t>( info_header.biWidth
                                                                     * info_header.biHeight ),
                                              0 );
    for ( size_t px = 0; px < texture_buffer_cvt.size(); px++ ) {
        texture_buffer_cvt[px] = rw_rh_engine::rwRGBA::Long( texture_buffer[px * 3],
                                                             texture_buffer[px * 3 + 1],
                                                             texture_buffer[px * 3 + 2],
                                                             0xFF );
    }
    RHEngine::ImageBufferInfo createInfo_{};
    createInfo_.type = RHEngine::ImageBufferType::RenderTargetBuffer;
    createInfo_.width = static_cast<uint32_t>( info_header.biWidth );
    createInfo_.height = static_cast<uint32_t>( info_header.biHeight );
    createInfo_.mipLevels = 1;
    createInfo_.format = RHEngine::ImageBufferFormat::RGBA8;
    createInfo_.initialDataVec = {{createInfo_.width * 4, texture_buffer_cvt.data()}};

    void *res;
    allocator->AllocateImageBuffer( createInfo_, res );

    return res;
}

void RayTracer::Initialize()
{
    mBlueNoiseTex = ReadBMP( "blue_noise.bmp" );
    auto allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();
    allocator->AllocateShader( {"shaders/d3d11/raytrace.hlsl",
                                "TraceRays",
                                RHEngine::ShaderStage::Compute},
                               mRaytraceCS );
    allocator->AllocateShader( {"shaders/d3d11/composite.hlsl",
                                "Composite",
                                RHEngine::ShaderStage::Compute},
                               mCompositeCS );
    allocator->AllocateShader( {"shaders/d3d11/accumulate.hlsl",
                                "Accumulate",
                                RHEngine::ShaderStage::Compute},
                               mAccumulateCS );

    allocator->AllocateConstantBuffer( {sizeof( TimeCycle )}, mTimeCycleCB );

    RHEngine::ImageBufferInfo createInfo_{};
    createInfo_.type = RHEngine::ImageBufferType::RenderTargetBuffer;
    createInfo_.width = 1280;
    createInfo_.height = 720;
    createInfo_.mipLevels = 1;
    createInfo_.format = RHEngine::ImageBufferFormat::RGBA16;

    createInfo_.width = 1600;
    createInfo_.height = 900;
    allocator->AllocateImageBuffer( createInfo_, mRayTracingTarget[0] );
    allocator->AllocateImageBuffer( createInfo_, mRayTracingTarget[1] );
    allocator->AllocateImageBuffer( createInfo_, mRayTracingTarget[2] );
    allocator->AllocateImageBuffer( createInfo_, mCompositeTarget );

    createInfo_.width = ( 1280 / 8 );
    createInfo_.height = ( 720 / 8 );
    allocator->AllocateImageBuffer( createInfo_, mGICacheTarget[0] );
    allocator->AllocateImageBuffer( createInfo_, mGICacheTarget[1] );
    current_rt_tex = 0;
}

void RayTracer::AllocateBVHOnGPU( rw_raytracing_lib::TLAS_BVH &&bvh )
{
    auto allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();
    RHEngine::IRenderingContext *context = static_cast<RHEngine::IRenderingContext *>(
        RHEngine::g_pRHRenderer->GetCurrentContext() );

    if ( !mTLAS_BVHBuffer ) {
        RHEngine::StructuredBufferInfo tlas_info{};
        tlas_info.elementSize = sizeof( rw_raytracing_lib::LinearBVHNodeTLAS );
        tlas_info.elementCount = 8000;
        allocator->AllocateStructuredBuffer( tlas_info, mTLAS_BVHBuffer );
    }
    context->UpdateBuffer( mTLAS_BVHBuffer,
                           bvh.tlas.data(),
                           bvh.tlas.size() * sizeof( rw_raytracing_lib::LinearBVHNodeTLAS ) );
}

void RayTracer::UpdateTimecyc( const DirectX::XMFLOAT4 &topSkyColor,
                               const DirectX::XMFLOAT4 &bottomSkyColor,
                               const DirectX::XMFLOAT4 &sunDir )
{
    mTCBuf.vSunDir = sunDir;
    mTCBuf.cTopSkyColor = topSkyColor;
    mTCBuf.cBottomSkyColor = bottomSkyColor;
    RHEngine::IRenderingContext *context = static_cast<RHEngine::IRenderingContext *>(
        RHEngine::g_pRHRenderer->GetCurrentContext() );
    context->UpdateBuffer( mTimeCycleCB, &mTCBuf, -1 );
}

void *RayTracer::Accumulate( void *curr_tex )
{
    RHEngine::IRenderingContext *context = static_cast<RHEngine::IRenderingContext *>(
        RHEngine::g_pRHRenderer->GetCurrentContext() );
    context->BindShader( mAccumulateCS );
    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget,
                               {{0,

                                 mRayTracingTarget[1 - current_rt_tex]}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource,
                               {{0, curr_tex}, {1, mRayTracingTarget[current_rt_tex]}} );
    context->FlushCache();
    context->DispatchThreads( 1600 / 8, 900 / 8, 1 );
    context->ResetShader( mAccumulateCS );

    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget, {{0, nullptr}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource, {{0, nullptr}, {1, nullptr}} );
    context->FlushCache();
    current_rt_tex = 1 - current_rt_tex;
    return mRayTracingTarget[current_rt_tex];
}

void RayTracer::AllocatePackedBLASGPU( const rw_raytracing_lib::PackedBLAS_BVH &bvh )
{
    auto allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();
    RHEngine::IRenderingContext *context = static_cast<RHEngine::IRenderingContext *>(
        RHEngine::g_pRHRenderer->GetCurrentContext() );
    if ( mBVHLeafBuffer == nullptr ) {
        RHEngine::StructuredBufferInfo bvh_nodes_info{};
        bvh_nodes_info.elementSize = sizeof( rw_raytracing_lib::LinearBVHNode );
        bvh_nodes_info.elementCount = 2000000;
        allocator->AllocateStructuredBuffer( bvh_nodes_info, mBVHLeafBuffer );
    }
    context->UpdateBuffer( mBVHLeafBuffer,
                           bvh.blas.nodes.data(),
                           bvh.blas.nodes.size() * sizeof( rw_raytracing_lib::LinearBVHNode ) );

    if ( mTriangleBuffer == nullptr ) {
        RHEngine::StructuredBufferInfo ordered_triangles_info{};
        ordered_triangles_info.elementSize = sizeof( RpTriangle );
        ordered_triangles_info.elementCount = 1000000;
        allocator->AllocateStructuredBuffer( ordered_triangles_info, mTriangleBuffer );
    }
    context->UpdateBuffer( mTriangleBuffer,
                           bvh.blas.ordered_triangles.data(),
                           bvh.blas.ordered_triangles.size() * sizeof( RpTriangle ) );

    if ( mVertexBuffer == nullptr ) {
        RHEngine::StructuredBufferInfo vertex_info{};
        vertex_info.elementSize = sizeof( rw_raytracing_lib::RTVertex );
        vertex_info.elementCount = 2000000;
        allocator->AllocateStructuredBuffer( vertex_info, mVertexBuffer );
    }
    context->UpdateBuffer( mVertexBuffer,
                           bvh.blas.vertices.data(),
                           bvh.blas.vertices.size() * sizeof( rw_raytracing_lib::RTVertex ) );

    if ( mMaterialBuffer == nullptr ) {
        RHEngine::StructuredBufferInfo vertex_info{};
        vertex_info.elementSize = sizeof( rw_raytracing_lib::MaterialInfo );
        vertex_info.elementCount = 20000;
        allocator->AllocateStructuredBuffer( vertex_info, mMaterialBuffer );
    }
    context->UpdateBuffer( mMaterialBuffer,
                           bvh.blas.materials.data(),
                           bvh.blas.materials.size() * sizeof( rw_raytracing_lib::MaterialInfo ) );
}

void RayTracer::TraceRays(
    void *gbPos, void *gbNormals, void *gbColor, void *gbVertexRadiance, void *textureCache )
{
    RHEngine::IRenderingContext *context = static_cast<RHEngine::IRenderingContext *>(
        RHEngine::g_pRHRenderer->GetCurrentContext() );
    context->BindShader( mRaytraceCS );
    RHEngine::RHSampler sampler{};
    sampler.adressU = RHEngine::RHSamplerAddressing::Wrap;
    sampler.adressV = RHEngine::RHSamplerAddressing::Wrap;
    sampler.adressW = RHEngine::RHSamplerAddressing::Wrap;
    sampler.filtering = RHEngine::RHSamplerFilter::Anisotropic;
    sampler.comparison = RHEngine::RHComparisonFunc::Never;
    sampler.borderColor = {255, 255, 255, 255};
    context->BindSamplers( {{0, &sampler}}, RHEngine::ShaderStage::Compute );
    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget,
                               {{0, mRayTracingTarget[2]},
                                {1, mGICacheTarget[current_gi_cache]},
                                {2, mGICacheTarget[1 - current_gi_cache]}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource,
                               {{0, mTriangleBuffer},
                                {1, mVertexBuffer},
                                {2, mBVHLeafBuffer},
                                {3, mTLAS_BVHBuffer},
                                {4, gbPos},
                                {5, gbNormals},
                                {6, gbColor},
                                {7, gbVertexRadiance},
                                {8, mBlueNoiseTex},
                                {9, mMaterialBuffer},
                                {10, textureCache}} );
    context->BindConstantBuffers( RHEngine::ShaderStage::Compute, {{1, mTimeCycleCB}} );
    context->FlushCache();
    context->DispatchThreads( 1600 / 8, 900 / 8, 1 );
    context->ResetShader( mRaytraceCS );

    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget,
                               {{0, nullptr}, {1, nullptr}, {2, nullptr}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource,
                               {{0, nullptr},
                                {1, nullptr},
                                {2, nullptr},
                                {3, nullptr},
                                {4, nullptr},
                                {5, nullptr},
                                {6, nullptr},
                                {7, nullptr},
                                {8, nullptr},
                                {9, nullptr},
                                {10, nullptr}} );
    context->FlushCache();

    current_gi_cache = 1 - current_gi_cache;
}

void *RayTracer::Composite( void *blurred_tex, void *gbColor )
{
    RHEngine::IRenderingContext *context = static_cast<RHEngine::IRenderingContext *>(
        RHEngine::g_pRHRenderer->GetCurrentContext() );
    context->BindShader( mCompositeCS );
    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget,
                               {{0, mCompositeTarget}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource,
                               {{0, blurred_tex}, {1, gbColor}} );
    context->FlushCache();
    context->DispatchThreads( 1600 / 8, 900 / 8, 1 );
    context->ResetShader( mCompositeCS );

    context->BindImageBuffers( RHEngine::ImageBindType::UnorderedAccessTarget, {{0, nullptr}} );
    context->BindImageBuffers( RHEngine::ImageBindType::CSResource, {{0, nullptr}, {1, nullptr}} );
    context->FlushCache();

    return mCompositeTarget;
}

void *RayTracer::RaytracedTex()
{
    return mRayTracingTarget[2];
}
