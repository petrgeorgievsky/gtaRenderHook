#include "stdafx.h"
#include "D3D1XPipeline.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"

#ifndef DebuggingShaders
CD3D1XPipeline::CD3D1XPipeline(CD3DRenderer* pRenderer, std::string pipeName): m_pRenderer{ pRenderer }, m_sPipeName{pipeName}
#else
CD3D1XPipeline::CD3D1XPipeline(CD3DRenderer* pRenderer, std::wstring pipeName) : m_pRenderer{ pRenderer }, m_sPipeName{ pipeName }
#endif // !DebuggingShaders
{
#ifndef DebuggingShaders
	std::string shaderPath = "shaders/" + m_sPipeName + ".fx";
#else
	std::wstring shaderPath = L"shaders/" + m_sPipeName + L".fx";
#endif // !DebuggingShaders

	m_pPS = new CD3D1XShader(m_pRenderer, RwD3D1XShaderType::PS, shaderPath.c_str(), "PS");
	m_pVS = new CD3D1XShader(m_pRenderer, RwD3D1XShaderType::VS, shaderPath.c_str(), "VS");
}


CD3D1XPipeline::~CD3D1XPipeline()
{
	delete m_pPS;
	delete m_pVS;
}
