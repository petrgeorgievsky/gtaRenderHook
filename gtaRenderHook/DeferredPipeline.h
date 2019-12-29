#pragma once
#include "D3D1XPipeline.h"
#include "ShadowPipeline.h"
#include "VoxelPipeline.h"
class CD3DRenderer;
class CD3D1XShader;
class CDeferredPipeline :public CShadowPipeline, public CVoxelPipeline
{
public:
#ifndef DebuggingShaders
    CDeferredPipeline( const std::string &pipeName, bool useVoxelPipe = true );
#else
    CDeferredPipeline( const std::wstring &pipeName );
#endif // !DebuggingShaders
    ~CDeferredPipeline();
protected:
    CD3D1XShader*		m_pDeferredPS = nullptr;
};

