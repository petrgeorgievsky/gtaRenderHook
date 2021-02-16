#include "cameraendupdatecmd.h"
#include "../system_funcs/rw_device_system_globals.h"
#include <DebugUtils/DebugLogger.h>

using namespace rh::rw::engine;

RwCameraEndUpdateCmd::RwCameraEndUpdateCmd( RwCamera *camera )
    : m_pCamera( camera )
{
    rh::debug::DebugLogger::Log( "RwCameraEndUpdateCmd created...",
                                 rh::debug::LogLevel::ConstrDestrInfo );
}

RwCameraEndUpdateCmd::~RwCameraEndUpdateCmd()
{
    rh::debug::DebugLogger::Log( "RwCameraEndUpdateCmd destroyed...",
                                 rh::debug::LogLevel::ConstrDestrInfo );
}

bool RwCameraEndUpdateCmd::Execute()
{
    if ( m_pCamera == nullptr )
        return false;

    /*    auto *cameraBackendExt = GetBackendCameraExt( m_pCamera );
        auto &frame_res =
            cameraBackendExt
                ->mFrameResourceCache[cameraBackendExt->mCurrentFrameId];

        frame_res.mCmdBuffer->EndRenderPass();
        frame_res.mCmdBuffer->EndRecord();

        DeviceGlobals::RenderHookDevice->ExecuteCommandBuffer(
            frame_res.mCmdBuffer, frame_res.mImageAquire,
            frame_res.mRenderExecute );
        frame_res.mBufferIsRecorded = true;
        cameraBackendExt->mCurrentFrameId =
            ( cameraBackendExt->mCurrentFrameId + 1 ) %
       gFrameResourceCacheSize;*/
    gRwDeviceGlobals.DeviceGlobalsPtr->curCamera = nullptr;
    return true;
}
