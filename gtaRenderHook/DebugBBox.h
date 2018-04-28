#pragma once
#include "DebugRenderObject.h"
#include "RwVectorMath.h"
#include "D3D1XVertexBuffer.h"
#include "D3D1XIndexBuffer.h"
#include "D3D1XVertexDeclaration.h"
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
	static CD3D1XVertexDeclaration*		m_pVertexDecl;
	static CD3D1XVertexBuffer*			m_pVertexBuffer;
	static CD3D1XIndexBuffer*			m_pIndexBuffer;
};

