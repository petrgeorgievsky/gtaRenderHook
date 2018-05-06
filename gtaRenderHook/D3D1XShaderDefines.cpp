// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XShaderDefines.h"
CD3D1XShaderDefineList* g_pGlobalShaderDefines;

CD3D1XShaderDefineList::CD3D1XShaderDefineList()
{
	Reset();
}

void CD3D1XShaderDefineList::AddDefine(const std::string & defineName, const std::string & defineValue)
{
	m_aDefines.emplace_back( defineName , defineValue);
}


void CD3D1XShaderDefineList::Reset()
{
	m_aDefines.clear();
}

CD3D1XShaderDefine::CD3D1XShaderDefine(const std::string & name, const std::string & def):m_sName{name},m_sDefinition{def}
{
}