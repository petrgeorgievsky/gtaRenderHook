#include "ClearScreenSample.h"
#include <common_headers.h>
#include <rw_engine/rw_camera/rw_camera.h>

void ClearScreenSample::CustomRender()
{
    const unsigned char min_val = 0;
    const unsigned char max_val = 0xFF;
    m_nCurrentRed = min( max( static_cast<unsigned char>( m_nCurrentRed + m_nCurrentRedIncr ), min_val ), max_val );
    
    // If we reached 100% pink or 100% blue we'll change increment direction
    if (m_nCurrentRed % 0xFF == 0)
        m_nCurrentRedIncr *= -1;
    
    RwRGBA clearColor = { m_nCurrentRed, 0, 255, 255 };
    rh::rw::engine::RwCameraClear( m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE );
}
