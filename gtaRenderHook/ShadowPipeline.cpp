// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "ShadowPipeline.h"
#include "D3D1XShader.h"

#ifndef DebuggingShaders
CShadowPipeline::CShadowPipeline(const std::string &pipeName) : CD3D1XPipeline{ pipeName }
#else
CShadowPipeline::CShadowPipeline(const std::wstring &pipeName) :  m_sPipeName{ pipeName }
#endif // !DebuggingShaders
{
#ifndef DebuggingShaders
	std::string shaderPath = "shaders/" + m_sPipeName + ".hlsl";
#else
	std::wstring shaderPath = L"shaders/" + m_sPipeName + L".hlsl";
#endif // !DebuggingShaders

	m_pShadowPS = new CD3D1XPixelShader( shaderPath, "ShadowPS");
}


CShadowPipeline::~CShadowPipeline()
{
	delete m_pShadowPS;
}
