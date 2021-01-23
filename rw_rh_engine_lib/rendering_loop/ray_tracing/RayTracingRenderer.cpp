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
#include <imgui.h>
#include <ipc/MemoryReader.h>
#include <numeric>
#include <rendering_loop/compute_skin_animation.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

namespace rh::rw::engine
{

RayTracingRenderer::RayTracingRenderer()
{
    const uint32_t rtx_resolution_w =
        rh::engine::EngineConfigBlock::It.RendererWidth;
    const uint32_t rtx_resolution_h =
        rh::engine::EngineConfigBlock::It.RendererHeight;
    // Non-RT stuff:
    for ( auto &fb : mFramebufferCache )
        fb = nullptr;
    mCameraDescription = new CameraDescription();
    mSkinAnimationPipe = new SkinAnimationPipeline( 110 );

    // Filters
    mVarTempAcummFilterPipe      = new VarAwareTempAccumFilterPipe();
    mVarTempAccumColorFilterPipe = new VarAwareTempAccumColorFilterPipe();
    mBilPipe                     = new BilateralFilterPipeline();

    // RT Stuff
    mBlasBuildPass    = new RTBlasBuildPass();
    mTlasBuildPass    = new RTTlasBuildPass();
    mSceneDescription = new RTSceneDescription();

    mPrimaryRaysPass = new RTPrimaryRaysPass( { .mScene  = mSceneDescription,
                                                .mCamera = mCameraDescription,
                                                .mWidth  = rtx_resolution_w,
                                                .mHeight = rtx_resolution_h } );

    mTiledLightCulling = new TiledLightCulling( TiledLightCullingParams{
        .mDevice       = gRenderDriver->GetDeviceState(),
        .mCameraDesc   = mCameraDescription,
        .mWidth        = rtx_resolution_w,
        .mHeight       = rtx_resolution_h,
        .mCurrentDepth = mPrimaryRaysPass->GetNormalsView() } );

    mRTAOPass = new RTAOPass( RTAOInitParams{
        mSceneDescription, mCameraDescription, mVarTempAcummFilterPipe,
        mBilPipe, mPrimaryRaysPass->GetNormalsView(),
        mPrimaryRaysPass->GetPrevNormalsView(),
        mPrimaryRaysPass->GetMotionView(), rtx_resolution_w,
        rtx_resolution_h } );

    mRTShadowsPass    = new RTShadowsPass( RTShadowsInitParams{
        rtx_resolution_w,
        rtx_resolution_h,
        mSceneDescription,
        mCameraDescription,
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
        .mWidth                = rtx_resolution_w,
        .mHeight               = rtx_resolution_h,
        .mScene                = mSceneDescription,
        .mCamera               = mCameraDescription,
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
            mPrimaryRaysPass->GetAlbedoView(),
            mPrimaryRaysPass->GetNormalsView(),
            mPrimaryRaysPass->GetMaterialsView(), mRTAOPass->GetAOView(),
            mRTShadowsPass->GetShadowsView(),
            mRTReflectionPass->GetReflectionView(),
            mPrimaryRaysPass->GetSkyCfg(), rtx_resolution_w,
            rtx_resolution_h } );

    // Utility TODO: Make it less painful
    mDebugPipeline = new DebugPipeline( DebugPipelineInitParams{
        .mWidth           = rtx_resolution_w,
        .mHeight          = rtx_resolution_h,
        .mTiledLightsList = mTiledLightCulling->GetTileListBuffer() } );

    mFrameTimeGraph.resize( 100, 0.00f );
}

void RayTracingRenderer::OnResize( const rh::engine::WindowParams &window ) {}

std::vector<rh::engine::CommandBufferSubmitInfo>
RayTracingRenderer::Render( SceneInfo *scene, rh::engine::ICommandBuffer *dest,
                            const rh::engine::SwapchainFrame &frame )
{
    using namespace rh::engine;

    auto &device       = gRenderDriver->GetDeviceState();
    auto  forward_pass = GetForwardPass();
    auto  framebuffer  = GetFrameBuffer( frame, forward_pass );
    auto  im2d         = GetIm2DRenderer( forward_pass );
    auto  im3d         = GetIm3DRenderer( forward_pass );
    auto  imgui        = GetImGui( forward_pass );

    auto record_start = std::chrono::high_resolution_clock::now();

    auto &mesh_pool = gRenderDriver->GetResources().GetMeshPool();
    for ( const auto &item : mSkinDrawCallList )
        mesh_pool.FreeResource( item.mMeshId );
    mSkinDrawCallList.clear();
    mRenderDispatchList.clear();

    ProcessDynamicGeometry( scene );

    mBlasBuildPass->Execute();
    if ( mBlasBuildPass->Completed() )
    {
        mRenderDispatchList.push_back( mBlasBuildPass->GetSubmitInfo(
            !mRenderDispatchList.empty()
                ? mRenderDispatchList.back().mToSignalDep
                : nullptr ) );
    }

    auto raytraced = RenderPrimaryRays( scene->mSceneMeshRenderBlock,
                                        scene->mSkinMeshRenderBlock );

    dest->BeginRecord();

    // Set viewport
    dest->SetViewports(
        0, { ViewPort{ .width    = static_cast<float>( frame.mWidth ),
                       .height   = static_cast<float>( frame.mHeight ),
                       .maxDepth = 1.0 } } );
    dest->SetScissors( 0, { Scissor{ 0, 0, frame.mWidth, frame.mHeight } } );

    mCameraDescription->Update( scene->mFrameInfo );
    if ( raytraced )
    {
        mSceneDescription->Update();
        mPrimaryRaysPass->Execute( mTLAS, dest, *scene->mFrameInfo );
        mTiledLightCulling->Execute( dest, *scene->mFrameInfo );
        mRTAOPass->Execute( mTLAS, dest );
        mRTShadowsPass->Execute( mTLAS, dest, *scene->mFrameInfo );
        mRTReflectionPass->Execute( mTLAS, dest, *scene->mFrameInfo );
        mDeferredComposePass->Execute( dest );
        mPrimaryRaysPass->ConvertNormalsToShaderRO( dest );
        // mDebugPipeline->Execute( dest );
    }

    std::array clear_values = {
        ClearValue{
            ClearValueType::Color,
            ClearValue::ClearColor{ scene->mFrameInfo->mClearColor.red,
                                    scene->mFrameInfo->mClearColor.green,
                                    scene->mFrameInfo->mClearColor.blue,
                                    scene->mFrameInfo->mClearColor.alpha },
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
    im3d->Render( scene->mIm3DRenderBlock, dest );
    im2d->Render( scene->mFrontendRenderBlock, dest );

    if ( imgui )
    {
        imgui->BeginFrame();
        DrawGUI();
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

bool RayTracingRenderer::RenderPrimaryRays( void *scene_data,
                                            void *skin_mesh_block )
{
    using namespace rh::engine;
    MemoryReader reader( scene_data );
    MemoryReader skin_reader( skin_mesh_block );

    uint64_t draw_call_count = *reader.Read<uint64_t>();
    if ( draw_call_count <= 0 )
        return false;

    skin_reader.Skip( sizeof( uint64_t ) * 2 );

    uint64_t                 material_count = *reader.Read<uint64_t>();
    ArrayProxy<MaterialData> materials(
        reader.Read<MaterialData>( material_count ), material_count );

    uint64_t skin_material_count = *skin_reader.Read<uint64_t>();
    ArrayProxy<MaterialData> skin_materials(
        skin_reader.Read<MaterialData>( skin_material_count ),
        skin_material_count );

    // Build TLAS
    std::vector<VkAccelerationStructureInstanceNV> instance_buffer{};
    instance_buffer.reserve( draw_call_count );
    uint64_t i = 0;

    ArrayProxy<DrawCallInfo> static_meshes(
        reader.Read<DrawCallInfo>( draw_call_count ), draw_call_count );

    for ( const auto &dc : static_meshes )
    {
        const auto &mesh = mBlasBuildPass->GetBlas( dc.mMeshId );
        if ( !mesh.mBlasBuilt )
            continue;

        mSceneDescription->RecordDrawCall(
            dc, &materials[dc.mMaterialListStart], dc.mMaterialListCount );

        auto blas = (VulkanBottomLevelAccelerationStructure *)mesh.mBLAS;

        VkAccelerationStructureInstanceNV instance{};
        std::copy( &dc.mWorldTransform.m[0][0],
                   &dc.mWorldTransform.m[0][0] + 3 * 4,
                   &instance.transform.matrix[0][0] );
        instance.mask                           = 0xFF;
        instance.accelerationStructureReference = blas->GetAddress();
        instance.instanceCustomIndex            = i;
        instance_buffer.push_back( instance );

        i++;
    }

    for ( const auto &skindc : mSkinDrawCallList )
    {
        const auto &mesh = mBlasBuildPass->GetBlas( skindc.mMeshId );
        if ( !mesh.mBlasBuilt )
            continue;
        mSceneDescription->RecordDrawCall(
            skindc, &skin_materials[skindc.mMaterialListStart],
            skindc.mMaterialListCount );

        auto blas = (VulkanBottomLevelAccelerationStructure *)mesh.mBLAS;

        VkAccelerationStructureInstanceNV instance{};
        std::copy( &skindc.mWorldTransform.m[0][0],
                   &skindc.mWorldTransform.m[0][0] + 3 * 4,
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

rh::engine::ArrayProxy<SkinDrawCallInfo> GetSkinMeshArray( void *mesh_block )
{
    MemoryReader reader( mesh_block );
    reader.Skip( sizeof( uint64_t ) );
    uint64_t drawcall_count  = *reader.Read<uint64_t>();
    uint64_t materials_count = *reader.Read<uint64_t>();
    reader.Skip( sizeof( MaterialData ) * materials_count );

    return rh::engine::ArrayProxy<SkinDrawCallInfo>(
        reader.Read<SkinDrawCallInfo>( drawcall_count ), drawcall_count );
}

void RayTracingRenderer::ProcessDynamicGeometry( SceneInfo *scene )
{
    using namespace rh::engine;
    rh::engine::ArrayProxy<SkinDrawCallInfo> skin_draw_calls =
        GetSkinMeshArray( scene->mSkinMeshRenderBlock );

    if ( skin_draw_calls.Size() <= 0 )
        return;

    // Generate Skin Meshes
    auto animated_meshes =
        mSkinAnimationPipe->AnimateSkinnedMeshes( skin_draw_calls );
    if ( animated_meshes.empty() )
        return;

    auto &mesh_pool = gRenderDriver->GetResources().GetMeshPool();

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
        sdc.mDrawCallId = dc.mInstanceId;
        sdc.mMeshId = mesh_pool.RequestResource( std::move( backendMeshData ) );
        sdc.mWorldTransform    = dc.mTransform;
        sdc.mMaterialListStart = dc.mMaterialListStart;
        sdc.mMaterialListCount = dc.mMaterialListCount;

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

void RayTracingRenderer::DrawGUI()
{
    static auto last_frame_time = std::chrono::high_resolution_clock::now();
    auto        ms_from_lf =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - last_frame_time )
            .count() /
        1000.0f;
    return;
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
    ImGui::EndGroup();
    ImGui::End();
    last_frame_time = std::chrono::high_resolution_clock::now();
}

rh::engine::IRenderPass *RayTracingRenderer::GetForwardPass()
{
    using namespace rh::engine;
    auto &device = gRenderDriver->GetDeviceState();
    if ( mForwardPass != nullptr )
        return mForwardPass;
    mForwardPass = device.CreateRenderPass( RenderPassCreateParams{
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
    auto &device = gRenderDriver->GetDeviceState();
    auto &window = gRenderDriver->GetMainWindow();
    if ( mImGUI )
        return mImGUI;

    mImGUI = dynamic_cast<VulkanDeviceState &>( device ).CreateImGUI( &window );

    mImGUI->Init( { pass } );

    ScopedPointer cmd_buffer = device.CreateCommandBuffer();

    cmd_buffer->BeginRecord();
    mImGUI->UploadFonts( cmd_buffer );
    cmd_buffer->EndRecord();

    device.ExecuteCommandBuffer( cmd_buffer, nullptr, nullptr );
    device.Wait( { cmd_buffer->ExecutionFinishedPrimitive() } );

    return mImGUI;
}

Im2DRenderer *
RayTracingRenderer::GetIm2DRenderer( rh::engine::IRenderPass *pass )
{
    if ( im2DRendererGlobals != nullptr )
        return im2DRendererGlobals;
    im2DRendererGlobals = new Im2DRenderer( mCameraDescription, pass );
    return im2DRendererGlobals;
}

Im3DRenderer *
RayTracingRenderer::GetIm3DRenderer( rh::engine::IRenderPass *pass )
{
    if ( im3DRenderer != nullptr )
        return im3DRenderer;
    im3DRenderer = new Im3DRenderer( mCameraDescription, pass );
    return im3DRenderer;
}

rh::engine::IFrameBuffer *
RayTracingRenderer::GetFrameBuffer( const rh::engine::SwapchainFrame &frame,
                                    rh::engine::IRenderPass *         pass )
{
    auto &framebuffer = mFramebufferCache[frame.mImageId];
    using namespace rh::engine;
    auto &device = gRenderDriver->GetDeviceState();

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
        mDepthBuffer         = device.CreateImageBuffer( ds_buffer );

        ImageViewCreateInfo ds_view{};
        ds_view.mBuffer  = mDepthBuffer;
        ds_view.mFormat  = ImageBufferFormat::D24S8;
        ds_view.mUsage   = ImageViewUsage::DepthStencilTarget;
        mDepthBufferView = device.CreateImageView( ds_view );

        mFrameWidth  = frame.mWidth;
        mFrameHeight = frame.mHeight;
    }

    framebuffer = device.CreateFrameBuffer(
        { .width      = frame.mWidth,
          .height     = frame.mHeight,
          .imageViews = { frame.mImageView, mDepthBufferView },
          .renderPass = pass } );

    return framebuffer;
}
} // namespace rh::rw::engine
  // namespace rh::rw::engine