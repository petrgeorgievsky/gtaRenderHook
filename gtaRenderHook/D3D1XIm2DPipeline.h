#ifndef D3D1XIm2DPipeline_h__
#define D3D1XIm2DPipeline_h__
#include "D3D1XPipeline.h"
class CD3D1XIm2DPipeline:public CD3D1XPipeline
{
public:
	CD3D1XIm2DPipeline();
	~CD3D1XIm2DPipeline();

	void Draw(RwPrimitiveType prim, RwIm2DVertex* verticles, RwUInt32 vertexCount);
	void DrawIndexed(RwPrimitiveType prim, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwUInt32 numIndices);

private:
	ID3D11InputLayout*      m_pVertexLayout			= nullptr;
	ID3D11Buffer*           m_pVertexBuffer			= nullptr;
	ID3D11Buffer*           m_pIndexBuffer			= nullptr;
};
#endif // D3D1XIm2DPipeline_h__

