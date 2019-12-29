// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "DeferredPipeline.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"

CDeferredPipeline::CDeferredPipeline( const std::string &pipeName, bool useVoxelPipe ) : CShadowPipeline{ pipeName }, CVoxelPipeline{ pipeName,useVoxelPipe }
{
    std::string shaderPath = "shaders/" + m_sPipeName + ".hlsl";
    m_pDeferredPS = new CD3D1XPixelShader( shaderPath, "DeferredPS" );
}


CDeferredPipeline::~CDeferredPipeline()
{
    delete m_pDeferredPS;
}
