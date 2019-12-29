#include "ray_tracer.h"
#include <Engine/Common/IGPUAllocator.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/structured_buffer_info.h>
#include <Engine/D3D11Impl/Buffers/D3D11StructuredBuffer.h>
#include <Engine/D3D11Impl/ImageBuffers/D3D11Texture2D.h>
#include <Engine/D3D11Impl/ImageBuffers/ImageBuffer.h>
#include <Engine/D3D11Impl/Shaders/D3D11Shader.h>
#include <Engine/IRenderer.h>

using namespace rw_raytracing_lib;
using namespace rh::engine;

void RayTracer::AllocateTLAS( const TLAS_BVH &bvh )
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    if ( mTLAS_BVHBuffer == nullptr ) {
        StructuredBufferInfo tlas_info{};
        tlas_info.elementSize = sizeof( LinearBVHNodeTLAS );
        tlas_info.elementCount = 8000;
        allocator->AllocateStructuredBuffer( tlas_info, mTLAS_BVHBuffer );
    }
    context->UpdateBuffer( mTLAS_BVHBuffer,
                           bvh.tlas.data(),
                           bvh.tlas.size() * sizeof( LinearBVHNodeTLAS ) );
}

void RayTracer::AllocatePackedBLAS( const PackedBLAS_BVH &bvh )
{
    auto allocator = g_pRHRenderer->GetGPUAllocator();
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    if ( mBVHLeafBuffer == nullptr ) {
        StructuredBufferInfo bvh_nodes_info{};
        bvh_nodes_info.elementSize = sizeof( LinearBVHNode );
        bvh_nodes_info.elementCount = 2000000;
        allocator->AllocateStructuredBuffer( bvh_nodes_info, mBVHLeafBuffer );
    }
    context->UpdateBuffer( mBVHLeafBuffer,
                           bvh.blas.nodes.data(),
                           bvh.blas.nodes.size() * sizeof( LinearBVHNode ) );

    if ( mTriangleBuffer == nullptr ) {
        StructuredBufferInfo ordered_triangles_info{};
        ordered_triangles_info.elementSize = sizeof( RpTriangle );
        ordered_triangles_info.elementCount = 1000000;
        allocator->AllocateStructuredBuffer( ordered_triangles_info, mTriangleBuffer );
    }
    context->UpdateBuffer( mTriangleBuffer,
                           bvh.blas.ordered_triangles.data(),
                           bvh.blas.ordered_triangles.size() * sizeof( RpTriangle ) );

    if ( mVertexBuffer == nullptr ) {
        StructuredBufferInfo vertex_info{};
        vertex_info.elementSize = sizeof( RTVertex );
        vertex_info.elementCount = 2000000;
        allocator->AllocateStructuredBuffer( vertex_info, mVertexBuffer );
    }
    context->UpdateBuffer( mVertexBuffer,
                           bvh.blas.vertices.data(),
                           bvh.blas.vertices.size() * sizeof( RTVertex ) );
}

void RayTracer::TraceRays( const TraceRaysDispatchParams &params )
{
    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );

    context->BindShader( params.shader );

    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{0, params.target}} );

    context->BindImageBuffers( ImageBindType::CSResource,
                               {{0, mTriangleBuffer},
                                {1, mVertexBuffer},
                                {2, mBVHLeafBuffer},
                                {3, mTLAS_BVHBuffer}} );

    context->FlushCache();

    context->DispatchThreads( params.dispatchThreadsX,
                              params.dispatchThreadsY,
                              params.dispatchThreadsZ );

    context->ResetShader( params.shader );

    context->BindImageBuffers( ImageBindType::UnorderedAccessTarget, {{0, nullptr}} );
    context->BindImageBuffers( ImageBindType::CSResource,
                               {{0, nullptr}, {1, nullptr}, {2, nullptr}, {3, nullptr}} );
}
