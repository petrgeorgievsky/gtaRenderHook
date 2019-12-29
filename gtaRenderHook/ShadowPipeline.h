#pragma once
#include "D3D1XPipeline.h"
class CD3D1XShader;
// TODO: Rethink pipelines completly, they shouldn't work the way it is right now
class CShadowPipeline :
    public CD3D1XPipeline
{
public:
#ifndef DebuggingShaders
    CShadowPipeline( const std::string &pipeName );
#else
    CShadowPipeline( const std::wstring &pipeName );
#endif // !DebuggingShaders
    ~CShadowPipeline();
    void SetShadowPipeShader( bool textured );
protected:
    CD3D1XShader*		m_pShadowPS = nullptr;
    CD3D1XShader*		m_pShadowPS_NoTex = nullptr;
};

