// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XPipeline.h"
#include "D3D1XShader.h"

CD3D1XPipeline::CD3D1XPipeline(const std::string& pipe_name) : m_sPipeName{ pipe_name }
{
	const auto shader_path = "shaders/" + m_sPipeName + ".hlsl";

	m_pPS = new CD3D1XPixelShader(shader_path, "PS");
	m_pVS = new CD3D1XVertexShader(shader_path, "VS");
}

CD3D1XPipeline::~CD3D1XPipeline()
{
	delete m_pPS;
	delete m_pVS;
}