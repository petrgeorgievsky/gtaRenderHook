#pragma once
#include "D3D1XPipeline.h"
class CD3D1XShader;
class CShadowPipeline :
	public CD3D1XPipeline
{
public:
#ifndef DebuggingShaders
	CShadowPipeline(std::string pipeName);
#else
	CShadowPipeline(std::wstring pipeName);
#endif // !DebuggingShaders
	~CShadowPipeline();
protected:
	CD3D1XShader*		m_pShadowPS = nullptr;
};

