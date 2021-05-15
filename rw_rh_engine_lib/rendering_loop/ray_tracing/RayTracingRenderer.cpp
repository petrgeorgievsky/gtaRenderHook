//
// Created by peter on 04.05.2020.
//

#include "RayTracingRenderer.h"
#include "BilateralFilterPass.h"
#include "CameraDescription.h"
#include "DeferredCompositionPass.h"
#include "RTAOPass.h"
#include "RTBlasBuildPass.h"
#include "RTPrimaryRaysPass.h"
#include "RTReflectionRaysPass.h"
#include "RTShadowsPass.h"
#include "RTTlasBuildPass.h"
#include "VarAwareTempAccumFilter.h"
#include "VarAwareTempAccumFilterColor.h"
#include "debug_pipeline.h"
#include "scene_description/gpu_mesh_buffer_pool.h"
#include "scene_description/gpu_texture_pool.h"
#include "tiled_light_culling.h"
#include <Engine/Common/ISwapchain.h>
#include <Engine/EngineConfigBlock.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <Engine/VulkanImpl/VulkanWin32Window.h>
#include <data_desc/frame_info.h>
#include <imgui.h>
#include <ipc/MemoryReader.h>
#include <numeric>
#include <render_driver/gpu_resources/resource_mgr.h>
#include <render_driver/render_driver.h>
#include <rendering_loop/compute_skin_animation.h>

namespace rh::rw::engine
{

RayTracingRenderer::RayTracingRenderer( const RendererBase &info )
    : Device( info.Device ), Window( info.Window ), Resources( info.Resources )
{
    const uint32_t rtx_resolution_w =
        rh::engine::EngineConfigBlock::It.RendererWidth;
    const uint32_t rtx_resolution_h =
        rh::engine::EngineConfigBlock::It.RendererHeight;

    // Populate resource pool
    {
        rgResourcePool.Create<CameraDescription>( info );
    }

    // Non-RT stuff:
    for ( auto &fb : mFramebufferCache )
        fb = nullptr;

    mSkinAnimationPipe =
        new SkinAnimationPipeline( { Device, Resources, 110 } );

    // Filters
    mVarTempAcummFilterPipe = new VarAwareTempAccumFilterPipe( Device );
    mVarTempAccumColorFilterPipe =
        new VarAwareTempAccumColorFilterPipe( Device );
    mBilPipe = new BilateralFilterPipeline( Device );

    // RT Stuff
    mBlasBuildPass    = new RTBlasBuildPass( { Device, Resources } );
    mTlasBuildPass    = new RTTlasBuildPass( Device );
    mSceneDescription = new RTSceneDescription( { Device, Resources } );

    mPrimaryRaysPass = new RTPrimaryRaysPass(
        { .Device  = Device,
          .mScene  = mSceneDescription,
          .mCamera = rgResourcePool.Get<CameraDescription>(),
          .mWidth  = rtx_resolution_w,
          .mHeight = rtx_resolution_h } );

    mTiledLightCulling = new TiledLightCulling( TiledLightCullingParams{
        .mDevice       = Device,
        .mCameraDesc   = rgResourcePool.Get<CameraDescription>(),
        .mWidth        = rtx_resolution_w,
        .mHeight       = rtx_resolution_h,
        .mCurrentDepth = mPrimaryRaysPass->GetNormalsView() } );

    mRTAOPass = new RTAOPass( RTAOInitParams{
        Device, mSceneDescription, rgResourcePool.Get<CameraDescription>(),
        mVarTempAcummFilterPipe, mBilPipe, mPrimaryRaysPass->GetNormalsView(),
        mPrimaryRaysPass->GetPrevNormalsView(),
        mPrimaryRaysPass->GetMotionView(), rtx_resolution_w,
        rtx_resolution_h } );

    mRTShadowsPass    = new RTShadowsPass( RTShadowsInitParams{
        Device,
        rtx_resolution_w,
        rtx_resolution_h,
        mSceneDescription,
        rgResourcePool.Get<CameraDescription>(),
        mVarTempAccumColorFilterPipe,
        mBilPipe,
        mTiledLightCulling->GetTileListBuffer(),
        mTiledLightCulling->GetLightIdxListBuffer(),
        mTiledLightCulling->GetLightBuffer(),
        mPrimaryRaysPass->GetNormalsView(),
        mPrimaryRaysPass->GetPrevNormalsView(),
        mPrimaryRaysPass->GetMotionView(),
        mPrimaryRaysPass->GetSkyCfg(),
    } );
    mRTReflectionPass = new RTReflectionRaysPass( RTReflectionInitParams{
        .Device                = Device,
        .mWidth                = rtx_resolution_w,
        .mHeight               = rtx_resolution_h,
        .mScene                = mSceneDescription,
        .mCamera               = rgResourcePool.Get<CameraDescription>(),
        .mVarTAColorFilterPipe = mVarTempAccumColorFilterPipe,
        .mBilateralFilterPipe  = mBilPipe,
        .mNormalsView          = mPrimaryRaysPass->GetNormalsView(),
        .mPrevNormalsView      = mPrimaryRaysPass->GetPrevNormalsView(),
        .mMotionVectorsView    = mPrimaryRaysPass->GetMotionView(),
        .mMaterialsView        = mPrimaryRaysPass->GetMaterialsView(),
        .mSkyCfg               = mPrimaryRaysPass->GetSkyCfg() } );

    // Opaque object composition:
    mDeferredComposePass =
        new DeferredCompositionPass( DeferredCompositionPassParams{
            Device, rgResourcePool.Get<CameraDescription>(),
            mPrimaryRaysPass->GetAlbedoView(),
            mPrimaryRaysPass->GetNormalsView(),
            mPrimaryRaysPass->GetMaterialsView(), mRTAOPass->GetAOView(),
            mRTShadowsPass->GetShadowsView(),
            mRTReflectionPass->GetReflectionView(),
            mPrimaryRaysPass->GetSkyCfg(), rtx_resolution_w,
            rtx_resolution_h } );

    mFrameTimeGraph.resize( 100, 0.00f );
}

void RayTracingRenderer::OnResize( const rh::engine::WindowParams &window ) {}

std::vector<rh::engine::CommandBufferSubmitInfo>
RayTracingRenderer::Render( const FrameState &                state,
                            rh::engine::ICommandBuffer *      dest,
                            const rh::engine::SwapchainFrame &frame )
{
    using namespace rh::engine;

    auto forward_pass = GetForwardPass();
    auto framebuffer  = GetFrameBuffer( frame, forward_pass );
    auto im2d         = GetIm2DRenderer( forward_pass );
    auto im3d         = GetIm3DRenderer( forward_pass );
    auto imgui        = GetImGui( forward_pass );

    auto record_start = std::chrono::high_resolution_clock::now();
    rgResourcePool.Update( state );

    auto &mesh_pool = Resources.GetMeshPool();
    for ( const auto &item : mSkinDrawCallList )
        mesh_pool.FreeResource( item.MeshId );
    mSkinDrawCallList.clear();
    mRenderDispatchList.clear();

    ProcessDynamicGeometry( state.SkinInstances );

    mBlasBuildPass->Execute();
    if ( mBlasBuildPass->Completed() )
    {
        mRenderDispatchList.push_back( mBlasBuildPass->GetSubmitInfo(
            !mRenderDispatchList.empty()
                ? mRenderDispatchList.back().mToSignalDep
                : nullptr ) );
    }

    auto raytraced =
        RenderPrimaryRays( state.MeshInstances, state.SkinInstances );

    dest->BeginRecord();

    // Set viewport
    dest->SetViewports(
        0, { ViewPort{ .width    = static_cast<float>( frame.mWidth ),
                       .height   = static_cast<float>( frame.mHeight ),
                       .maxDepth = 1.0 } } );
    dest->SetScissors( 0, { Scissor{ 0, 0, frame.mWidth, frame.mHeight } } );

    if ( raytraced )
    {
        mSceneDescription->Update();
        mPrimaryRaysPass->Execute( mTLAS, dest, *state.Sky );
        mTiledLightCulling->Execute( dest, state.Lights );
        mRTAOPass->Execute( mTLAS, dest );
        mRTShadowsPass->Execute( mTLAS, dest );
        mRTReflectionPass->Execute( mTLAS, dest );
        mDeferredComposePass->Execute( dest );
        mPrimaryRaysPass->ConvertNormalsToShaderRO( dest );
        // mDebugPipeline->Execute( dest );
    }

    std::array clear_values = {
        ClearValue{ ClearValueType::Color,
                    ClearValue::ClearColor{ state.Viewport->ClearColor.red,
                                            state.Viewport->ClearColor.green,
                                            state.Viewport->ClearColor.blue,
                                            state.Viewport->ClearColor.alpha },
                    {} },
        ClearValue{ ClearValueType::Depth,
                    {},
                    ClearValue::ClearDepthStencil{ 1.0f, 0 } } };

    dest->BeginRenderPass( { .m_pRenderPass  = forward_pass,
                             .m_pFrameBuffer = framebuffer,
                             .m_aClearValues = clear_values } );
    im2d->Reset();
    im3d->Reset();
    if ( raytraced )
    {
        im2d->DrawDepthMask( mPrimaryRaysPass->GetNormalsView(), dest );
        im2d->DrawQuad( mDeferredComposePass->GetResultView(), dest );
    }
    im3d->Render( state.Im3D, dest );
    im2d->Render( state.Im2D, dest );

    if ( imgui )
    {
        ImGuiDriver->NewFrame( *state.ImGuiInput );
        imgui->BeginFrame();
        DrawGUI( state );
        imgui->DrawGui( dest );
    }
    dest->EndRenderPass();
    dest->EndRecord();
    mCPURecordTime =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - record_start )
            .count();
    return mRenderDispatchList;
}

bool RayTracingRenderer::RenderPrimaryRays( const MeshInstanceState &mesh_data,
                                            const SkinInstanceState &skin_data )
{
    using namespace rh::engine;

    uint64_t draw_call_count =
        mesh_data.DrawCalls.Size() + skin_data.DrawCalls.Size();
    if ( draw_call_count <= 0 )
        return false;

    // Build TLAS
    std::vector<VkAccelerationStructureInstanceNV> instance_buffer{};
    instance_buffer.reserve( draw_call_count );
    uint64_t i = 0;

    for ( const auto &dc : mesh_data.DrawCalls )
    {
        const auto &mesh = mBlasBuildPass->GetBlas( dc.MeshId );
        if ( !mesh.mBlasBuilt )
            continue;

        mSceneDescription->RecordDrawCall(
            dc, &mesh_data.Materials[dc.MaterialListStart],
            dc.MaterialListCount );

        auto blas = (VulkanBottomLevelAccelerationStructure *)mesh.mBLAS;

        VkAccelerationStructureInstanceNV instance{};
        std::copy( &dc.WorldTransform.m[0][0],
                   &dc.WorldTransform.m[0][0] + 3 * 4,
                   &instance.transform.matrix[0][0] );
        instance.mask                           = 0xFF;
        instance.accelerationStructureReference = blas->GetAddress();
        instance.instanceCustomIndex            = i;
        instance_buffer.push_back( instance );

        i++;
    }

    for ( const auto &skindc : mSkinDrawCallList )
    {
        const auto &mesh = mBlasBuildPass->GetBlas( skindc.MeshId );
        if ( !mesh.mBlasBuilt )
            continue;
        mSceneDescription->RecordDrawCall(
            skindc, &skin_data.Materials[skindc.MaterialListStart],
            skindc.MaterialListCount );

        auto blas = (VulkanBottomLevelAccelerationStructure *)mesh.mBLAS;

        VkAccelerationStructureInstanceNV instance{};
        std::copy( &skindc.WorldTransform.m[0][0],
                   &skindc.WorldTransform.m[0][0] + 3 * 4,
                   &instance.transform.matrix[0][0] );
        instance.mask                           = 0xFF;
        instance.accelerationStructureReference = blas->GetAddress();
        instance.instanceCustomIndex            = i;
        instance_buffer.push_back( instance );
        i++;
    }

    delete mTLAS;
    mTLAS = mTlasBuildPass->Execute( std::move( instance_buffer ) );
    mRenderDispatchList.push_back( mTlasBuildPass->GetSubmitInfo(
        !mRenderDispatchList.empty() ? mRenderDispatchList.back().mToSignalDep
                                     : nullptr ) );
    return true;
}

void RayTracingRenderer::ProcessDynamicGeometry(
    const SkinInstanceState &state )
{
    using namespace rh::engine;
    if ( state.DrawCalls.Size() <= 0 )
        return;

    // Generate Skin Meshes
    auto animated_meshes =
        mSkinAnimationPipe->AnimateSkinnedMeshes( state.DrawCalls );
    if ( animated_meshes.empty() )
        return;

    auto &mesh_pool = Resources.GetMeshPool();

    // Generate dynamic geometry transforms
    for ( auto &dc : animated_meshes )
    {
        BackendMeshData backendMeshData{};
        backendMeshData.mIndexBuffer  = dc.mData.mIndexBuffer;
        backendMeshData.mVertexBuffer = dc.mData.mVertexBuffer;
        backendMeshData.mVertexCount  = dc.mData.mVertexCount;
        backendMeshData.mIndexCount   = dc.mData.mIndexCount;
        dc.mData.mIndexBuffer->AddRef();

        DrawCallInfo sdc{};
        sdc.DrawCallId = dc.mInstanceId;
        sdc.MeshId = mesh_pool.RequestResource( std::move( backendMeshData ) );
        sdc.WorldTransform    = dc.mTransform;
        sdc.MaterialListStart = dc.mMaterialListStart;
        sdc.MaterialListCount = dc.mMaterialListCount;

        mSkinDrawCallList.push_back( sdc );
    }

    mRenderDispatchList.push_back( mSkinAnimationPipe->GetAnimateSubmitInfo(
        !mRenderDispatchList.empty() ? mRenderDispatchList.back().mToSignalDep
                                     : nullptr ) );
}

RayTracingRenderer::~RayTracingRenderer()
{
    for ( auto fb : mFramebufferCache )
        delete fb;
}

void RayTracingRenderer::DrawGUI( const FrameState &scene )
{
    static auto last_frame_time = std::chrono::high_resolution_clock::now();
    auto        ms_from_lf =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - last_frame_time )
            .count() /
        1000.0f;
    ImGui::Begin( "Info" );

    ImGui::BeginGroup();

    ImGui::Text( "BLAS Built in last frame:%llu.", mBlasBuilt );

    std::rotate( mFrameTimeGraph.begin(), mFrameTimeGraph.begin() + 1,
                 mFrameTimeGraph.end() );
    mFrameTimeGraph[mFrameTimeGraph.size() - 1] = mCPURecordTime / 1000.0f;
    ImGui::PlotLines( "Frame Times", mFrameTimeGraph.data(),
                      mFrameTimeGraph.size() );
    auto avg_frame_rec_time = std::accumulate( mFrameTimeGraph.begin(),
                                               mFrameTimeGraph.end(), 0.0f ) /
                              mFrameTimeGraph.size();
    ImGui::Text( "Avg CPU record time:%.3f ms.", avg_frame_rec_time );
    ImGui::Text( "FPS:%.1f", 1000.0f / ( ms_from_lf + 0.0001f ) );
    mRTAOPass->UpdateUI();
    ImGui::EndGroup();

    if ( ImGui::CollapsingHeader( "Im2DCalls" ) )
    {
        ImGui::Text( "Im2DCall count: %llu",
                     static_cast<uint64_t>( scene.Im2D.DrawCalls.Size() ) );
        int id = 0;
        for ( const auto &im2d_call : scene.Im2D.DrawCalls )
        {
            ImGui::PushID( id );
            if ( ImGui::CollapsingHeader( "Im2DCall" ) )
            {
                ImGui::Text( "RasterId : %llu", im2d_call.mRasterId );
                ImGui::Text( "Index count : %u", im2d_call.mVertexCount );
                ImGui::Text( "Vertex count : %u", im2d_call.mIndexCount );
            }
            ImGui::PopID();
            id++;
        }
    }

    if ( ImGui::CollapsingHeader( "AnimatedDrawCalls" ) )
    {
        ImGui::Text(
            "AnimatedInstances count: %llu",
            static_cast<uint64_t>( scene.SkinInstances.DrawCalls.Size() ) );
        int id     = 0;
        int mtx_id = 0;
        for ( const auto &instance : scene.SkinInstances.DrawCalls )
        {
            ImGui::PushID( id );
            if ( ImGui::CollapsingHeader( "AnimatedInstance" ) )
            {
                ImGui::Text( "Instance Id : %llu", instance.DrawCallId );
                ImGui::Text( "Mesh id : %llu", instance.MeshId );
                ImGui::Text(
                    "World Transform\n "
                    "row_0 - x:%f; y:%f; z:%f;\n"
                    "row_1 - x:%f; y:%f; z:%f;\n"
                    "row_2 - x:%f; y:%f; z:%f;\n"
                    "row_3 - x:%f; y:%f; z:%f;",
                    instance.WorldTransform._11, instance.WorldTransform._12,
                    instance.WorldTransform._13, instance.WorldTransform._21,
                    instance.WorldTransform._22, instance.WorldTransform._23,
                    instance.WorldTransform._31, instance.WorldTransform._32,
                    instance.WorldTransform._33, instance.WorldTransform._41,
                    instance.WorldTransform._42, instance.WorldTransform._43 );

                if ( ImGui::CollapsingHeader( "Matrices" ) )
                {
                    for ( const auto &bone_mtx : instance.BoneTransform )
                    {
                        ImGui::PushID( mtx_id );
                        ImGui::Text( "Matrix %u\n "
                                     "row_0 - x:%f; y:%f; z:%f;\n"
                                     "row_1 - x:%f; y:%f; z:%f;\n"
                                     "row_2 - x:%f; y:%f; z:%f;\n"
                                     "row_3 - x:%f; y:%f; z:%f;",
                                     mtx_id, bone_mtx._11, bone_mtx._12,
                                     bone_mtx._13, bone_mtx._21, bone_mtx._22,
                                     bone_mtx._23, bone_mtx._31, bone_mtx._32,
                                     bone_mtx._33, bone_mtx._41, bone_mtx._42,
                                     bone_mtx._43 );
                        ImGui::PopID();
                        mtx_id++;
                    }
                }
            }
            ImGui::PopID();
            id++;
        }
    }

    ImGui::End();
    last_frame_time = std::chrono::high_resolution_clock::now();
}

rh::engine::IRenderPass *RayTracingRenderer::GetForwardPass()
{
    using namespace rh::engine;
    if ( mForwardPass != nullptr )
        return mForwardPass;
    mForwardPass = Device.CreateRenderPass( RenderPassCreateParams{
        .mAttachments =
            {
                // main framebuffer TODO: Maybe allow for HDR?
                { .mFormat     = ImageBufferFormat::BGRA8,
                  .mDestLayout = ImageLayout::PresentSrc },
                // depth buffer
                { .mFormat     = ImageBufferFormat::D24S8,
                  .mDestLayout = ImageLayout::DepthStencilReadOnly },
            },
        .mSubpasses = {
            { .mBindPoint              = PipelineBindPoint::Graphics,
              .mColorAttachments       = { { .mReqLayout =
                                           ImageLayout::ColorAttachment,
                                       .mAttachmentId = 0 } },
              .mDepthStencilAttachment = AttachmentRef{
                  .mReqLayout    = ImageLayout::DepthStencilAttachment,
                  .mAttachmentId = 1 } } } } );
    return mForwardPass;
}

rh::engine::VulkanImGUI *
RayTracingRenderer::GetImGui( rh::engine::IRenderPass *pass )
{

    using namespace rh::engine;
    if ( mImGUI )
        return mImGUI;
    ImGuiDriver = new ImGuiWin32DriverHandler();
    mImGUI = dynamic_cast<VulkanDeviceState &>( Device ).CreateImGUI( &Window );

    mImGUI->Init( { pass } );
    ImGuiDriver->Init(
        dynamic_cast<VulkanWin32Window &>( Window ).GetHandle() );
    ScopedPointer cmd_buffer = Device.CreateCommandBuffer();

    cmd_buffer->BeginRecord();
    mImGUI->UploadFonts( cmd_buffer );
    cmd_buffer->EndRecord();

    Device.ExecuteCommandBuffer( cmd_buffer, nullptr, nullptr );
    Device.Wait( { cmd_buffer->ExecutionFinishedPrimitive() } );

    return mImGUI;
}

Im2DRenderer *
RayTracingRenderer::GetIm2DRenderer( rh::engine::IRenderPass *pass )
{
    if ( im2DRendererGlobals != nullptr )
        return im2DRendererGlobals;
    im2DRendererGlobals =
        new Im2DRenderer( Device, Resources.GetRasterPool(),
                          rgResourcePool.Get<CameraDescription>(), pass );
    return im2DRendererGlobals;
}

Im3DRenderer *
RayTracingRenderer::GetIm3DRenderer( rh::engine::IRenderPass *pass )
{
    if ( im3DRenderer != nullptr )
        return im3DRenderer;
    im3DRenderer =
        new Im3DRenderer( Device, Resources.GetRasterPool(),
                          rgResourcePool.Get<CameraDescription>(), pass );
    return im3DRenderer;
}

rh::engine::IFrameBuffer *
RayTracingRenderer::GetFrameBuffer( const rh::engine::SwapchainFrame &frame,
                                    rh::engine::IRenderPass *         pass )
{
    auto &framebuffer = mFramebufferCache[frame.mImageId];
    using namespace rh::engine;

    if ( framebuffer != nullptr )
        return framebuffer;
    if ( mFrameWidth != frame.mWidth || mFrameHeight != frame.mHeight )
    {
        // Recreate depth buffer
        delete mDepthBuffer;

        ImageBufferCreateParams ds_buffer{};
        ds_buffer.mHeight    = frame.mHeight;
        ds_buffer.mWidth     = frame.mWidth;
        ds_buffer.mFormat    = ImageBufferFormat::D24S8;
        ds_buffer.mUsage     = ImageBufferUsage::DepthStencilAttachment;
        ds_buffer.mDimension = ImageDimensions::d2D;
        mDepthBuffer         = Device.CreateImageBuffer( ds_buffer );

        ImageViewCreateInfo ds_view{};
        ds_view.mBuffer  = mDepthBuffer;
        ds_view.mFormat  = ImageBufferFormat::D24S8;
        ds_view.mUsage   = ImageViewUsage::DepthStencilTarget;
        mDepthBufferView = Device.CreateImageView( ds_view );

        mFrameWidth  = frame.mWidth;
        mFrameHeight = frame.mHeight;
    }

    framebuffer = Device.CreateFrameBuffer(
        { .width      = frame.mWidth,
          .height     = frame.mHeight,
          .imageViews = { frame.mImageView, mDepthBufferView },
          .renderPass = pass } );

    return framebuffer;
}
} // namespace rh::rw::engine
  // namespace rh::rw::engine