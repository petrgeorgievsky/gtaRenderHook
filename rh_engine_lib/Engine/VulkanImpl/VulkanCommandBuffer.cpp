#include "VulkanCommandBuffer.h"
#include "SyncPrimitives/VulkanCPUSyncPrimitive.h"
#include "VulkanBuffer.h"
#include "VulkanConvert.h"
#include "VulkanDescriptorSet.h"
#include "VulkanFrameBuffer.h"
#include "VulkanImage.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"
#include <DebugUtils/DebugLogger.h>

using namespace rh::engine;

VulkanCommandBuffer::VulkanCommandBuffer( vk::Device        device,
                                          vk::CommandPool   pool,
                                          vk::CommandBuffer cmd_buffer )
    : m_vkCmdBuffer( cmd_buffer ), mPool( pool ), mDevice( device ),
      m_executionSyncPrim( new VulkanCPUSyncPrimitive( device ) )
{
}

ISyncPrimitive *VulkanCommandBuffer::ExecutionFinishedPrimitive()
{
    return m_executionSyncPrim;
}

void VulkanCommandBuffer::CopyBufferToImage(
    const BufferToImageCopyInfo &copy_info )
{
    vk::Image       img    = *static_cast<VulkanImage *>( copy_info.mImage );
    vk::Buffer      buffer = *static_cast<VulkanBuffer *>( copy_info.mBuffer );
    vk::ImageLayout layout = Convert( copy_info.mImageLayout );

    // SmallVectorArena<vk::BufferImageCopy, 8> arena;
    std::vector<vk::BufferImageCopy> image_copies{};

    std::ranges::for_each(
        copy_info.mRegions,
        [&image_copies]( const BufferToImageCopySubInfo &info )
        {
            vk::BufferImageCopy vk_info{};
            vk_info.bufferRowLength   = info.mFrom.mRowLength;
            vk_info.bufferImageHeight = info.mFrom.mRowCount;
            vk_info.bufferOffset      = info.mFrom.mOffset;
            vk_info.imageExtent       = vk::Extent3D{
                info.mTo.mExtentW, info.mTo.mExtentH, info.mTo.mExtentD };
            vk_info.imageOffset = vk::Offset3D{
                info.mTo.mOffsetX, info.mTo.mOffsetY, info.mTo.mOffsetZ };
            vk_info.imageSubresource = vk::ImageSubresourceLayers{
                vk::ImageAspectFlagBits::eColor, info.mTo.mSubresource.mipLevel,
                info.mTo.mSubresource.baseArrayLayer,
                info.mTo.mSubresource.layerCount };

            image_copies.push_back( vk_info );
        } );

    m_vkCmdBuffer.copyBufferToImage( buffer, img, layout, image_copies );
}

void rh::engine::VulkanCommandBuffer::PipelineBarrier(
    const PipelineBarrierInfo &info )
{
    vk::MemoryBarrier mem_barrier{};
    mem_barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
    mem_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    // SmallVectorArena<vk::ImageMemoryBarrier, 8> arena;
    std::vector<vk::ImageMemoryBarrier> image_barriers;
    std::vector<vk::MemoryBarrier>      memory_barriers;

    std::ranges::for_each(
        info.mImageMemoryBarriers,
        [&image_barriers]( const ImageMemoryBarrierInfo &info )
        {
            vk::ImageMemoryBarrier vk_info{};
            vk_info.image         = *dynamic_cast<VulkanImage *>( info.mImage );
            vk_info.oldLayout     = Convert( info.mSrcLayout );
            vk_info.newLayout     = Convert( info.mDstLayout );
            vk_info.srcAccessMask = Convert( info.mSrcMemoryAccess );
            vk_info.dstAccessMask = Convert( info.mDstMemoryAccess );
            vk_info.subresourceRange = vk::ImageSubresourceRange{
                vk::ImageAspectFlagBits::eColor, info.mSubresRange.baseMipLevel,
                info.mSubresRange.levelCount, info.mSubresRange.baseArrayLayer,
                info.mSubresRange.layerCount };

            image_barriers.push_back( vk_info );
        } );
    std::ranges::for_each( info.mMemoryBarriers,
                           [&memory_barriers]( const MemoryBarrierInfo &info )
                           {
                               vk::MemoryBarrier vk_info{};
                               vk_info.srcAccessMask =
                                   Convert( info.mSrcMemoryAccess );
                               vk_info.dstAccessMask =
                                   Convert( info.mDstMemoryAccess );
                               memory_barriers.push_back( vk_info );
                           } );

    m_vkCmdBuffer.pipelineBarrier( Convert( info.mSrcStage ),
                                   Convert( info.mDstStage ), {},
                                   memory_barriers, {}, image_barriers );
}

void VulkanCommandBuffer::BindDescriptorSets(
    const DescriptorSetBindInfo &bind_info )
{
    vk::PipelineLayout vk_pipeline_layout =
        *dynamic_cast<VulkanPipelineLayout *>( bind_info.mPipelineLayout );

    // SmallVectorArena<vk::DescriptorSet, 8> arena;
    std::vector<vk::DescriptorSet> descriptor_sets{};

    std::ranges::for_each( bind_info.mDescriptorSets,
                           [&descriptor_sets]( IDescriptorSet *set )
                           {
                               vk::DescriptorSet vk_set =
                                   *dynamic_cast<VulkanDescriptorSet *>( set );
                               descriptor_sets.push_back( vk_set );
                           } );

    m_vkCmdBuffer.bindDescriptorSets(
        Convert( bind_info.mPipelineBindPoint ), vk_pipeline_layout,
        bind_info.mDescriptorSetsOffset, descriptor_sets, {} );
}

void VulkanCommandBuffer::BeginRecord()
{
    vk::CommandBufferBeginInfo begin_info{};
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    auto result      = m_vkCmdBuffer.begin( begin_info );
    if ( result != vk::Result::eSuccess )
        debug::DebugLogger::ErrorFmt( "Failed to begin cmd buffer recording:%s",
                                      vk::to_string( result ).c_str() );
}

void VulkanCommandBuffer::EndRecord()
{
    auto result = m_vkCmdBuffer.end();
    if ( result != vk::Result::eSuccess )
        debug::DebugLogger::ErrorFmt( "Failed to begin cmd buffer recording:%s",
                                      vk::to_string( result ).c_str() );
}

// TODO: Pass Params!!!!
void VulkanCommandBuffer::BeginRenderPass( const RenderPassBeginInfo &params )
{
    vk::RenderPassBeginInfo begin_info{};
    begin_info.renderPass =
        *dynamic_cast<VulkanRenderPass *>( params.m_pRenderPass );
    begin_info.framebuffer =
        dynamic_cast<VulkanFrameBuffer *>( params.m_pFrameBuffer )->GetImpl();
    // SmallVectorArena<vk::ClearValue, 8> arena;
    std::vector<vk::ClearValue> clear_values{};
    clear_values.reserve( params.m_aClearValues.Size() );

    // Convert clear params...
    std::ranges::transform(
        params.m_aClearValues, std::back_inserter( clear_values ),
        []( const ClearValue &cv )
        {
            vk::ClearValue cv_res;
            if ( cv.type == ClearValueType::Color )
            {
                cv_res.color = vk::ClearColorValue( std::array<float, 4>{
                    ( cv.color.r / 255.0f ), ( cv.color.g / 255.0f ),
                    ( cv.color.b / 255.0f ), ( cv.color.a / 255.0f ) } );
            }
            else
            {
                cv_res.depthStencil = vk::ClearDepthStencilValue(
                    cv.depthStencil.depth, cv.depthStencil.stencil );
            }

            return cv_res;
        } );

    auto frame_buffer_info     = params.m_pFrameBuffer->GetInfo();
    begin_info.pClearValues    = clear_values.data();
    begin_info.clearValueCount = static_cast<uint32_t>( clear_values.size() );
    begin_info.renderArea.extent.width  = frame_buffer_info.width;
    begin_info.renderArea.extent.height = frame_buffer_info.height;
    m_vkCmdBuffer.beginRenderPass( begin_info, vk::SubpassContents::eInline );
}

void VulkanCommandBuffer::EndRenderPass() { m_vkCmdBuffer.endRenderPass(); }

void VulkanCommandBuffer::BindPipeline( IPipeline *pipeline )
{
    auto         vk_pipe      = static_cast<VulkanPipeline *>( pipeline );
    vk::Pipeline vk_pipe_impl = *vk_pipe;
    m_vkCmdBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics,
                                vk_pipe_impl );
}

void VulkanCommandBuffer::BindVertexBuffers(
    uint32_t start_id, const ArrayProxy<VertexBufferBinding> &buffers )
{
    // SmallVectorArena<vk::Buffer, 4>     vertex_buffers_arena;
    // SmallVectorArena<vk::DeviceSize, 4> vertex_buffer_offsets_arena;
    std::vector<vk::Buffer>     vertex_buffers{};
    std::vector<vk::DeviceSize> vertex_buffer_offsets{};
    vertex_buffers.reserve( buffers.Size() );
    vertex_buffer_offsets.reserve( buffers.Size() );

    std::ranges::for_each(
        buffers,
        [&vertex_buffers,
         &vertex_buffer_offsets]( const VertexBufferBinding &binding )
        {
            vertex_buffers.push_back(
                *static_cast<VulkanBuffer *>( binding.mBuffer ) );
            vertex_buffer_offsets.push_back( binding.mOffset );
        } );

    m_vkCmdBuffer.bindVertexBuffers( start_id, vertex_buffers,
                                     vertex_buffer_offsets );
}

void VulkanCommandBuffer::BindIndexBuffer( uint32_t offset, IBuffer *buffer,
                                           IndexType type )
{

    m_vkCmdBuffer.bindIndexBuffer(
        *dynamic_cast<VulkanBuffer *>( buffer ), offset,
        type == IndexType::i16 ? vk::IndexType::eUint16
                               : vk::IndexType::eUint32 );
}

void VulkanCommandBuffer::Draw( uint32_t vertex_count, uint32_t instance_count,
                                uint32_t first_vertex, uint32_t first_instance )
{
    m_vkCmdBuffer.draw( vertex_count, instance_count, first_vertex,
                        first_instance );
}

void VulkanCommandBuffer::DrawIndexed( uint32_t index_count,
                                       uint32_t instance_count,
                                       uint32_t first_index,
                                       uint32_t first_vertex,
                                       uint32_t first_instance )
{
    m_vkCmdBuffer.drawIndexed( index_count, instance_count, first_index,
                               first_vertex, first_instance );
}

void VulkanCommandBuffer::SetViewports( uint32_t                    start_id,
                                        const ArrayProxy<ViewPort> &viewports )
{
    // SmallVectorArena<vk::Viewport, 8> vk_viewports_arena;
    std::vector<vk::Viewport> vk_viewports{};
    std::ranges::transform( viewports, std::back_inserter( vk_viewports ),
                            []( const ViewPort &viewport )
                            {
                                vk::Viewport vk_vp{};
                                vk_vp.x        = viewport.topLeftX;
                                vk_vp.y        = viewport.topLeftY;
                                vk_vp.width    = viewport.width;
                                vk_vp.height   = viewport.height;
                                vk_vp.maxDepth = viewport.maxDepth;
                                vk_vp.minDepth = viewport.minDepth;
                                return vk_vp;
                            } );

    m_vkCmdBuffer.setViewport( start_id, vk_viewports );
}
void VulkanCommandBuffer::SetScissors( uint32_t                   start_id,
                                       const ArrayProxy<Scissor> &scissors )
{
    // SmallVectorArena<vk::Rect2D, 2> vk_scissors_arena;

    std::vector<vk::Rect2D> vk_scissors{};
    std::ranges::transform( scissors, std::back_inserter( vk_scissors ),
                            []( const Scissor &scissor )
                            {
                                vk::Rect2D vk_sc{};
                                vk_sc.offset.x      = scissor.offset_x;
                                vk_sc.offset.y      = scissor.offset_y;
                                vk_sc.extent.width  = scissor.size_x;
                                vk_sc.extent.height = scissor.size_y;
                                return vk_sc;
                            } );

    m_vkCmdBuffer.setScissor( start_id, vk_scissors );
}
void VulkanCommandBuffer::BuildBLAS(
    VulkanBottomLevelAccelerationStructure *blas, IBuffer *scratch_buffer )
{
    /*m_vkCmdBuffer.buildAccelerationStructureNV(
        blas->GetImplInfo(), nullptr, 0, 0, blas->GetImpl(), nullptr,
        *dynamic_cast<VulkanBuffer *>( scratch_buffer ), 0 );*/
}

void VulkanCommandBuffer::BuildBLAS(
    const ArrayProxy<BlasBuildInfo> &build_info )
{
    std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> geometries{};
    std::vector<const vk::AccelerationStructureBuildRangeInfoKHR *>
        ranges_ptrs{};
    for ( auto b_i : build_info )
    {
        vk::AccelerationStructureBuildGeometryInfoKHR build_geometry_info_khr{};
        const auto &geom = b_i.Accel->GetGeometry();
        build_geometry_info_khr.mode =
            vk::BuildAccelerationStructureModeKHR::eBuild;
        build_geometry_info_khr.type =
            vk::AccelerationStructureTypeKHR::eBottomLevel;
        build_geometry_info_khr.geometryCount = geom.size();
        build_geometry_info_khr.pGeometries   = geom.data();

        vk::BufferDeviceAddressInfo address_info{};
        address_info.buffer = *dynamic_cast<VulkanBuffer *>( b_i.TempBuffer );
        build_geometry_info_khr.scratchData.deviceAddress =
            mDevice.getBufferAddress( address_info );
        //  =
        // ->
        build_geometry_info_khr.dstAccelerationStructure = b_i.Accel->GetImpl();
        geometries.push_back( build_geometry_info_khr );
        ranges_ptrs.push_back( b_i.Accel->GetBuildRanges().data() );
    }
    m_vkCmdBuffer.buildAccelerationStructuresKHR( geometries, ranges_ptrs );
}

void VulkanCommandBuffer::BuildTLAS( VulkanTopLevelAccelerationStructure *tlas,
                                     IBuffer *scratch_buffer,
                                     IBuffer *instance_buffer )
{
    auto vk_instance_buffer = dynamic_cast<VulkanBuffer *>( instance_buffer );
    auto vk_scratch_buffer  = dynamic_cast<VulkanBuffer *>( scratch_buffer );

    vk::AccelerationStructureBuildGeometryInfoKHR build_info{};
    vk::AccelerationStructureBuildRangeInfoKHR    build_range_info{};
    build_range_info.primitiveCount = tlas->mMaxInstances;

    vk::AccelerationStructureGeometryKHR tlas_geom{};

    tlas_geom.geometryType = vk::GeometryTypeKHR::eInstances;
    tlas_geom.geometry.instances.sType =
        vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;
    tlas_geom.geometry.instances.data =
        mDevice.getBufferAddress( { *vk_instance_buffer } );

    build_info.mode          = vk::BuildAccelerationStructureModeKHR::eBuild;
    build_info.type          = vk::AccelerationStructureTypeKHR::eTopLevel;
    build_info.geometryCount = 1;
    build_info.pGeometries   = &tlas_geom;
    build_info.dstAccelerationStructure = tlas->GetImpl();
    build_info.scratchData = mDevice.getBufferAddress( { *vk_scratch_buffer } );

    m_vkCmdBuffer.buildAccelerationStructuresKHR( { build_info },
                                                  { &build_range_info } );
}
void VulkanCommandBuffer::BindRayTracingPipeline(
    VulkanRayTracingPipeline *pipeline )
{
    m_vkCmdBuffer.bindPipeline( vk::PipelineBindPoint::eRayTracingKHR,
                                *pipeline );
}
void VulkanCommandBuffer::DispatchRays( const VulkanRayDispatch &dispatch )
{
    vk::StridedDeviceAddressRegionKHR ray_gen_region{};
    vk::StridedDeviceAddressRegionKHR miss_region{};
    vk::StridedDeviceAddressRegionKHR hit_region{};
    vk::StridedDeviceAddressRegionKHR callable_region{};
    if ( dispatch.mRayGenBuffer )
    {
        ray_gen_region.deviceAddress =
            mDevice.getBufferAddress(
                { *dynamic_cast<VulkanBuffer *>( dispatch.mRayGenBuffer ) } ) +
            dispatch.mRayGenOffset;
        ray_gen_region.size = ray_gen_region.stride = dispatch.mRayGenSize;
    }
    if ( dispatch.mMissBuffer )
    {
        miss_region.deviceAddress =
            mDevice.getBufferAddress(
                { *dynamic_cast<VulkanBuffer *>( dispatch.mMissBuffer ) } ) +
            dispatch.mMissOffset;
        miss_region.stride = dispatch.mMissStride;
        miss_region.size   = dispatch.mMissSize;
    }
    if ( dispatch.mHitBuffer )
    {
        hit_region.deviceAddress =
            mDevice.getBufferAddress(
                { *dynamic_cast<VulkanBuffer *>( dispatch.mHitBuffer ) } ) +
            dispatch.mHitOffset;
        hit_region.stride = dispatch.mHitStride;
        hit_region.size   = dispatch.mHitSize;
    }
    if ( dispatch.mCallableBuffer )
    {
        callable_region.deviceAddress =
            mDevice.getBufferAddress( { *dynamic_cast<VulkanBuffer *>(
                dispatch.mCallableBuffer ) } ) +
            dispatch.mCallableOffset;
        callable_region.stride = dispatch.mCallableStride;
        callable_region.size   = dispatch.mCallableSize;
    }
    m_vkCmdBuffer.traceRaysKHR( ray_gen_region, miss_region, hit_region,
                                callable_region, dispatch.mX, dispatch.mY,
                                dispatch.mZ );
}
void VulkanCommandBuffer::DispatchCompute(
    const VulkanComputeDispatch &dispatch )
{
    m_vkCmdBuffer.dispatch( dispatch.mX, dispatch.mY, dispatch.mZ );
}
void VulkanCommandBuffer::BindComputePipeline( VulkanComputePipeline *pipeline )
{
    m_vkCmdBuffer.bindPipeline( vk::PipelineBindPoint::eCompute,
                                pipeline->GetImpl() );
}
void VulkanCommandBuffer::CopyImageToImage(
    const ImageToImageCopyInfo &copy_info )
{

    vk::Image       src_img    = *dynamic_cast<VulkanImage *>( copy_info.mSrc );
    vk::Image       dest_img   = *dynamic_cast<VulkanImage *>( copy_info.mDst );
    vk::ImageLayout src_layout = Convert( copy_info.mSrcLayout );
    vk::ImageLayout dest_layout = Convert( copy_info.mDstLayout );

    // SmallVectorArena<vk::BufferImageCopy, 8> arena;
    std::vector<vk::ImageCopy> image_copies{};

    std::ranges::for_each(
        copy_info.mRegions,
        [&image_copies]( const ImageToImageCopySubInfo &info )
        {
            vk::ImageCopy vk_info{};
            vk_info.extent = vk::Extent3D{
                info.mSrc.mExtentW, info.mSrc.mExtentH, info.mSrc.mExtentD };

            vk_info.srcOffset = vk::Offset3D{
                info.mSrc.mOffsetX, info.mSrc.mOffsetY, info.mSrc.mOffsetZ };
            vk_info.srcSubresource = vk::ImageSubresourceLayers{
                vk::ImageAspectFlagBits::eColor,
                info.mSrc.mSubresource.mipLevel,
                info.mSrc.mSubresource.baseArrayLayer,
                info.mSrc.mSubresource.layerCount };

            vk_info.dstOffset = vk::Offset3D{
                info.mDest.mOffsetX, info.mDest.mOffsetY, info.mDest.mOffsetZ };
            vk_info.dstSubresource = vk::ImageSubresourceLayers{
                vk::ImageAspectFlagBits::eColor,
                info.mDest.mSubresource.mipLevel,
                info.mDest.mSubresource.baseArrayLayer,
                info.mDest.mSubresource.layerCount };

            image_copies.push_back( vk_info );
        } );

    m_vkCmdBuffer.copyImage( src_img, src_layout, dest_img, dest_layout,
                             image_copies );
}
void VulkanCommandBuffer::CopyImageToBuffer(
    const ImageToBufferCopyInfo &copy_info )
{

    vk::Image       img    = *static_cast<VulkanImage *>( copy_info.mImage );
    vk::Buffer      buffer = *static_cast<VulkanBuffer *>( copy_info.mBuffer );
    vk::ImageLayout layout = Convert( copy_info.mImageLayout );

    // SmallVectorArena<vk::BufferImageCopy, 8> arena;
    std::vector<vk::BufferImageCopy> image_copies{};

    std::ranges::for_each(
        copy_info.mRegions,
        [&image_copies]( const ImageToBufferCopySubInfo &info )
        {
            vk::BufferImageCopy vk_info{};
            vk_info.imageExtent = vk::Extent3D{
                info.mFrom.mExtentW, info.mFrom.mExtentH, info.mFrom.mExtentD };
            vk_info.imageOffset = vk::Offset3D{
                info.mFrom.mOffsetX, info.mFrom.mOffsetY, info.mFrom.mOffsetZ };
            vk_info.imageSubresource = vk::ImageSubresourceLayers{
                vk::ImageAspectFlagBits::eColor,
                info.mFrom.mSubresource.mipLevel,
                info.mFrom.mSubresource.baseArrayLayer,
                info.mFrom.mSubresource.layerCount };
            vk_info.bufferRowLength   = info.mTo.mRowLength;
            vk_info.bufferImageHeight = info.mTo.mRowCount;
            vk_info.bufferOffset      = info.mTo.mOffset;

            image_copies.push_back( vk_info );
        } );

    m_vkCmdBuffer.copyImageToBuffer( img, layout, buffer, image_copies );
}
