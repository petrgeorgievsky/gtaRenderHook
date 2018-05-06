// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "PostProcessEffect.h"


CPostProcessEffect::CPostProcessEffect(std::string name):m_effectName{name}
{
}


CPostProcessEffect::~CPostProcessEffect()
{
}

void CPostProcessEffect::Render(RwRaster * inputRaster)
{
}
