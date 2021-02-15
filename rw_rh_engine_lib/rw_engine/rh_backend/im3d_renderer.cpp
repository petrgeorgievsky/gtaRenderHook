//
// Created by peter on 21.11.2020.
//

#include "im3d_renderer.h"
#include "im2d_backend.h"
#include "im3d_backend.h"
#include "raster_backend.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/blend_op.h>
#include <Engine/Common/types/comparison_func.h>
#include <Engine/Common/types/sampler_filter.h>
#include <Engine/Common/types/shader_stage.h>
#include <render_driver/render_driver.h>
#include <rendering_loop/DescriptorGenerator.h>
#include <rendering_loop/ray_tracing/CameraDescription.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{
using namespace rh::engine;

constexpr auto VERTEX_COUNT_LIMIT     = 100000;
constexpr auto INDEX_COUNT_LIMIT      = 100000;
constexpr auto DRAW_CALL_POOL_SIZE    = 1000;
constexpr auto TEXTURE_DESC_POOL_SIZE = 1000;

Im3DRenderer::Im3DRenderer( CameraDescription *cdsec, IRenderPass *render_pass )
{
    mRenderPass                = render_pass;
    mCamDesc                   = cdsec;
    auto &              device = gRenderDriver->GetDeviceState();
    DescriptorGenerator d_gen{};

    d_gen
        .AddDescriptor( 0, 0, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::Vertex )
        .AddDescriptor( 1, 0, 0, DescriptorType::Sampler, 1,
                        ShaderStage::Pixel )
        .AddDescriptor( 1, 1, 0, DescriptorType::ROTexture, 1,
                        ShaderStage::Pixel );

    mObjectSetLayout = d_gen.FinalizeDescriptorSet( 0, DRAW_CALL_POOL_SIZE );
    mTextureDescSetLayout =
        d_gen.FinalizeDescriptorSet( 1, TEXTURE_DESC_POOL_SIZE );
    mDescSetAllocator = d_gen.FinalizeAllocator();

    // create pipeline layouts
    mTexLayout = device.CreatePipelineLayout(
        { { cdsec->GetSetLayout(), mObjectSetLayout,
            mTextureDescSetLayout } } );
    mNoTexLayout = device.CreatePipelineLayout(
        { { cdsec->GetSetLayout(), mObjectSetLayout } } );

    // shaders
    mBaseVertex.desc = { .mShaderPath  = "shaders/d3d11/engine/Im3D.hlsl",
                         .mEntryPoint  = "BaseVS",
                         .mShaderStage = ShaderStage::Vertex };

    mTexPixel.desc   = { .mShaderPath  = "shaders/d3d11/engine/Im3D.hlsl",
                       .mEntryPoint  = "TexPS",
                       .mShaderStage = ShaderStage::Pixel };
    mNoTexPixel.desc = { .mShaderPath  = "shaders/d3d11/engine/Im3D.hlsl",
                         .mEntryPoint  = "NoTexPS",
                         .mShaderStage = ShaderStage::Pixel };

    mBaseVertex.shader = device.CreateShader( mBaseVertex.desc );
    mTexPixel.shader   = device.CreateShader( mTexPixel.desc );
    mNoTexPixel.shader = device.CreateShader( mNoTexPixel.desc );

    // create buffers
    mVertexBuffer = device.CreateBuffer(
        { .mSize  = sizeof( RwIm3DVertex ) * VERTEX_COUNT_LIMIT,
          .mUsage = BufferUsage::VertexBuffer } );
    mIndexBuffer =
        device.CreateBuffer( { .mSize  = sizeof( uint16_t ) * INDEX_COUNT_LIMIT,
                               .mUsage = BufferUsage::IndexBuffer } );

    std::vector tex_layout_array = std::vector(
        TEXTURE_DESC_POOL_SIZE, (IDescriptorSetLayout *)mTextureDescSetLayout );

    mDescriptorSetPool = mDescSetAllocator->AllocateDescriptorSets(
        { .mLayouts = tex_layout_array } );

    SamplerDesc sampler_desc{};
    sampler_desc.mInfo.filtering = SamplerFilter::Linear;
    mTextureSampler              = device.CreateSampler( sampler_desc );

    for ( auto &i : mDescriptorSetPool )
    {
        std::array              sampler_upd_info = { ImageUpdateInfo{
            ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } };
        DescriptorSetUpdateInfo info{};
        info.mDescriptorType  = DescriptorType::Sampler;
        info.mBinding         = 0;
        info.mSet             = i;
        info.mImageUpdateInfo = sampler_upd_info;
        device.UpdateDescriptorSets( info );
    }

    std::vector obj_layout_array = std::vector(
        DRAW_CALL_POOL_SIZE, (IDescriptorSetLayout *)mObjectSetLayout );
    rh::engine::DescriptorSetsAllocateParams alloc_params{};
    alloc_params.mLayouts = obj_layout_array;
    mMatrixDescriptorSetPool =
        mDescSetAllocator->AllocateDescriptorSets( alloc_params );
    mMatrixBuffer = device.CreateBuffer(
        BufferCreateInfo{ sizeof( DirectX::XMFLOAT4X4 ) * DRAW_CALL_POOL_SIZE,
                          BufferUsage::ConstantBuffer, Dynamic, nullptr } );
    uint32_t buff_offset = 0;

    for ( auto &i : mMatrixDescriptorSetPool )
    {
        std::array              buffer_upd_info = { BufferUpdateInfo{
            buff_offset, sizeof( DirectX::XMFLOAT4X4 ), mMatrixBuffer } };
        DescriptorSetUpdateInfo info{};
        info.mDescriptorType   = DescriptorType::ROBuffer;
        info.mBinding          = 0;
        info.mSet              = i;
        info.mBufferUpdateInfo = buffer_upd_info;
        device.UpdateDescriptorSets( info );
        buff_offset += sizeof( DirectX::XMFLOAT4X4 );
    }
}

struct PackedIm3DState
{
    union
    {
        struct PackedState
        {
            bool    hasTexture : 1;
            bool    zWriteEnable : 1;
            bool    zTestEnable : 1;
            bool    enableBlend : 1;
            uint8_t srcBlendState : 4;
            uint8_t destBlendState : 4;
            uint8_t primType : 4;
        } s_val;
        uint64_t i_val;
    };
};

AttachmentBlendState UnpackBlendState( const PackedIm3DState &s )
{
    return { .srcBlend       = static_cast<BlendOp>( s.s_val.srcBlendState ),
             .destBlend      = static_cast<BlendOp>( s.s_val.destBlendState ),
             .blendCombineOp = BlendCombineOp::Add,
             .srcBlendAlpha  = static_cast<BlendOp>( s.s_val.srcBlendState ),
             .destBlendAlpha = static_cast<BlendOp>( s.s_val.destBlendState ),
             .blendAlphaCombineOp = BlendCombineOp::Add,
             .enableBlending      = s.s_val.enableBlend };
}

rh::engine::IPipeline *Im3DRenderer::GetCachedPipeline( uint64_t hash )
{
    if ( mIm3DPipelines.contains( hash ) )
        return mIm3DPipelines.at( hash );

    auto &device = gRenderDriver->GetDeviceState();

    PackedIm3DState s{};
    s.i_val = hash;

    ShaderStageDesc vs_stage_desc{ .mStage  = mBaseVertex.desc.mShaderStage,
                                   .mShader = mBaseVertex.shader,
                                   .mEntryPoint =
                                       mBaseVertex.desc.mEntryPoint };

    ShaderStageDesc ps_stage_desc{ .mStage      = mTexPixel.desc.mShaderStage,
                                   .mShader     = mTexPixel.shader,
                                   .mEntryPoint = mTexPixel.desc.mEntryPoint };
    ShaderStageDesc ps_stage_notex_desc{
        .mStage      = mNoTexPixel.desc.mShaderStage,
        .mShader     = mNoTexPixel.shader,
        .mEntryPoint = mNoTexPixel.desc.mEntryPoint };

    std::array<VertexBindingDesc, 1> vertex_binding_desc = {
        { { 0, sizeof( RwIm3DVertex ), VertexBindingRate::PerVertex } } };

    std::array<VertexInputElementDesc, 4> vertex_layout_desc{
        { { 0, 0, InputElementType::Vec3fp32, 0, "POSITION" },
          { 0, 1, InputElementType::Vec3fp32, 12, "TEXCOORD0" },
          { 0, 2, InputElementType::Vec4fp8, 24, "COLOR" },
          { 0, 3, InputElementType::Vec2fp32, 28, "TEXCOORD1" } } };

    // Dynamic states
    BlendState blend_state = { { UnpackBlendState( s ) },
                               { 1.0f, 1.0f, 1.0f, 1.0f } };

    DepthStencilState depth_state{};
    depth_state.enableDepthBuffer   = s.s_val.zTestEnable;
    depth_state.enableDepthWrite    = s.s_val.zWriteEnable;
    depth_state.depthComparisonFunc = ComparisonFunc::LessEqual;
    // Weird vulkan or just modern api glitch or just old api bug:
    // Basically if depth test is disabled we can't write to depth buffer
    // GTA uses such stuff to mask radar for example
    if ( depth_state.enableDepthWrite && !depth_state.enableDepthBuffer )
    {
        depth_state.enableDepthBuffer   = true;
        depth_state.depthComparisonFunc = ComparisonFunc::Always;
    }

    mIm3DPipelines[hash] = device.CreateRasterPipeline(
        { .mRenderPass   = mRenderPass,
          .mLayout       = s.s_val.hasTexture ? mTexLayout : mNoTexLayout,
          .mShaderStages = { vs_stage_desc, s.s_val.hasTexture
                                                ? ps_stage_desc
                                                : ps_stage_notex_desc },
          .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
          .mTopology             = Topology::TriangleList, // TODO: Allow more
          .mBlendState           = blend_state,
          .mDepthStencilState    = depth_state } );

    return mIm3DPipelines[hash];
}
void     Im3DRenderer::Reset() { mVertexBufferOffset = 0; }
uint64_t Im3DRenderer::Render( void *                      memory,
                               rh::engine::ICommandBuffer *cmd_buffer )
{
    MemoryReader stream( memory );
    stream.Skip( sizeof( uint64_t ) );
    uint64_t index_count = *stream.Read<uint64_t>();

    // Update buffers
    if ( index_count > 0 )
        mIndexBuffer->Update( stream.Read<int16_t>( index_count ),
                              index_count * sizeof( int16_t ), 0 );

    uint64_t vertex_count = *stream.Read<uint64_t>();
    if ( vertex_count <= 0 )
        return stream.Pos();

    mVertexBuffer->Update( stream.Read<RwIm3DVertex>( vertex_count ),
                           vertex_count * sizeof( RwIm3DVertex ),
                           mVertexBufferOffset );

    // Bind buffers
    cmd_buffer->BindIndexBuffer( 0, mIndexBuffer, IndexType::i16 );
    cmd_buffer->BindVertexBuffers(
        0, { { mVertexBuffer, 0, sizeof( RwIm3DVertex ) } } );

    cmd_buffer->BindDescriptorSets(
        { .mPipelineLayout       = mNoTexLayout,
          .mDescriptorSetsOffset = 0,
          .mDescriptorSets       = { mCamDesc->GetDescSet() } } );

    uint64_t draw_call_count = *stream.Read<uint64_t>();
    if ( draw_call_count <= 0 )
        return stream.Pos();

    ArrayProxy<Im3DDrawCall> draw_calls(
        stream.Read<Im3DDrawCall>( draw_call_count ), draw_call_count );

    auto     vertex_offset = mVertexBufferOffset / sizeof( RwIm3DVertex );
    uint32_t dc_id         = 0;
    auto *   matrix_buffers =
        static_cast<DirectX::XMFLOAT4X4 *>( mMatrixBuffer->Lock() );
    for ( auto &draw_call : draw_calls )
    {
        // Compute pipeline hash
        if ( draw_call.mState.mPrimType != 3 )
            continue;
        PackedIm3DState s{};
        s.s_val.enableBlend    = draw_call.mState.mBlendEnable;
        s.s_val.hasTexture     = draw_call.mRasterId != gEmptyTextureId;
        s.s_val.srcBlendState  = draw_call.mState.mColorBlendSrc;
        s.s_val.destBlendState = draw_call.mState.mColorBlendDst;
        s.s_val.zTestEnable    = draw_call.mState.mZTestEnable;
        s.s_val.zWriteEnable   = draw_call.mState.mZWriteEnable;

        std::copy( &draw_call.mWorldTransform.m[0][0],
                   &draw_call.mWorldTransform.m[0][0] + 3 * 4,
                   &matrix_buffers[dc_id].m[0][0] );
        matrix_buffers[dc_id].m[3][0] = 0.0f;
        matrix_buffers[dc_id].m[3][1] = 0.0f;
        matrix_buffers[dc_id].m[3][2] = 0.0f;
        matrix_buffers[dc_id].m[3][3] = 1.0f;

        cmd_buffer->BindDescriptorSets(
            { .mPipelineLayout       = mNoTexLayout,
              .mDescriptorSetsOffset = 1,
              .mDescriptorSets       = { mMatrixDescriptorSetPool[dc_id] } } );

        if ( draw_call.mRasterId != gEmptyTextureId )
        {
            cmd_buffer->BindDescriptorSets(
                { .mPipelineLayout       = mTexLayout,
                  .mDescriptorSetsOffset = 2,
                  .mDescriptorSets       = {
                      GetRasterDescSet( draw_call.mRasterId ) } } );
        }

        cmd_buffer->BindPipeline( GetCachedPipeline( s.i_val ) );
        if ( draw_call.mIndexCount > 0 )
        {
            cmd_buffer->DrawIndexed(
                draw_call.mIndexCount, 1, draw_call.mIndexBufferOffset,
                vertex_offset + draw_call.mVertexBufferOffset, 0 );
        }
        else
        {

            cmd_buffer->Draw( draw_call.mVertexCount, 1,
                              vertex_offset + draw_call.mVertexBufferOffset,
                              0 );
        }
        dc_id++;
    }
    mMatrixBuffer->Unlock();

    mVertexBufferOffset += vertex_count * sizeof( RwIm3DVertex );

    return stream.Pos();
}
rh::engine::IDescriptorSet *Im3DRenderer::GetRasterDescSet( uint64_t id )
{
    auto &resources   = gRenderDriver->GetResources();
    auto &raster_pool = resources.GetRasterPool();
    auto  set         = mDescriptorSetPool[mDescriptorSetPoolId];
    auto  img_view    = raster_pool.GetResource( id ).mImageView;
    gRenderDriver->GetDeviceState().UpdateDescriptorSets(
        { .mSet             = set,
          .mBinding         = 1,
          .mDescriptorType  = DescriptorType::ROTexture,
          .mImageUpdateInfo = {
              { ImageLayout::ShaderReadOnly, img_view, nullptr } } } );
    mDescriptorSetPoolId =
        ( mDescriptorSetPoolId + 1 ) % mDescriptorSetPool.size();
    return set;
}
Im3DRenderer::~Im3DRenderer()
{
    for ( auto &ptr : mDescriptorSetPool )
        delete ptr;
    for ( auto &ptr : mMatrixDescriptorSetPool )
        delete ptr;
}

} // namespace rh::rw::engine