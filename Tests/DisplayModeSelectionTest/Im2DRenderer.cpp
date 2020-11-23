#include "Im2DRenderer.h"
#include <Engine/Common/ScopedPtr.h>

using namespace rh::engine;

Im2DRenderer::Im2DRenderer( const Im2DRendererInitParams &params )
    : mDeviceState( params.mDeviceState )
{
    CreatePipelineLayout( params );
    CreatePipeline( params );
    CreateBuffers( params );
    mCmdBufferCount = params.mCmdBufferCount;

    DescriptorSetAllocatorCreateParams dsc_all_cp{};
    std::array                         dsc_pool_sizes = {
        DescriptorPoolSize{ DescriptorType::ROTexture, 32 },
        DescriptorPoolSize{ DescriptorType::Sampler, 32 } };

    dsc_all_cp.mMaxSets         = 40;
    dsc_all_cp.mDescriptorPools = dsc_pool_sizes;
    mDescriptorSetAlloc =
        mDeviceState->CreateDescriptorSetAllocator( dsc_all_cp );

    std::vector layout_array =
        std::vector( mCmdBufferCount, mTextureDescriptorSetLayout );

    DescriptorSetsAllocateParams all_params{};
    all_params.mLayouts = layout_array;
    mTextureDescriptorSets =
        mDescriptorSetAlloc->AllocateDescriptorSets( all_params );

    SamplerDesc sampler_desc{};
    mTextureSampler = mDeviceState->CreateSampler( sampler_desc );

    for ( size_t i = 0; i < mTextureDescriptorSets.size(); i++ )
    {
        std::array              sampler_upd_info = { ImageUpdateInfo{
            ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } };
        DescriptorSetUpdateInfo info{};
        info.mDescriptorType  = DescriptorType::Sampler;
        info.mBinding         = 0;
        info.mSet             = mTextureDescriptorSets[i];
        info.mImageUpdateInfo = sampler_upd_info;
        mDeviceState->UpdateDescriptorSets( info );
    }
}

Im2DRenderer::~Im2DRenderer()
{
    delete m2DPipeline;
    delete m2DPipelineLayout;
    delete mVertexBuffer;
}

void Im2DRenderer::RecordDrawCall( const ArrayProxy<VertexType> &vertex_list,
                                   const ArrayProxy<IndexType> & index_list )
{
}

void Im2DRenderer::RecordDrawCall( const ArrayProxy<VertexType> &vertex_list )
{
    auto vertex_count = static_cast<uint32_t>( vertex_list.Size() );
    mVertexBuffer->Update( vertex_list.Data(),
                           vertex_count * sizeof( VertexType ) );
    mDrawCalls.push_back( { 0, vertex_count, 0, 0 } );
}

void Im2DRenderer::SetImageView( rh::engine::IImageView *img_view )
{
    std::array img_upd_info = {
        ImageUpdateInfo{ ImageLayout::ShaderReadOnly, img_view, nullptr } };
    DescriptorSetUpdateInfo info{};
    info.mBinding         = 1;
    info.mDescriptorType  = DescriptorType::ROTexture;
    info.mSet             = mTextureDescriptorSets[mFrameCounter];
    info.mImageUpdateInfo = img_upd_info;
    mDeviceState->UpdateDescriptorSets( info );
}

void Im2DRenderer::DrawBatch( ICommandBuffer *cmdbuffer )
{
    std::array<VertexBufferBinding, 1> vbuffers = {
        { mVertexBuffer, 0, sizeof( VertexType ) } };
    std::array<IDescriptorSet *, 1> desc_sets = {
        mTextureDescriptorSets[mFrameCounter] };

    DescriptorSetBindInfo desc_set_bind{};
    desc_set_bind.mPipelineLayout       = m2DPipelineLayout;
    desc_set_bind.mDescriptorSetsOffset = 1;
    desc_set_bind.mDescriptorSets       = desc_sets;

    cmdbuffer->BindDescriptorSets( desc_set_bind );
    cmdbuffer->BindVertexBuffers( 0, vbuffers );
    cmdbuffer->BindPipeline( m2DPipeline );
    cmdbuffer->Draw( mDrawCalls[0].vertexCount, 1, mDrawCalls[0].vertexBase,
                     0 );
    mDrawCalls.clear();
}

void Im2DRenderer::FrameEnd()
{
    mFrameCounter = ( mFrameCounter + 1 ) % mCmdBufferCount;
}

IPipelineLayout *Im2DRenderer::GetLayout() { return m2DPipelineLayout; }

void Im2DRenderer::CreatePipelineLayout( const Im2DRendererInitParams &params )
{
    std::array desc_set_bindings = {
        DescriptorBinding{
            0,                       //  mBindingId;
            DescriptorType::Sampler, //  mDescriptorType;
            1,                       //  mCount;
            ShaderStage::Pixel       //  mShaderStages;
        },
        DescriptorBinding{
            1,                         //  mBindingId;
            DescriptorType::ROTexture, //  mDescriptorType;
            1,                         //  mCount;
            ShaderStage::Pixel         //  mShaderStages;
        } };
    mTextureDescriptorSetLayout =
        mDeviceState->CreateDescriptorSetLayout( { desc_set_bindings } );
    std::array layout_array = { params.mGlobalsDescriptorSetLayout,
                                mTextureDescriptorSetLayout };
    PipelineLayoutCreateParams pipe_layout_ci{};
    pipe_layout_ci.mSetLayouts = layout_array;
    m2DPipelineLayout = mDeviceState->CreatePipelineLayout( pipe_layout_ci );
}

void Im2DRenderer::CreatePipeline( const Im2DRendererInitParams &params )
{
    ShaderDesc vs_desc{ .mShaderPath  = "shaders/d3d11/engine/Im2D.hlsl",
                        .mEntryPoint  = "BaseVS",
                        .mShaderStage = ShaderStage::Vertex };

    ShaderDesc ps_desc{ .mShaderPath  = "shaders/d3d11/engine/Im2D.hlsl",
                        .mEntryPoint  = "TexPS",
                        .mShaderStage = ShaderStage::Pixel };

    ScopedPointer vs_shader = mDeviceState->CreateShader( vs_desc );
    ScopedPointer ps_shader = mDeviceState->CreateShader( ps_desc );

    ShaderStageDesc vs_stage_desc{ .mStage      = vs_desc.mShaderStage,
                                   .mShader     = vs_shader,
                                   .mEntryPoint = vs_desc.mEntryPoint };

    ShaderStageDesc ps_stage_desc{ .mStage      = ps_desc.mShaderStage,
                                   .mShader     = ps_shader,
                                   .mEntryPoint = ps_desc.mEntryPoint };

    std::array vertex_binding_desc = { VertexBindingDesc{
        0, sizeof( VertexType ), VertexBindingRate::PerVertex } };

    std::array vertex_layout_desc = {
        VertexInputElementDesc{ 0, 0, InputElementType::Vec4fp32, 0, "POSITION",
                                0 },
        VertexInputElementDesc{ 0, 1, InputElementType::Vec2fp32, 16,
                                "TEXCOORD", 0 },
        VertexInputElementDesc{ 0, 2, InputElementType::Vec4fp8, 24, "COLOR",
                                0 } };

    RasterPipelineCreateParams pipe_create_params{
        .mRenderPass           = params.mRenderPass,
        .mLayout               = m2DPipelineLayout,
        .mShaderStages         = { vs_stage_desc, ps_stage_desc },
        .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
        .mTopology             = Topology::TriangleList };

    m2DPipeline = mDeviceState->CreateRasterPipeline( pipe_create_params );
}

void Im2DRenderer::CreateBuffers( const Im2DRendererInitParams &params )
{
    constexpr auto   VERTEX_COUNT_LIMIT = 100000;
    BufferCreateInfo v_buff_create_info{};
    v_buff_create_info.mSize  = sizeof( Im2DVertex ) * VERTEX_COUNT_LIMIT;
    v_buff_create_info.mUsage = BufferUsage::VertexBuffer;
    mVertexBuffer = mDeviceState->CreateBuffer( v_buff_create_info );
}
