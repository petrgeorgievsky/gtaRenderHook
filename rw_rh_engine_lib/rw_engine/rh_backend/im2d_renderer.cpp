//
// Created by peter on 25.06.2020.
//

#include "im2d_renderer.h"
#include <ipc/MemoryReader.h>
#include <render_client/im2d_state_recorder.h>
#include <render_driver/gpu_resources/raster_pool.h>

#include <rendering_loop/DescriptorGenerator.h>
#include <rendering_loop/ray_tracing/CameraDescription.h>

#include <rw_engine/rh_backend/im2d_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/types/blend_op.h>
#include <Engine/Common/types/comparison_func.h>
#include <Engine/Common/types/sampler_filter.h>

#include <span>

namespace rh::rw::engine
{
using namespace rh::engine;

constexpr auto VERTEX_COUNT_LIMIT     = 100000;
constexpr auto INDEX_COUNT_LIMIT      = 100000;
constexpr auto TEXTURE_DESC_POOL_SIZE = 1000;

constexpr auto Im2DCallbackId = 421;

Im2DRenderer::Im2DRenderer( rh::engine::IDeviceState &device,
                            RasterPoolType &          raster_pool,
                            CameraDescription *       cdsec,
                            rh::engine::IRenderPass * render_pass )
    : Device( device ), RasterPool( raster_pool ), mCamDesc( cdsec )
{
    mRenderPass = render_pass;
    // screen stuff
    DescriptorGenerator d_gen{ Device };

    d_gen
        .AddDescriptor( 0, 0, 0, DescriptorType::ROBuffer, 1,
                        ShaderStage::Pixel | ShaderStage::Vertex )
        .AddDescriptor( 1, 0, 0, DescriptorType::Sampler, 1,
                        ShaderStage::Pixel )
        .AddDescriptor( 1, 1, 0, DescriptorType::ROTexture, 1,
                        ShaderStage::Pixel );

    mGlobalSetLayout = d_gen.FinalizeDescriptorSet( 0, 1 );
    mTextureDescSetLayout =
        d_gen.FinalizeDescriptorSet( 1, TEXTURE_DESC_POOL_SIZE );
    mDescSetAllocator = d_gen.FinalizeAllocator();

    // create pipeline layouts
    mTexLayout = Device.CreatePipelineLayout(
        { { mGlobalSetLayout, cdsec->GetSetLayout(),
            mTextureDescSetLayout } } );
    mNoTexLayout = Device.CreatePipelineLayout(
        { { mGlobalSetLayout, cdsec->GetSetLayout() } } );

    // create pipelines
    mBaseVertex.desc = { .mShaderPath  = "shaders/d3d11/engine/Im2D.hlsl",
                         .mEntryPoint  = "BaseVS",
                         .mShaderStage = ShaderStage::Vertex };

    mTexPixel.desc   = { .mShaderPath  = "shaders/d3d11/engine/Im2D.hlsl",
                       .mEntryPoint  = "TexPS",
                       .mShaderStage = ShaderStage::Pixel };
    mNoTexPixel.desc = { .mShaderPath  = "shaders/d3d11/engine/Im2D.hlsl",
                         .mEntryPoint  = "NoTexPS",
                         .mShaderStage = ShaderStage::Pixel };

    mDepthMaskPixel.desc = { .mShaderPath  = "shaders/d3d11/engine/Im2D.hlsl",
                             .mEntryPoint  = "DepthMaskPS",
                             .mShaderStage = ShaderStage::Pixel };

    mBaseVertex.shader     = Device.CreateShader( mBaseVertex.desc );
    mTexPixel.shader       = Device.CreateShader( mTexPixel.desc );
    mNoTexPixel.shader     = Device.CreateShader( mNoTexPixel.desc );
    mDepthMaskPixel.shader = Device.CreateShader( mDepthMaskPixel.desc );

    // create buffers
    mVertexBuffer = Device.CreateBuffer(
        { .mSize  = sizeof( RwIm2DVertex ) * VERTEX_COUNT_LIMIT,
          .mUsage = BufferUsage::VertexBuffer } );
    mIndexBuffer =
        Device.CreateBuffer( { .mSize  = sizeof( int16_t ) * INDEX_COUNT_LIMIT,
                               .mUsage = BufferUsage::IndexBuffer } );

    std::vector tex_layout_array =
        std::vector( TEXTURE_DESC_POOL_SIZE, mTextureDescSetLayout );

    mDescriptorSetPool = mDescSetAllocator->AllocateDescriptorSets(
        { .mLayouts = tex_layout_array } );

    SamplerDesc sampler_desc{};
    sampler_desc.mInfo.filtering = SamplerFilter::Linear;
    mTextureSampler              = Device.CreateSampler( sampler_desc );

    for ( auto &i : mDescriptorSetPool )
    {
        std::array              sampler_upd_info = { ImageUpdateInfo{
            ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } };
        DescriptorSetUpdateInfo info{};
        info.mDescriptorType  = DescriptorType::Sampler;
        info.mBinding         = 0;
        info.mSet             = i;
        info.mImageUpdateInfo = sampler_upd_info;
        Device.UpdateDescriptorSets( info );
    }

    std::array<rh::engine::IDescriptorSetLayout *, 1> layout_array_ = {
        mGlobalSetLayout };
    rh::engine::DescriptorSetsAllocateParams alloc_params{};
    alloc_params.mLayouts = layout_array_;
    mBaseDescSet = mDescSetAllocator->AllocateDescriptorSets( alloc_params )[0];

    struct RenderStateBuffer
    {
        float fScreenWidth;
        float fScreenHeight;
        float fRedness;
        float fPadding;
    };
    RenderStateBuffer buff{};
    buff.fScreenWidth  = 640;
    buff.fScreenHeight = 480;

    rh::engine::BufferCreateInfo rs_buff_create_info{};
    rs_buff_create_info.mSize        = sizeof( RenderStateBuffer );
    rs_buff_create_info.mUsage       = rh::engine::BufferUsage::ConstantBuffer;
    rs_buff_create_info.mFlags       = rh::engine::BufferFlags::Immutable;
    rs_buff_create_info.mInitDataPtr = &buff;
    mGlobalsBuffer = Device.CreateBuffer( rs_buff_create_info );

    rh::engine::DescriptorSetUpdateInfo desc_set_upd_info{};
    desc_set_upd_info.mSet              = mBaseDescSet;
    desc_set_upd_info.mDescriptorType   = rh::engine::DescriptorType::ROBuffer;
    std::array update_info              = { rh::engine::BufferUpdateInfo{
        0, sizeof( RenderStateBuffer ), mGlobalsBuffer } };
    desc_set_upd_info.mBufferUpdateInfo = update_info;
    Device.UpdateDescriptorSets( desc_set_upd_info );

    /// pip create

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
    ShaderStageDesc ps_stage_depthmask_desc{
        .mStage      = mDepthMaskPixel.desc.mShaderStage,
        .mShader     = mDepthMaskPixel.shader,
        .mEntryPoint = mDepthMaskPixel.desc.mEntryPoint };

    std::array<VertexBindingDesc, 1> vertex_binding_desc = {
        { { 0, sizeof( RwIm2DVertex ), VertexBindingRate::PerVertex } } };

    std::array<VertexInputElementDesc, 3> vertex_layout_desc{
        { { 0, 0, InputElementType::Vec4fp32, 0, "POSITION" },
          { 0, 1, InputElementType::Vec4fp8, 16, "COLOR" },
          { 0, 2, InputElementType::Vec2fp32, 20, "TEXCOORD" } } };

    BlendState default_blend_state = {
        { { .srcBlend            = BlendOp::SrcAlpha,
            .destBlend           = BlendOp::InvSrcAlpha,
            .blendCombineOp      = BlendCombineOp::Add,
            .srcBlendAlpha       = BlendOp::Zero,
            .destBlendAlpha      = BlendOp::Zero,
            .blendAlphaCombineOp = BlendCombineOp::Add,
            .enableBlending      = true } },
        { 1.0f, 1.0f, 1.0f, 1.0f } };

    mPipelineNoTex = Device.CreateRasterPipeline(
        { .mRenderPass           = render_pass,
          .mLayout               = mNoTexLayout,
          .mShaderStages         = { vs_stage_desc, ps_stage_notex_desc },
          .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
          .mTopology             = Topology::TriangleList,
          .mBlendState           = default_blend_state } );

    mPipelineTex = Device.CreateRasterPipeline(
        { .mRenderPass           = render_pass,
          .mLayout               = mTexLayout,
          .mShaderStages         = { vs_stage_desc, ps_stage_desc },
          .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
          .mTopology             = Topology::TriangleList,
          .mBlendState           = default_blend_state } );

    BlendState depth_mask_blend_state = {
        { { .srcBlend            = BlendOp::One,
            .destBlend           = BlendOp::Zero,
            .blendCombineOp      = BlendCombineOp::Add,
            .srcBlendAlpha       = BlendOp::One,
            .destBlendAlpha      = BlendOp::Zero,
            .blendAlphaCombineOp = BlendCombineOp::Add,
            .enableBlending      = true } },
        { 1.0f, 1.0f, 1.0f, 1.0f } };
    DepthStencilState depth_state{};
    depth_state.enableDepthBuffer   = true;
    depth_state.enableDepthWrite    = true;
    depth_state.depthComparisonFunc = ComparisonFunc::Always;
    mDepthMaskPipeline              = Device.CreateRasterPipeline(
        { .mRenderPass           = render_pass,
          .mLayout               = mTexLayout,
          .mShaderStages         = { vs_stage_desc, ps_stage_depthmask_desc },
          .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
          .mTopology             = Topology::TriangleList,
          .mBlendState           = default_blend_state,
          .mDepthStencilState    = depth_state } );

    RasterPool.AddOnDestructCallback(
        [this]( RasterData &data, uint64_t id ) { mTextureCache.erase( id ); },
        Im2DCallbackId );
}

Im2DRenderer::~Im2DRenderer()
{
    RasterPool.RemoveOnDestructCallback( Im2DCallbackId );
    delete mPipelineTex;
    delete mPipelineNoTex;
    delete mDepthMaskPipeline;
    for ( auto &ptr : mDescriptorSetPool )
        delete ptr;
    delete mBaseDescSet;
    delete mTextureSampler;
    delete mGlobalsBuffer;
    delete mVertexBuffer;
    delete mIndexBuffer;
    delete mTextureDescSetLayout;
    delete mGlobalSetLayout;
    delete mBaseVertex.shader;
    delete mNoTexPixel.shader;
    delete mTexPixel.shader;
    delete mDepthMaskPixel.shader;
    delete mTexLayout;
    delete mNoTexLayout;
    delete mDescSetAllocator;
}

struct PackedIm2DState
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
        } s_val;
        uint64_t i_val;
    };
};

uint64_t Im2DRenderer::Render( const Im2DRenderState &     state,
                               rh::engine::ICommandBuffer *cmd_buffer )
{
    // Update buffers
    if ( state.IndexBuffer.Size() > 0 )
        mIndexBuffer->Update( state.IndexBuffer.Data(),
                              state.IndexBuffer.Size() * sizeof( int16_t ), 0 );

    if ( state.VertexBuffer.Size() <= 0 )
        return 0;

    auto current_display_mode = [this]() {
        uint32_t display_mode;
        Device.GetCurrentDisplayMode( display_mode );
        DisplayModeInfo info{};
        Device.GetDisplayModeInfo( display_mode, info );
        return info;
    }();

    // Transform to screen-space
    // TODO: at the moment this doesn't fit well, maybe move that to a different
    // place
    for ( auto &vtx : std::span( (RwIm2DVertex *)state.VertexBuffer.Data(),
                                 state.VertexBuffer.Size() ) )
    {
        vtx.x = vtx.x * ( 2.0f / float( current_display_mode.width ) ) - 1.0f;
        vtx.y = vtx.y * ( 2.0f / float( current_display_mode.height ) ) - 1.0f;
    }

    mVertexBuffer->Update( state.VertexBuffer.Data(),
                           state.VertexBuffer.Size() * sizeof( RwIm2DVertex ),
                           mVertexBufferOffset );

    // Bind buffers
    cmd_buffer->BindIndexBuffer( 0, mIndexBuffer, IndexType::i16 );
    cmd_buffer->BindVertexBuffers(
        0, { { mVertexBuffer, 0, sizeof( RwIm2DVertex ) } } );

    cmd_buffer->BindDescriptorSets(
        { .mPipelineLayout       = mNoTexLayout,
          .mDescriptorSetsOffset = 0,
          .mDescriptorSets       = { mBaseDescSet, mCamDesc->GetDescSet() } } );

    if ( state.DrawCalls.Size() <= 0 )
        return 0;

    auto vertex_offset = mVertexBufferOffset / sizeof( RwIm2DVertex );
    for ( auto &draw_call : state.DrawCalls )
    {
        // Compute pipeline hash

        PackedIm2DState s{};
        s.s_val.enableBlend = draw_call.mBlendState.mBlendEnable;
        s.s_val.hasTexture =
            draw_call.mRasterId != BackendRasterPlugin::NullRasterId;
        s.s_val.srcBlendState  = draw_call.mBlendState.mColorBlendSrc;
        s.s_val.destBlendState = draw_call.mBlendState.mColorBlendDst;
        s.s_val.zTestEnable    = draw_call.mBlendState.mZTestEnable;
        s.s_val.zWriteEnable   = draw_call.mBlendState.mZWriteEnable;
        if ( draw_call.mRasterId != BackendRasterPlugin::NullRasterId )
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
    }

    mVertexBufferOffset += state.VertexBuffer.Size() * sizeof( RwIm2DVertex );

    return 0;
}

void Im2DRenderer::DrawQuad( rh::engine::IImageView *    texture,
                             rh::engine::ICommandBuffer *cmd_buffer )
{
    auto w = 1.0f;
    auto h = 1.0f;

    std::array<RwIm2DVertex, 6> quad{
        RwIm2DVertex{ -1, -1, 0, 0, 0xFFFFFFFF, 0.0f, 0.0f },
        RwIm2DVertex{ w, h, -1, 0, 0xFFFFFFFF, 1.0f, 1.0f },
        RwIm2DVertex{ -1, h, 0, 0, 0xFFFFFFFF, 0.0f, 1.0f },
        RwIm2DVertex{ -1, -1, 0, 0, 0xFFFFFFFF, 0.0f, 0.0f },
        RwIm2DVertex{ w, -1, 0, 0, 0xFFFFFFFF, 1.0f, 0.0f },
        RwIm2DVertex{ w, h, 0, 0, 0xFFFFFFFF, 1.0f, 1.0f } };

    mVertexBuffer->Update( quad.data(), 6 * sizeof( RwIm2DVertex ),
                           mVertexBufferOffset );
    mVertexBufferOffset += 6 * sizeof( RwIm2DVertex );
    IDescriptorSet *texDescriptorSet;
    auto            raster_id = reinterpret_cast<uint64_t>( texture );
    // auto            cache_entry = mTextureCache.find( raster_id );
    // if ( cache_entry != mTextureCache.end() )
    //   texDescriptorSet = cache_entry->second;
    // else
    {
        std::array<ImageUpdateInfo, 1> img_upd_info = {
            ImageUpdateInfo{ ImageLayout::ShaderReadOnly, texture, nullptr } };

        texDescriptorSet = mTextureCache[raster_id] =
            mDescriptorSetPool[mDescriptorSetPoolId];
        DescriptorSetUpdateInfo info{};
        info.mBinding         = 1;
        info.mDescriptorType  = DescriptorType::ROTexture;
        info.mSet             = texDescriptorSet;
        info.mImageUpdateInfo = img_upd_info;
        Device.UpdateDescriptorSets( info );
        mDescriptorSetPoolId =
            ( mDescriptorSetPoolId + 1 ) % mDescriptorSetPool.size();
    }

    std::array<IDescriptorSet *, 3> tex_desc_sets = {
        mBaseDescSet, mCamDesc->GetDescSet(), texDescriptorSet };
    DescriptorSetBindInfo tex_desc_set_bind{};
    tex_desc_set_bind.mPipelineLayout       = mTexLayout;
    tex_desc_set_bind.mDescriptorSetsOffset = 0;
    tex_desc_set_bind.mDescriptorSets       = tex_desc_sets;
    cmd_buffer->BindDescriptorSets( tex_desc_set_bind );

    std::array<VertexBufferBinding, 1> vbuffers = {
        { mVertexBuffer, 0, sizeof( RwIm2DVertex ) } };
    cmd_buffer->BindVertexBuffers( 0, vbuffers );

    cmd_buffer->BindPipeline( mPipelineTex );
    cmd_buffer->Draw( 6, 1, 0, 0 );
}

rh::engine::IDescriptorSet *Im2DRenderer::GetRasterDescSet( uint64_t id )
{
    // auto cache_entry = mTextureCache.find( id );
    // if ( cache_entry != mTextureCache.end() )
    //    return cache_entry->second;
    // else
    {
        auto set = mTextureCache[id] = mDescriptorSetPool[mDescriptorSetPoolId];
        auto img_view                = RasterPool.GetResource( id ).mImageView;
        Device.UpdateDescriptorSets(
            { .mSet             = set,
              .mBinding         = 1,
              .mDescriptorType  = DescriptorType::ROTexture,
              .mImageUpdateInfo = {
                  { ImageLayout::ShaderReadOnly, img_view, nullptr } } } );
        mDescriptorSetPoolId =
            ( mDescriptorSetPoolId + 1 ) % mDescriptorSetPool.size();
        return set;
    }
}

void Im2DRenderer::DrawQuad( uint64_t                    texture_id,
                             rh::engine::ICommandBuffer *cmd_buffer )
{
    auto w = 1.0f;
    auto h = 1.0f;

    std::array<RwIm2DVertex, 6> quad{
        RwIm2DVertex{ -1, -1, 0, 0, 0xFFFFFFFF, 0.0f, 0.0f },
        RwIm2DVertex{ w, h, 0, 0, 0xFFFFFFFF, 1.0f, 1.0f },
        RwIm2DVertex{ -1, h, 0, 0, 0xFFFFFFFF, 0.0f, 1.0f },
        RwIm2DVertex{ -1, -1, 0, 0, 0xFFFFFFFF, 0.0f, 0.0f },
        RwIm2DVertex{ w, -1, 0, 0, 0xFFFFFFFF, 1.0f, 0.0f },
        RwIm2DVertex{ w, h, 0, 0, 0xFFFFFFFF, 1.0f, 1.0f } };

    mVertexBuffer->Update( quad.data(), quad.size() * sizeof( RwIm2DVertex ),
                           0 );

    cmd_buffer->BindPipeline( mPipelineTex );

    cmd_buffer->BindDescriptorSets(
        { .mPipelineLayout = mTexLayout,
          .mDescriptorSets = { mBaseDescSet, mCamDesc->GetDescSet(),
                               GetRasterDescSet( texture_id ) } } );

    cmd_buffer->BindVertexBuffers(
        0, { { mVertexBuffer, 0, sizeof( RwIm2DVertex ) } } );

    cmd_buffer->Draw( 6, 1, 0, 0 );
}
void Im2DRenderer::Reset() { mVertexBufferOffset = 0; }

AttachmentBlendState UnpackBlendState( const PackedIm2DState &s )
{
    return { .srcBlend       = static_cast<BlendOp>( s.s_val.srcBlendState ),
             .destBlend      = static_cast<BlendOp>( s.s_val.destBlendState ),
             .blendCombineOp = BlendCombineOp::Add,
             .srcBlendAlpha  = static_cast<BlendOp>( s.s_val.srcBlendState ),
             .destBlendAlpha = static_cast<BlendOp>( s.s_val.destBlendState ),
             .blendAlphaCombineOp = BlendCombineOp::Add,
             .enableBlending      = s.s_val.enableBlend };
}

rh::engine::IPipeline *Im2DRenderer::GetCachedPipeline( uint64_t hash )
{
    if ( mIm2DPipelines.contains( hash ) )
        return mIm2DPipelines.at( hash );

    PackedIm2DState s{};
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
        { { 0, sizeof( RwIm2DVertex ), VertexBindingRate::PerVertex } } };

    std::array<VertexInputElementDesc, 3> vertex_layout_desc{
        { { 0, 0, InputElementType::Vec4fp32, 0, "POSITION" },
          { 0, 1, InputElementType::Vec4fp8, 16, "COLOR" },
          { 0, 2, InputElementType::Vec2fp32, 20, "TEXCOORD" } } };

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

    mIm2DPipelines[hash] = Device.CreateRasterPipeline(
        { .mRenderPass   = mRenderPass,
          .mLayout       = s.s_val.hasTexture ? mTexLayout : mNoTexLayout,
          .mShaderStages = { vs_stage_desc, s.s_val.hasTexture
                                                ? ps_stage_desc
                                                : ps_stage_notex_desc },
          .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
          .mTopology             = Topology::TriangleList,
          .mBlendState           = blend_state,
          .mDepthStencilState    = depth_state } );

    return mIm2DPipelines[hash];
}

void Im2DRenderer::DrawDepthMask( rh::engine::IImageView *    texture,
                                  rh::engine::ICommandBuffer *cmd_buffer )
{
    auto w = 1.0f;
    auto h = 1.0f;

    std::array<RwIm2DVertex, 6> quad{
        RwIm2DVertex{ -1, -1, 0, 0, 0xFFFFFFFF, 0.0f, 0.0f },
        RwIm2DVertex{ w, h, 0, 0, 0xFFFFFFFF, 1.0f, 1.0f },
        RwIm2DVertex{ -1, h, 0, 0, 0xFFFFFFFF, 0.0f, 1.0f },
        RwIm2DVertex{ -1, -1, 0, 0, 0xFFFFFFFF, 0.0f, 0.0f },
        RwIm2DVertex{ w, -1, 0, 0, 0xFFFFFFFF, 1.0f, 0.0f },
        RwIm2DVertex{ w, h, 0, 0, 0xFFFFFFFF, 1.0f, 1.0f } };

    mVertexBuffer->Update( quad.data(), 6 * sizeof( RwIm2DVertex ),
                           mVertexBufferOffset );
    mVertexBufferOffset += 6 * sizeof( RwIm2DVertex );
    IDescriptorSet *texDescriptorSet;
    auto            raster_id = reinterpret_cast<uint64_t>( texture );
    // auto            cache_entry = mTextureCache.find( raster_id );
    // if ( cache_entry != mTextureCache.end() )
    //   texDescriptorSet = cache_entry->second;
    // else
    {
        std::array<ImageUpdateInfo, 1> img_upd_info = {
            ImageUpdateInfo{ ImageLayout::ShaderReadOnly, texture, nullptr } };

        texDescriptorSet = mTextureCache[raster_id] =
            mDescriptorSetPool[mDescriptorSetPoolId];
        DescriptorSetUpdateInfo info{};
        info.mBinding         = 1;
        info.mDescriptorType  = DescriptorType::ROTexture;
        info.mSet             = texDescriptorSet;
        info.mImageUpdateInfo = img_upd_info;
        Device.UpdateDescriptorSets( info );
        mDescriptorSetPoolId =
            ( mDescriptorSetPoolId + 1 ) % mDescriptorSetPool.size();
    }

    std::array<IDescriptorSet *, 3> tex_desc_sets = {
        mBaseDescSet, mCamDesc->GetDescSet(), texDescriptorSet };
    DescriptorSetBindInfo tex_desc_set_bind{};
    tex_desc_set_bind.mPipelineLayout       = mTexLayout;
    tex_desc_set_bind.mDescriptorSetsOffset = 0;
    tex_desc_set_bind.mDescriptorSets       = tex_desc_sets;
    cmd_buffer->BindDescriptorSets( tex_desc_set_bind );

    std::array<VertexBufferBinding, 1> vbuffers = {
        { mVertexBuffer, 0, sizeof( RwIm2DVertex ) } };
    cmd_buffer->BindVertexBuffers( 0, vbuffers );

    cmd_buffer->BindPipeline( mDepthMaskPipeline );
    cmd_buffer->Draw( 6, 1, 0, 0 );
}

} // namespace rh::rw::engine