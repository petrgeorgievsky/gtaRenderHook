//
// Created by peter on 04.05.2020.
//

#include "GBufferPass.h"
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IRenderPass.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
namespace rh::rw::engine
{

void GBufferPass::InitializeRenderPass()
{
    using namespace rh::engine;
    RenderPassCreateParams render_pass_desc{};

    /// ALBEDO G-BUFFER
    AttachmentDescription render_pass_color_desc{};
    AttachmentRef         color_attach_ref{};
    render_pass_color_desc.mFormat     = ImageBufferFormat::RGBA16;
    render_pass_color_desc.mDestLayout = ImageLayout::ShaderReadOnly;
    color_attach_ref.mReqLayout        = ImageLayout::ColorAttachment;
    color_attach_ref.mAttachmentId     = 0;
    render_pass_desc.mAttachments.push_back( render_pass_color_desc );

    /// NORMALS/DEPTH G-BUFFER
    AttachmentDescription render_pass_normals_desc{};
    AttachmentRef         normals_attach_ref{};
    render_pass_normals_desc.mFormat     = ImageBufferFormat::RGBA16;
    render_pass_normals_desc.mDestLayout = ImageLayout::ShaderReadOnly;
    normals_attach_ref.mReqLayout        = ImageLayout::ColorAttachment;
    normals_attach_ref.mAttachmentId     = 1;
    render_pass_desc.mAttachments.push_back( render_pass_normals_desc );

    SubpassDescription main_subpass{};
    main_subpass.mBindPoint = PipelineBindPoint::Graphics;
    main_subpass.mColorAttachments.push_back( color_attach_ref );
    main_subpass.mColorAttachments.push_back( normals_attach_ref );

    render_pass_desc.mSubpasses.push_back( main_subpass );

    mRenderPass =
        DeviceGlobals::RenderHookDevice->CreateRenderPass( render_pass_desc );
}

void GBufferPass::InitializePipeline()
{
    using namespace rh::engine;
    // layouts
    std::array<DescriptorBinding, 1> camera_desc_set_bindings = {
        { { 0, DescriptorType::ROBuffer, 1, ShaderStage::Vertex, 0, 0 } } };
    mCameraSetLayout =
        DeviceGlobals::RenderHookDevice->CreateDescriptorSetLayout(
            { camera_desc_set_bindings } );

    std::array desc_set_bindings = { DescriptorBinding{
        0, DescriptorType::ROBuffer, 1, ShaderStage::Vertex, 0, 1 } };
    mModelSetLayout =
        DeviceGlobals::RenderHookDevice->CreateDescriptorSetLayout(
            { desc_set_bindings } );

    // create pipeline layouts
    std::array layout_array = { mCameraSetLayout, mModelSetLayout };

    PipelineLayoutCreateParams pipe_layout_ci{};
    pipe_layout_ci.mSetLayouts = layout_array;
    mPipeLayout =
        DeviceGlobals::RenderHookDevice->CreatePipelineLayout( pipe_layout_ci );

    mVsDesc = { .mShaderPath  = "shaders/d3d11/engine/gbuffer.hlsl",
                .mEntryPoint  = "BaseVS",
                .mShaderStage = ShaderStage::Vertex };

    mPsDesc = { .mShaderPath  = "shaders/d3d11/engine/gbuffer.hlsl",
                .mEntryPoint  = "BasePS",
                .mShaderStage = ShaderStage::Pixel };

    ShaderStageDesc vs_stage_desc{ .mStage      = mVsDesc.mShaderStage,
                                   .mShader     = mVertexShader,
                                   .mEntryPoint = mVsDesc.mEntryPoint };

    ShaderStageDesc ps_stage_desc{ .mStage      = mPsDesc.mShaderStage,
                                   .mShader     = mPixelShader,
                                   .mEntryPoint = mPsDesc.mEntryPoint };

    std::array<VertexBindingDesc, 1> vertex_binding_desc = {
        VertexBindingDesc{ 0, sizeof( VertexDescPosColorUVNormals ),
                           VertexBindingRate::PerVertex } };

    std::array vertex_layout_desc = {
        VertexInputElementDesc{ 0, 0, InputElementType::Vec4fp32, 0, "POSITION",
                                0 },
        VertexInputElementDesc{ 0, 1, InputElementType::Vec2fp32, 16,
                                "TEXCOORD", 0 },
        VertexInputElementDesc{ 0, 2, InputElementType::Vec3fp32, 24, "NORMAL",
                                0 },
        VertexInputElementDesc{ 0, 3, InputElementType::Vec4fp32, 36, "WEIGHTS",
                                0 },
        VertexInputElementDesc{ 0, 4, InputElementType::Uint32, 36 + 16,
                                "INDICES", 0 },
        VertexInputElementDesc{ 0, 5, InputElementType::Vec4fp8, 36 + 16 + 4,
                                "COLOR", 0 },
        VertexInputElementDesc{ 0, 6, InputElementType::Uint32, 36 + 16 + 8,
                                "MAT_IDX", 0 } };

    RasterPipelineCreateParams pipe_create_params{
        .mRenderPass           = mRenderPass,
        .mLayout               = mPipeLayout,
        .mShaderStages         = { vs_stage_desc, ps_stage_desc },
        .mVertexInputStateDesc = { vertex_binding_desc, vertex_layout_desc },
        .mTopology             = Topology::TriangleList };

    mPipeline = DeviceGlobals::RenderHookDevice->CreateRasterPipeline(
        pipe_create_params );
}

void GBufferPass::InitializeFrameBuffer()
{
    using namespace rh::engine;

    auto  gbuffer_resolution_w = 1920;
    auto  gbuffer_resolution_h = 1080;
    auto &device               = *DeviceGlobals::RenderHookDevice;

    {
        ImageBufferCreateParams render_buffer_ci{};
        render_buffer_ci.mDimension = ImageDimensions::d2D;
        render_buffer_ci.mUsage =
            ImageBufferUsage::ColorAttachment | ImageBufferUsage::Sampled;
        render_buffer_ci.mFormat = ImageBufferFormat::RGBA16;
        render_buffer_ci.mWidth  = gbuffer_resolution_w;
        render_buffer_ci.mHeight = gbuffer_resolution_h;

        mFramebufferImages[0] = device.CreateImageBuffer( render_buffer_ci );

        ImageViewCreateInfo rb_img_view_ci{};
        rb_img_view_ci.mUsage =
            ImageViewUsage::RenderTarget | ImageViewUsage::ShaderResource;
        rb_img_view_ci.mFormat = render_buffer_ci.mFormat;
        rb_img_view_ci.mBuffer = mFramebufferImages[0];

        mFramebufferImageViews[0] = device.CreateImageView( rb_img_view_ci );
    }

    {
        ImageBufferCreateParams render_buffer_ci{};
        render_buffer_ci.mDimension = ImageDimensions::d2D;
        render_buffer_ci.mUsage =
            ImageBufferUsage::ColorAttachment | ImageBufferUsage::Sampled;
        render_buffer_ci.mFormat = ImageBufferFormat::RGBA16;
        render_buffer_ci.mWidth  = gbuffer_resolution_w;
        render_buffer_ci.mHeight = gbuffer_resolution_h;

        mFramebufferImages[1] = device.CreateImageBuffer( render_buffer_ci );

        ImageViewCreateInfo rb_img_view_ci{};
        rb_img_view_ci.mUsage =
            ImageViewUsage::RenderTarget | ImageViewUsage::ShaderResource;
        rb_img_view_ci.mFormat = render_buffer_ci.mFormat;
        rb_img_view_ci.mBuffer = mFramebufferImages[1];

        mFramebufferImageViews[1] = device.CreateImageView( rb_img_view_ci );
    }
    FrameBufferCreateParams create_params{};
    create_params.width      = gbuffer_resolution_w;
    create_params.height     = gbuffer_resolution_h;
    create_params.imageViews = mFramebufferImageViews;
    create_params.renderPass = mRenderPass;

    mFramebuffer = device.CreateFrameBuffer( create_params );
}

void GBufferPass::RenderMesh()
{
    /*using namespace rh::engine;
    auto                               cmd_buff = mCommandBuffer;
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

    cmd_buff->BindPipeline( mPipeline );

    cmd_buff->DrawIndexed( mesh.mIndexCount, 1, 0, 0, 0 );*/
}

} // namespace rh::rw::engine