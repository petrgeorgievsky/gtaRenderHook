#include "ClearScreenSample.h"

void ClearScreenSample::CustomRender()
{
    m_nCurrentRed = m_nCurrentRed + m_nCurrentRedIncr;
    // If we reached 100% pink or 100% blue we'll change increment direction
    if (m_nCurrentRed % 0xFF == 0)
        m_nCurrentRedIncr *= -1;
    RwRGBA clearColor = { m_nCurrentRed, 0, 255, 255 };
    RHEngine::g_pRWRenderEngine->CameraClear(m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE);
}