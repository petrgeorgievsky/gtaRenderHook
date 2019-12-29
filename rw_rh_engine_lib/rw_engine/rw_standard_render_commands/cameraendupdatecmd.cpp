#include "cameraendupdatecmd.h"
#include "../global_definitions.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/IRenderer.h>

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
    void *frameBufferInternal = GetInternalRaster( m_pCamera->frameBuffer );

    if ( m_pCamera->frameBuffer == nullptr || frameBufferInternal == nullptr )
        return false;

    if ( !rh::engine::g_pRHRenderer->PresentSwapChainImage( frameBufferInternal ) )
        return false;
    if ( !rh::engine::g_pRHRenderer->EndCommandList( nullptr ) )
        return false;

    return true;
}
