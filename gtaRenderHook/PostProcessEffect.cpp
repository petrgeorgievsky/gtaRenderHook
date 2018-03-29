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
