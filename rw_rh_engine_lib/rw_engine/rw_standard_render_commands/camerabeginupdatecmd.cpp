#include "camerabeginupdatecmd.h"

#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

#include <DebugUtils/DebugLogger.h>
#include <render_client/render_client.h>

namespace rh::rw::engine
{
DirectX::XMFLOAT4X4 GetCameraTransform( RwCamera *camera );
DirectX::XMFLOAT4X4 GetProjectionTransform( RwCamera &camera );

RwCameraBeginUpdateCmd::RwCameraBeginUpdateCmd( RwCamera *camera )
    : m_pCamera( camera )
{
    rh::debug::DebugLogger::Log( "CameraBeginUpdateCmd created...",
                                 rh::debug::LogLevel::ConstrDestrInfo );

#ifndef ARCH_64BIT
    /// Some GTA versions break due to off by one errors with unlocked
    /// framerate, setting FPU to single precision fixes that, it was done in
    /// original RW on engine start and every time beginUpdate command was
    /// executed
    unsigned int current_word = 0;
    _controlfp_s( &current_word, _PC_24, _MCW_PC );
#endif

    // "Fixes" some bugs related to camera update being called 2+ times between
    // frames, TODO: Implement a way to handle multiple cameras?
    // if ( GetCurrentSceneGraph()->mFrameInfo.camera_updated )
    //    return;
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
    gRwDeviceGlobals.DeviceGlobalsPtr->curCamera = m_pCamera;
    return true;
}

void RwCameraBeginUpdateCmd::SetupCameraContext()
{
    assert( m_pCamera );
    auto camera_transform     = GetCameraTransform( m_pCamera );
    auto clip_space_transform = GetProjectionTransform( *m_pCamera );

    using namespace DirectX;
    auto world_matrix = XMLoadFloat4x4( &camera_transform );

    // View transform matrix is inverse of world transform matrix of camera
    auto view = XMMatrixInverse( nullptr, XMMatrixTranspose( world_matrix ) );
    auto proj = XMLoadFloat4x4( &clip_space_transform );
    auto proj_inv = XMMatrixInverse( nullptr, proj );

    auto &frame_info = gRenderClient->RenderState.ViewportState.Camera;

    frame_info.mViewPrev = frame_info.mView;
    frame_info.mProjPrev = frame_info.mProj;

    DirectX::XMStoreFloat4x4( &frame_info.mView, view );
    DirectX::XMStoreFloat4x4( &frame_info.mProj, proj );
    DirectX::XMStoreFloat4x4( &frame_info.mViewInv, world_matrix );
    DirectX::XMStoreFloat4x4( &frame_info.mProjInv, proj_inv );

    // frame_info.camera_updated = true;
}

DirectX::XMFLOAT4X4 GetCameraTransform( RwCamera *camera )
{
    const RwMatrix &camLTM = ( rwFrame::GetParent( camera ) )->ltm;

    return { -camLTM.right.x, -camLTM.right.y, -camLTM.right.z, 0,
             -camLTM.up.x,    -camLTM.up.y,    -camLTM.up.z,    0,
             camLTM.at.x,     camLTM.at.y,     camLTM.at.z,     0,
             camLTM.pos.x,    camLTM.pos.y,    camLTM.pos.z,    1 };
}

DirectX::XMFLOAT4X4 GetProjectionTransform( RwCamera &camera )
{
    float m22, m23, m33;
    if ( camera.projectionType == rwPARALLEL )
    {
        m22 = 1.0f / ( camera.farPlane - camera.nearPlane );
        m23 = 0.0f;
        m33 = 1.0f;
    }
    else
    {
        m22 = camera.farPlane / ( camera.farPlane - camera.nearPlane );
        m23 = 1.0f;
        m33 = 0.0f;
    }
    return { camera.recipViewWindow.x,
             0,
             0,
             0,
             0,
             camera.recipViewWindow.y,
             0,
             0,
             camera.recipViewWindow.x * camera.viewOffset.x,
             camera.recipViewWindow.y * camera.viewOffset.y,
             m22,
             m23,
             -camera.recipViewWindow.x * camera.viewOffset.x,
             -camera.recipViewWindow.y * camera.viewOffset.y,
             -m22 * camera.nearPlane,
             m33 };
}
} // namespace rh::rw::engine