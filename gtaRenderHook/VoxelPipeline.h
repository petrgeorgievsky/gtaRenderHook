#pragma once
#include "D3D1XPipeline.h"
class CD3D1XShader;
class CVoxelPipeline
{
public:
#ifndef DebuggingShaders
    CVoxelPipeline( std::string pipeName, bool useVoxelPipe );
#else
    CVoxelPipeline( std::wstring pipeName );
#endif // !DebuggingShaders
    ~CVoxelPipeline();
protected:
    CD3D1XShader*		m_pVoxelPS = nullptr;
    CD3D1XShader*		m_pVoxelEmmissivePS = nullptr;
    CD3D1XShader*		m_pVoxelGS = nullptr;
    CD3D1XShader*		m_pVoxelVS = nullptr;
    bool m_bUseVoxelPipe = true;
};

