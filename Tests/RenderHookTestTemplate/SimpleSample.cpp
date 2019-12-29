#include "SimpleSample.h"

void SimpleSample::CustomRender()
{
    RwRGBA clearColor = { 128, 128, 255, 255 };
    RHEngine::g_pRWRenderEngine->CameraClear( m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE );
}