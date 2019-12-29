#include "camerabeginupdatecmd.h"
#include "../global_definitions.h"
#include "../rw_macro_constexpr.h"
#include "RwRenderEngine.h"
#include "rw_engine/rw_api_injectors.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/viewport.h>
#include <Engine/D3D11Impl/ImageBuffers/D3D11DepthStencilBuffer.h>
#include <Engine/IRenderer.h>
#include <common.h>

using namespace rh::rw::engine;

RwCameraBeginUpdateCmd::RwCameraBeginUpdateCmd( RwCamera *camera )
    : m_pCamera( camera )
{
    rh::debug::DebugLogger::Log( "CameraBeginUpdateCmd created...",
                                 rh::debug::LogLevel::ConstrDestrInfo );
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
    void *frameBufferInternal = GetInternalRaster( m_pCamera->frameBuffer );

    SetupCameraContext();

    // We treat default camera as a output view
    if ( m_pCamera->frameBuffer->cType == RwRasterType::rwRASTERTYPECAMERA )
    {
        // Hack to resize main depth-stencil buffer after swapchain is resized
        if ( g_bRwSwapchainChanged )
        {
            if ( m_pCamera->zBuffer )
            {
                rh::engine::g_pRHRenderer->BindImageBuffers(
                    rh::engine::ImageBindType::DepthStencilTarget,
                    {{0, nullptr}} );
                g_pRaster_API.fpDestroyRaster( m_pCamera->zBuffer );
                m_pCamera->zBuffer = g_pRaster_API.fpCreateRaster(
                    m_pCamera->frameBuffer->width,
                    m_pCamera->frameBuffer->height, 32, rwRASTERTYPEZBUFFER );
            }
            g_bRwSwapchainChanged = false;
        }
        if ( !rh::engine::g_pRHRenderer->RequestSwapChainImage(
                 frameBufferInternal ) )
            return false;
    }

    if ( !rh::engine::g_pRHRenderer->BeginCommandList( nullptr ) )
        return false;

    rh::engine::g_pRHRenderer->BindImageBuffers(
        rh::engine::ImageBindType::RenderTarget, {{0, frameBufferInternal}} );
    if ( m_pCamera->zBuffer )
    {
        void *zBufferInternal = GetInternalRaster( m_pCamera->zBuffer );

        if ( zBufferInternal )
        {
            rh::engine::g_pRHRenderer->BindImageBuffers(
                rh::engine::ImageBindType::DepthStencilTarget,
                {{0, zBufferInternal}} );
        }
    }

    // Set viewport
    rh::engine::ViewPort vp{};
    vp.width    = static_cast<float>( m_pCamera->frameBuffer->width );
    vp.height   = static_cast<float>( m_pCamera->frameBuffer->height );
    vp.maxDepth = 1.0;
    rh::engine::g_pRHRenderer->BindViewPorts( {vp} );

    return true;
}

void RwCameraBeginUpdateCmd::SetupCameraContext()
{
    DirectX::XMMATRIX world_matrix = GetCameraTransform();
    DirectX::XMStoreFloat4( &g_cameraContext->viewDir, world_matrix.r[2] );
    DirectX::XMStoreFloat4( &g_cameraContext->viewPos, world_matrix.r[3] );

    // View transform matrix is inverse of world transform matrix of camera
    DirectX::XMMATRIX view = DirectX::XMMatrixInverse( nullptr, world_matrix );
    DirectX::XMMATRIX proj = GetProjectionTransform();
    DirectX::XMMATRIX view_proj = view * proj;

    DirectX::XMStoreFloat4x4( &g_cameraContext->viewTransform, view );
    DirectX::XMStoreFloat4x4( &g_cameraContext->projTransform, proj );
    DirectX::XMStoreFloat4x4( &g_cameraContext->clipSpaceTransform, view_proj );
    DirectX::XMStoreFloat4x4( &g_cameraContext->invClipSpaceTransform,
                              world_matrix );
}

DirectX::XMMATRIX RwCameraBeginUpdateCmd::GetCameraTransform()
{
    const RwMatrix &camLTM = ( rwFrame::GetParent( m_pCamera ) )->ltm;

    DirectX::XMMATRIX world_matrix = {
        -camLTM.right.x, -camLTM.right.y, -camLTM.right.z, 0,
        camLTM.up.x,     camLTM.up.y,     camLTM.up.z,     0,
        camLTM.at.x,     camLTM.at.y,     camLTM.at.z,     0,
        camLTM.pos.x,    camLTM.pos.y,    camLTM.pos.z,    1};
    return world_matrix;
}

DirectX::XMMATRIX RwCameraBeginUpdateCmd::GetProjectionTransform()
{
    RwReal m22, m23, m33;
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
    return {m_pCamera->recipViewWindow.x,
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
            m33};
}
