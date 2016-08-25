#pragma once
class CD3DRenderer;
class CD3D1XShader;
class CD3D1XPipeline
{
public:
#ifndef DebuggingShaders
	CD3D1XPipeline(CD3DRenderer* pRenderer, std::string pipeName);
#else
	CD3D1XPipeline(CD3DRenderer* pRenderer, std::wstring pipeName);
#endif // !DebuggingShaders
	~CD3D1XPipeline();
protected:
	CD3DRenderer*		m_pRenderer = nullptr;
	CD3D1XShader*		m_pVS		= nullptr;
	CD3D1XShader*		m_pPS		= nullptr;
#ifndef DebuggingShaders
	std::string			m_sPipeName = "D3D1XPipeline";
#else
	std::wstring		m_sPipeName = L"D3D1XPipeline";
#endif // !DebuggingShaders

};

