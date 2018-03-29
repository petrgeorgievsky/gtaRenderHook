#include "stdafx.h"
#include "DeferredPipeline.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"

CDeferredPipeline::CDeferredPipeline(std::string pipeName, bool useVoxelPipe) : CShadowPipeline{pipeName}, CVoxelPipeline{ pipeName,useVoxelPipe }
{
	std::string shaderPath = "shaders/" + m_sPipeName + ".hlsl";
	m_pDeferredPS = new CD3D1XPixelShader(shaderPath, "DeferredPS");
}


CDeferredPipeline::~CDeferredPipeline()
{
	delete m_pDeferredPS;
}
