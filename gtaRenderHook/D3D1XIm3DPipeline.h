
#ifndef D3D1XIm3DPipeline_h__
#define D3D1XIm3DPipeline_h__
#include "D3D1XShader.h"
#include "D3D1XPipeline.h"
struct Im3DDef
{
	float x, y, z;
	RwRGBA color;
	float u, v;
};
class CD3D1XIm3DPipeline: public CD3D1XPipeline
{
public:
	CD3D1XIm3DPipeline(CD3DRenderer* pRenderer);
	~CD3D1XIm3DPipeline();

	RwBool SubmitNode();
private:
	ID3D11InputLayout*      m_pVertexLayout = nullptr;
	ID3D11Buffer*           m_pVertexBuffer = nullptr;
	ID3D11Buffer*           m_pIndexBuffer = nullptr;
};
#endif // D3D1XIm3DPipeline_h__

