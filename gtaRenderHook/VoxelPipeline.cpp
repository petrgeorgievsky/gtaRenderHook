#include "stdafx.h"
#include "VoxelPipeline.h"
#include "D3D1XShader.h"


#ifndef DebuggingShaders
CVoxelPipeline::CVoxelPipeline(std::string pipeName,bool useVoxelPipe):useVoxelPipe{ useVoxelPipe }
#else
CVoxelPipeline::CVoxelPipeline(std::wstring pipeName)
#endif // !DebuggingShaders
{
#ifndef DebuggingShaders
	std::string shaderPath = "shaders/" + pipeName + ".hlsl";
#else
	std::wstring shaderPath = L"shaders/" + pipeName + L".hlsl";
#endif // !DebuggingShaders
	if (useVoxelPipe) {
		m_pVoxelPS = new CD3D1XPixelShader(shaderPath.c_str(), "VoxelPS");
		m_pVoxelEmmissivePS = new CD3D1XPixelShader(shaderPath.c_str(), "VoxelEmmissivePS");
		m_pVoxelVS = new CD3D1XVertexShader(shaderPath.c_str(), "VoxelVS");
		m_pVoxelGS = new CD3D1XGeometryShader(shaderPath.c_str(), "VoxelGS");
	}
}


CVoxelPipeline::~CVoxelPipeline()
{
	if (useVoxelPipe) {
		delete m_pVoxelPS;
		delete m_pVoxelEmmissivePS;
		delete m_pVoxelVS;
		delete m_pVoxelGS;
	}
}
