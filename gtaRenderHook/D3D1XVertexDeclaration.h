#ifndef D3D1XVertexDeclaration_h__
#define D3D1XVertexDeclaration_h__
// TODO: Implement this class in existing code.
class CD3DRenderer;
class CD3D1XShader;
class CD3D1XVertexDeclaration
{
public:
	CD3D1XVertexDeclaration(CD3DRenderer* pRenderer,CD3D1XShader* pVS,UINT flags);
	~CD3D1XVertexDeclaration();
	std::vector<D3D11_INPUT_ELEMENT_DESC>	&getElementInfo()	{ return m_elements; }
	UINT									&getStride()		{ return m_stride; }
private:
	CD3DRenderer*							m_pRenderer		= nullptr;
	ID3D11InputLayout*						m_inputLayout	= nullptr;
	std::vector<D3D11_INPUT_ELEMENT_DESC>	m_elements;
	UINT									m_inputInfo,
											m_stride;
};
#endif D3D1XVertexDeclaration_h__
