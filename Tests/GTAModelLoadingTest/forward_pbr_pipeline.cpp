#include "forward_pbr_pipeline.h"
#include <Engine/Common/IDescriptorSet.h>
#include <Engine/Common/IDescriptorSetAllocator.h>
#include <Engine/Common/IDescriptorSetLayout.h>
#include <Engine/Common/IDeviceState.h>
#include <common_headers.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_texture/rw_texture.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

using namespace rh::engine;
using namespace rh::rw::engine;
ForwardPBRPipeline::ForwardPBRPipeline() {}

ForwardPBRPipeline::~ForwardPBRPipeline() {}

void ForwardPBRPipeline::Init( rh::engine::IRenderPass *render_pass )
{
    if ( mInitialized )
        return;
    std::array<DescriptorBinding, 1> camera_desc_set_bindings = { {
        { 0,                        //  mBindingId;
          DescriptorType::ROBuffer, //  mDescriptorType;
          1,                        //  mCount;
          ShaderStage::Vertex, 0 }  //  mShaderStages;
    } };
    mCameraSetLayout =
        DeviceGlobals::RenderHookDevice->CreateDescriptorSetLayout(
            { camera_desc_set_bindings } );

    std::array desc_set_bindings = { DescriptorBinding{
        0, DescriptorType::ROBuffer, 1, ShaderStage::Vertex, 1 } };
    mModelSetLayout =
        DeviceGlobals::RenderHookDevice->CreateDescriptorSetLayout(
            { desc_set_bindings } );

    // create pipeline layouts
    std::array layout_array = { mCameraSetLayout, mModelSetLayout };

    PipelineLayoutCreateParams pipe_layout_ci{};
    pipe_layout_ci.mSetLayouts = layout_array;
    mPipeLayout =
        DeviceGlobals::RenderHookDevice->CreatePipelineLayout( pipe_layout_ci );

    // create pipelines
    mBaseVertex.desc = { .mShaderPath  = "shaders/d3d11/engine/Basic3D.hlsl",
                         .mEntryPoint  = "BaseVS",
                         .mShaderStage = ShaderStage::Vertex };

    mBasePixel.desc = { .mShaderPath  = "shaders/d3d11/engine/Basic3D.hlsl",
                        .mEntryPoint  = "TexPS",
                        .mShaderStage = ShaderStage::Pixel };

    mBaseVertex.shader =
        DeviceGlobals::RenderHookDevice->CreateShader( mBaseVertex.desc );
    mBasePixel.shader =
        DeviceGlobals::RenderHookDevice->CreateShader( mBasePixel.desc );
    // create buffers

    /// descriptorset pools
    DescriptorSetAllocatorCreateParams dsc_all_cp{};
    std::array<DescriptorPoolSize, 3>  dsc_pool_sizes = {
        DescriptorPoolSize{ DescriptorType::ROTexture, 32 },
        DescriptorPoolSize{ DescriptorType::Sampler, 32 },
        DescriptorPoolSize{ DescriptorType::ROBuffer, 33 } };

    dsc_all_cp.mMaxSets         = 64;
    dsc_all_cp.mDescriptorPools = dsc_pool_sizes;
    mDescSetAllocator =
        DeviceGlobals::RenderHookDevice->CreateDescriptorSetAllocator(
            dsc_all_cp );

    std::vector tex_layout_array = std::vector( 32, mModelSetLayout );

    DescriptorSetsAllocateParams all_params{};
    all_params.mLayouts = tex_layout_array;
    mModelDescSet = mDescSetAllocator->AllocateDescriptorSets( all_params );

    // SamplerDesc sampler_desc{};
    // static auto mTextureSampler =
    //    DeviceGlobals::RenderHookDevice->CreateSampler( sampler_desc );

    /* for ( auto &i : mDescriptorSetPool )
     {
         std::array              sampler_upd_info = { ImageUpdateInfo{
             ImageLayout::ShaderReadOnly, nullptr, mTextureSampler } };
         DescriptorSetUpdateInfo info{};
         info.mDescriptorType  = DescriptorType::Sampler;
         info.mBinding         = 0;
         info.mSet             = i;
         info.mImageUpdateInfo = sampler_upd_info;
         DeviceGlobals::RenderHookDevice->UpdateDescriptorSets( info );
     }*/

    std::array<rh::engine::IDescriptorSetLayout *, 1> layout_array_ = {
        mCameraSetLayout };
    rh::engine::DescriptorSetsAllocateParams alloc_params{};
    alloc_params.mLayouts = layout_array_;
    mCameraDescSet =
        mDescSetAllocator->AllocateDescriptorSets( alloc_params )[0];
    struct MtxBuffer
    {
        DirectX::XMFLOAT4X4 mMatrix;
    };
    MtxBuffer buff{};

    BufferCreateInfo rs_buff_create_info{};
    rs_buff_create_info.mSize        = sizeof( MtxBuffer );
    rs_buff_create_info.mUsage       = BufferUsage::ConstantBuffer;
    rs_buff_create_info.mFlags       = BufferFlags::Dynamic;
    rs_buff_create_info.mInitDataPtr = &buff;
    static auto mCameraBuffer =
        DeviceGlobals::RenderHookDevice->CreateBuffer( rs_buff_create_info );

    DescriptorSetUpdateInfo desc_set_upd_info{};
    desc_set_upd_info.mSet            = mCameraDescSet;
    desc_set_upd_info.mDescriptorType = DescriptorType::ROBuffer;
    std::array update_info            = {
        BufferUpdateInfo{ 0, sizeof( MtxBuffer ), mCameraBuffer } };
    desc_set_upd_info.mBufferUpdateInfo = update_info;

    DeviceGlobals::RenderHookDevice->UpdateDescriptorSets( desc_set_upd_info );

    DescriptorSetUpdateInfo mesh_desc_set_upd_info{};
    mesh_desc_set_upd_info.mSet              = mModelDescSet[0];
    mesh_desc_set_upd_info.mDescriptorType   = DescriptorType::ROBuffer;
    mesh_desc_set_upd_info.mBufferUpdateInfo = update_info;

    DeviceGlobals::RenderHookDevice->UpdateDescriptorSets(
        mesh_desc_set_upd_info );

    /// pip create

    ShaderStageDesc vs_stage_desc{ .mStage  = mBaseVertex.desc.mShaderStage,
                                   .mShader = mBaseVertex.shader,
                                   .mEntryPoint =
                                       mBaseVertex.desc.mEntryPoint };

    ShaderStageDesc ps_stage_desc{ .mStage      = mBasePixel.desc.mShaderStage,
                                   .mShader     = mBasePixel.shader,
                                   .mEntryPoint = mBasePixel.desc.mEntryPoint };

    std::array vertex_binding_desc = {
        VertexBindingDesc{ 0, sizeof( VertexDescPosColorUVNormals ),
                           VertexBindingRate::PerVertex } };

    std::array vertex_layout_desc = {
        VertexInputElementDesc{ 0, 0, InputElementType::Vec4fp32, 0, "POSITION",
                                0 },
        VertexInputElementDesc{ 0, 1, InputElementType::Vec4fp8, 16, "COLOR",
                                0 },
        VertexInputElementDesc{ 0, 2, InputElementType::Vec2fp32, 20,
                                "TEXCOORD", 0 },
        VertexInputElementDesc{ 0, 3, InputElementType::Vec3fp32, 28, "NORMAL",
                                0 } };

    RasterPipelineCreateParams pipe_create_params{
        .mRenderPass           = render_pass,
        .mLayout               = mPipeLayout,
        .mShaderStages         = { vs_stage_desc, ps_stage_desc },
        .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
        .mTopology             = Topology::TriangleList };

    mPipelineImpl = DeviceGlobals::RenderHookDevice->CreateRasterPipeline(
        pipe_create_params );
    mInitialized = true;
}

void ForwardPBRPipeline::Draw( rh::engine::ICommandBuffer *           cmd_buff,
                               const rh::rw::engine::BackendMeshData &mesh )
{

    std::array<VertexBufferBinding, 1> vbuffers = {
        { mesh.mVertexBuffer->Get(), 0,
          sizeof( VertexDescPosColorUVNormals ) } };
    cmd_buff->BindIndexBuffer( 0, mesh.mIndexBuffer->Get(), IndexType::i16 );
    cmd_buff->BindVertexBuffers( 0, vbuffers );

    std::array            desc_sets = { mCameraDescSet, mModelDescSet[0] };
    DescriptorSetBindInfo desc_set_bind{};
    desc_set_bind.mPipelineLayout       = mPipeLayout;
    desc_set_bind.mDescriptorSetsOffset = 0;
    desc_set_bind.mDescriptorSets       = desc_sets;
    cmd_buff->BindDescriptorSets( desc_set_bind );

    cmd_buff->BindPipeline( mPipelineImpl );

    cmd_buff->DrawIndexed( mesh.mIndexCount, 1, 0, 0, 0 );
}
