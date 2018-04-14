#pragma once
#include "DebugRenderObject.h"
#include "RwVectorMath.h"
class CD3D1XShader;
class DebugBBox :
	public DebugRenderObject
{
public:
	DebugBBox(RW::BBox bbox);
	DebugBBox(RW::BBox bbox, RW::Matrix rotationMatrix);
	~DebugBBox();
	void Render();
	static void Initialize();
	static void Shutdown();
private:
	RwMatrix m_WorldMatrix;
	// Base pixel shader ptr.
	static CD3D1XShader*		m_pVS;
	// Base vertex shader ptr.
	static CD3D1XShader*		m_pPS;
	static RwV3d					m_aVerticles[8];
	static USHORT					m_aIndices[24];
	static ID3D11InputLayout*      m_pVertexLayout;
	static ID3D11Buffer*           m_pVertexBuffer;
	static ID3D11Buffer*           m_pIndexBuffer;
};

