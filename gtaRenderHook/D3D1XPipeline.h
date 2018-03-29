#ifndef D3D1XPipeline_h__
#define D3D1XPipeline_h__
class CD3D1XShader;
// Direct3D Pipeline base class
class CD3D1XPipeline
{
public:
	CD3D1XPipeline(std::string pipeName);
	~CD3D1XPipeline();
protected:
	void DrawIndexed(UINT, UINT, UINT);
	// Base pixel shader ptr.
	CD3D1XShader*		m_pVS		= nullptr;
	// Base vertex shader ptr.
	CD3D1XShader*		m_pPS		= nullptr;
	// Pipeline name.
	std::string			m_sPipeName = "D3D1XPipeline";
};
#endif