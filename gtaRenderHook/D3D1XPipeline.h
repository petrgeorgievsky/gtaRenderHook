#ifndef D3D1XPipeline_h__
#define D3D1XPipeline_h__
class CD3D1XShader;
/*!
	\class CD3D1XPipeline
	\brief Base D3D pipeline class.

	This class represents rendering alghorithm for one object of some type.
	If you want to add custom D3D pipeline for some rendering effect, use this class as base.
*/
class CD3D1XPipeline
{
public:
	/*!
		Initializes pixel and vertex shaders
	*/
	CD3D1XPipeline(const std::string &pipeName);
	/*!
		Releases shader resources
	*/
	~CD3D1XPipeline();
protected:
	// Base pixel shader ptr.
	CD3D1XShader*		m_pVS		= nullptr;
	// Base vertex shader ptr.
	CD3D1XShader*		m_pPS		= nullptr;
	// Pipeline name.
	std::string			m_sPipeName = "D3D1XPipeline";
};
#endif