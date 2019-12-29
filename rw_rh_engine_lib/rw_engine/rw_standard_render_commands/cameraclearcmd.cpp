#include "cameraclearcmd.h"
#include "../global_definitions.h"
#include <Engine/Common/types/image_clear_type.h>
#include <Engine/IRenderer.h>
#include <common.h>
using namespace rh::rw::engine;

RwCameraClearCmd::RwCameraClearCmd( RwCamera *camera, RwRGBA *color, int32_t clear_mode )
    : m_pCamera( camera )
    , m_nClearMode( clear_mode )
{
    m_aClearColor = {color->red / 255.0f,
                     color->green / 255.0f,
                     color->blue / 255.0f,
                     color->alpha / 255.0f};
}

RwCameraClearCmd::~RwCameraClearCmd() {}

bool RwCameraClearCmd::Execute()
{
    if ( m_pCamera->frameBuffer && m_nClearMode & rwCAMERACLEARIMAGE ) {
        void *frameBufferInternal = GetInternalRaster( m_pCamera->frameBuffer );
        if ( frameBufferInternal )
            rh::engine::g_pRHRenderer->ClearImageBuffer( rh::engine::ImageClearType::Color,
                                                         frameBufferInternal,
                                                         m_aClearColor );
    }

    if ( m_pCamera->zBuffer && m_nClearMode & rwCAMERACLEARZ ) {
        void *zBufferInternal = GetInternalRaster( m_pCamera->zBuffer );
        if ( zBufferInternal )
            rh::engine::g_pRHRenderer->ClearImageBuffer( rh::engine::ImageClearType::Depth,
                                                         zBufferInternal,
                                                         m_aClearColor );
    }

    return true;
}
