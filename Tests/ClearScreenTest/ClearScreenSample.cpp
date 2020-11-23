#include "ClearScreenSample.h"
#include <common_headers.h>
#include <rw_engine/rw_camera/rw_camera.h>

void ClearScreenSample::CustomRender()
{
    using namespace rh::rw::engine;
    constexpr unsigned char min_val = 0;
    constexpr unsigned char max_val = 0xFF;

    m_nCurrentRed = min(
        max( static_cast<unsigned char>( m_nCurrentRed + m_nCurrentRedIncr ),
             min_val ),
        max_val );

    // If we reached 100% pink or 100% blue we'll change increment direction
    if ( m_nCurrentRed % 0xFF == 0 )
        m_nCurrentRedIncr *= -1;

    RwRGBA clear_color = { m_nCurrentRed, 0, 0xFF, 0xFF };
    RwCameraClear( m_pMainCamera, &clear_color, rwCAMERACLEARIMAGE );
}
