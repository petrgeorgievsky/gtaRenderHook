#include "stdafx.h"
#include "D3D1XGlobalShaderDefines.h"
CD3D1XGlobalShaderDefines* g_pGlobalShaderDefines;

CD3D1XGlobalShaderDefines::CD3D1XGlobalShaderDefines()
{
	Reset();
}

void CD3D1XGlobalShaderDefines::AddDefine(const std::string & defineName, const std::string & defineValue)
{

	m_aDefines.emplace_back( defineName , defineValue);
}


void CD3D1XGlobalShaderDefines::Reset()
{
	m_aDefines.clear();
}

CD3D1XShaderDefine::CD3D1XShaderDefine(const std::string & name, const std::string & def):m_sName{name},m_sDefinition{def}
{
}