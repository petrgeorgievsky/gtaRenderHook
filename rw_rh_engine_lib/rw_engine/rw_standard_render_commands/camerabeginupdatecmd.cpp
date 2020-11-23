#include "camerabeginupdatecmd.h"
#include "../global_definitions.h"
#include "../rh_backend/camera_backend.h"
#include "../rh_backend/raster_backend.h"
#include "../rw_macro_constexpr.h"
#include "../system_funcs/rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/ICommandBuffer.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ISwapchain.h>
#include <Engine/Common/IWindow.h>
#include <render_loop.h>
#include <scene_graph.h>

using namespace rh::rw::engine;

RwCameraBeginUpdateCmd::RwCameraBeginUpdateCmd( RwCamera *camera )
    : m_pCamera( camera )
{
    rh::debug::DebugLogger::Log( "CameraBeginUpdateCmd created...",
                                 rh::debug::LogLevel::ConstrDestrInfo );
    // hot steamy bug fix
    if ( GetCurrentSceneGraph()->mFrameInfo.camera_updated )
        return;
    SetupCameraContext();
}

RwCameraBeginUpdateCmd::~RwCameraBeginUpdateCmd()
{
    rh::debug::DebugLogger::Log( "CameraBeginUpdateCmd destroyed...",
                                 rh::debug::LogLevel::ConstrDestrInfo );
}

bool RwCameraBeginUpdateCmd::Execute()
{
    if ( m_pCamera == nullptr || m_pCamera->frameBuffer == nullptr )
        return false;
    /* auto *frameBufferInternal = GetBackendRasterExt( m_pCamera->frameBuffer
     ); auto *cameraBackendExt    = GetBackendCameraExt( m_pCamera ); auto
     frame_res = cameraBackendExt
             ->mFrameResourceCache[cameraBackendExt->mCurrentFrameId];
     auto                    command_buffer    = frame_res.mCmdBuffer;
     auto                    image_acquire     = frame_res.mImageAquire;
     rh::engine::IImageView *frame_raster_view = nullptr;

     // SetupCameraContext();
     if ( frame_res.mBufferIsRecorded )
     {
         std::array<rh::engine::ISyncPrimitive *, 1> exec_prim_list = {
             command_buffer->ExecutionFinishedPrimitive()};

         DeviceGlobals::RenderHookDevice->Wait( exec_prim_list );
         frame_res.mBufferIsRecorded = false;
     }
     uint32_t width  = m_pCamera->frameBuffer->width;
     uint32_t height = m_pCamera->frameBuffer->height;
     // We treat default camera as a output view
     if ( m_pCamera->frameBuffer->cType == RwRasterType::rwRASTERTYPECAMERA )
     {
         auto swap_chain = frameBufferInternal->mWindow->GetSwapchain();
         auto frame = swap_chain.mSwapchain->GetAvaliableFrame( image_acquire );

         cameraBackendExt->mCurrentFramebufferId = frame.mImageId;
         frame_raster_view                       = frame.mImageView;
         width                                   = frame.mWidth;
         height                                  = frame.mHeight;

         frameBufferInternal->mCurrentBackBufferId = frame.mImageId;
         frameBufferInternal->mRenderExecute       = frame_res.mRenderExecute;
     }
     auto &framebuffer =
         cameraBackendExt
             ->mFramebufferCache[cameraBackendExt->mCurrentFramebufferId];

     if ( cameraBackendExt->mRenderPass == nullptr )
     {
         rh::engine::RenderPassCreateParams render_pass_desc{};

         rh::engine::AttachmentDescription render_pass_color_desc{};
         render_pass_color_desc.mFormat = rh::engine::ImageBufferFormat::BGRA8;
         render_pass_color_desc.mSrcLayout = rh::engine::ImageLayout::Undefined;
         render_pass_color_desc.mDestLayout =
             rh::engine::ImageLayout::PresentSrc;
         render_pass_color_desc.mLoadOp         = rh::engine::LoadOp::Clear;
         render_pass_color_desc.mStoreOp        = rh::engine::StoreOp::Store;
         render_pass_color_desc.mStencilLoadOp  = rh::engine::LoadOp::DontCare;
         render_pass_color_desc.mStencilStoreOp = rh::engine::StoreOp::DontCare;
         render_pass_desc.mAttachments.push_back( render_pass_color_desc );

         rh::engine::SubpassDescription main_subpass{};
         rh::engine::AttachmentRef      color_attach_ref{};
         color_attach_ref.mReqLayout = rh::engine::ImageLayout::ColorAttachment;
         main_subpass.mBindPoint     = rh::engine::PipelineBindPoint::Graphics;
         main_subpass.mColorAttachments.push_back( color_attach_ref );
         render_pass_desc.mSubpasses.push_back( main_subpass );

         cameraBackendExt->mRenderPass =
             DeviceGlobals::RenderHookDevice->CreateRenderPass(
                 render_pass_desc );
     }
     if ( framebuffer == nullptr )
     {
         std::vector<rh::engine::IImageView *> img_views{frame_raster_view};
         rh::engine::FrameBufferCreateParams   create_params{};
         create_params.width      = width;
         create_params.height     = height;
         create_params.imageViews = img_views;
         create_params.renderPass = cameraBackendExt->mRenderPass;

         framebuffer =
             DeviceGlobals::RenderHookDevice->CreateFrameBuffer( create_params
     );
     }

     command_buffer->BeginRecord();
     // Set viewport
     rh::engine::ViewPort vp{};
     vp.width    = static_cast<float>( width );
     vp.height   = static_cast<float>( height );
     vp.maxDepth = 1.0;
     std::array<rh::engine::ViewPort, 1> viewports = {vp};
     command_buffer->SetViewports( 0, viewports );
     std::array scissors = {rh::engine::Scissor{
         0,     // float topLeftX;
         0,     // float topLeftY;
         width, // float width;
         height // float height;
     }};
     command_buffer->SetScissors( 0, scissors );

     std::array<rh::engine::ClearValue, 1> clear_values = {
         rh::engine::ClearValue{rh::engine::ClearValueType::Color,
                                rh::engine::ClearValue::ClearColor{
                                    cameraBackendExt->mClearColor.red,
                                    cameraBackendExt->mClearColor.green,
                                    cameraBackendExt->mClearColor.blue,
                                    cameraBackendExt->mClearColor.alpha},
                                {}}};
     rh::engine::RenderPassBeginInfo info{};
     info.m_pRenderPass  = cameraBackendExt->mRenderPass;
     info.m_pFrameBuffer = framebuffer;
     info.m_aClearValues = clear_values;
     command_buffer->BeginRenderPass( info );*/
    DeviceGlobals::DeviceGlobalsPtr->curCamera = m_pCamera;
    return true;
}

void RwCameraBeginUpdateCmd::SetupCameraContext()
{
    auto              camera_transform     = GetCameraTransform();
    auto              clip_space_transform = GetProjectionTransform();
    DirectX::XMMATRIX world_matrix =
        DirectX::XMLoadFloat4x4( &camera_transform );
    // View transform matrix is inverse of world transform matrix of camera
    DirectX::XMMATRIX view = DirectX::XMMatrixInverse(
        nullptr, DirectX::XMMatrixTranspose( world_matrix ) );
    DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4( &clip_space_transform );
    DirectX::XMMATRIX proj_inv = DirectX::XMMatrixInverse( nullptr, proj );

    GetCurrentSceneGraph()->mFrameInfo.mViewPrev =
        GetCurrentSceneGraph()->mFrameInfo.mView;
    GetCurrentSceneGraph()->mFrameInfo.mProjPrev =
        GetCurrentSceneGraph()->mFrameInfo.mProj;

    DirectX::XMStoreFloat4x4( &GetCurrentSceneGraph()->mFrameInfo.mView, view );
    DirectX::XMStoreFloat4x4( &GetCurrentSceneGraph()->mFrameInfo.mProj, proj );
    DirectX::XMStoreFloat4x4( &GetCurrentSceneGraph()->mFrameInfo.mViewInv,
                              world_matrix );
    DirectX::XMStoreFloat4x4( &GetCurrentSceneGraph()->mFrameInfo.mProjInv,
                              proj_inv );
    GetCurrentSceneGraph()->mFrameInfo.camera_updated = true;
}

DirectX::XMFLOAT4X4 RwCameraBeginUpdateCmd::GetCameraTransform()
{
    const RwMatrix &camLTM = ( rwFrame::GetParent( m_pCamera ) )->ltm;

    return { -camLTM.right.x, -camLTM.right.y, -camLTM.right.z, 0,
             -camLTM.up.x,    -camLTM.up.y,    -camLTM.up.z,    0,
             camLTM.at.x,     camLTM.at.y,     camLTM.at.z,     0,
             camLTM.pos.x,    camLTM.pos.y,    camLTM.pos.z,    1 };
}

DirectX::XMFLOAT4X4 RwCameraBeginUpdateCmd::GetProjectionTransform()
{
    float m22, m23, m33;
    if ( m_pCamera->projectionType == rwPARALLEL )
    {
        m22 = 1.0f / ( m_pCamera->farPlane - m_pCamera->nearPlane );
        m23 = 0.0f;
        m33 = 1.0f;
    }
    else
    {
        m22 = m_pCamera->farPlane /
              ( m_pCamera->farPlane - m_pCamera->nearPlane );
        m23 = 1.0f;
        m33 = 0.0f;
    }
    return { m_pCamera->recipViewWindow.x,
             0,
             0,
             0,
             0,
             m_pCamera->recipViewWindow.y,
             0,
             0,
             m_pCamera->recipViewWindow.x * m_pCamera->viewOffset.x,
             m_pCamera->recipViewWindow.y * m_pCamera->viewOffset.y,
             m22,
             m23,
             -m_pCamera->recipViewWindow.x * m_pCamera->viewOffset.x,
             -m_pCamera->recipViewWindow.y * m_pCamera->viewOffset.y,
             -m22 * m_pCamera->nearPlane,
             m33 };
}
