#include "cameraendupdatecmd.h"
#include <rw_engine/system_funcs/rw_device_system_globals.h>

#include <DebugUtils/DebugLogger.h>

namespace rh::rw::engine
{

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
    gRwDeviceGlobals.DeviceGlobalsPtr->curCamera = nullptr;
    return true;
}
} // namespace rh::rw::engine
