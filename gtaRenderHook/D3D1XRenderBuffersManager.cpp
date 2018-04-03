#include "stdafx.h"
#include "D3D1XRenderBuffersManager.h"
#include "D3D1XStateManager.h"
#include "D3DRenderer.h"
#include "RwD3D1XEngine.h"
#include "CDebug.h"


CD3D1XRenderBuffersManager::CD3D1XRenderBuffersManager()
{
	m_pPerFrameMatrixBuffer = new CD3D1XConstantBuffer<PerFrameMatrixBuffer>();
	m_pPerObjectMatrixBuffer = new CD3D1XConstantBuffer<PerObjectMatrixBuffer>();
	m_pPerMaterialBuffer = new CD3D1XConstantBuffer<MaterialBuffer>();

	g_pStateMgr->SetConstantBufferVS(m_pPerFrameMatrixBuffer, 1);
	g_pStateMgr->SetConstantBufferPS(m_pPerFrameMatrixBuffer, 1);
	g_pStateMgr->SetConstantBufferCS(m_pPerFrameMatrixBuffer, 1);
	g_pStateMgr->SetConstantBufferDS(m_pPerFrameMatrixBuffer, 1);
	g_pStateMgr->SetConstantBufferVS(m_pPerObjectMatrixBuffer, 2);
	g_pStateMgr->SetConstantBufferPS(m_pPerMaterialBuffer, 3);
}


CD3D1XRenderBuffersManager::~CD3D1XRenderBuffersManager()
{
	delete m_pPerFrameMatrixBuffer;
	delete m_pPerObjectMatrixBuffer;
	delete m_pPerMaterialBuffer;
}

void CD3D1XRenderBuffersManager::UpdateViewProjMatricles(RwMatrix & view, RwMatrix & proj)
{
	m_pPerFrameMatrixBuffer->data.mView = view;
	m_pPerFrameMatrixBuffer->data.mProjection = proj;

	RwMatrixInvert(&m_pPerFrameMatrixBuffer->data.mInvView, &view);	
	Multipy4x4Matrices((RwGraphicsMatrix*)&m_pPerFrameMatrixBuffer->data.mViewProjection,
		(RwGraphicsMatrix*)&m_pPerFrameMatrixBuffer->data.mView, (RwGraphicsMatrix*)&m_pPerFrameMatrixBuffer->data.mProjection);
	Inverse4x4Matrix((RwGraphicsMatrix*)&m_pPerFrameMatrixBuffer->data.mInvViewProj, 
		(RwGraphicsMatrix*)&m_pPerFrameMatrixBuffer->data.mViewProjection);
	m_pPerFrameMatrixBuffer->Update();
	
}

void CD3D1XRenderBuffersManager::UpdateViewMatrix(RwMatrix & view)
{
	m_pPerFrameMatrixBuffer->data.mView = view;

	RwMatrixInvert(&m_pPerFrameMatrixBuffer->data.mInvView, &view);
	_RwMatrixMultiply(&m_pPerFrameMatrixBuffer->data.mViewProjection, &view, &m_pPerFrameMatrixBuffer->data.mProjection);
	RwMatrixInvert(&m_pPerFrameMatrixBuffer->data.mInvViewProj, &m_pPerFrameMatrixBuffer->data.mViewProjection);
	m_pPerFrameMatrixBuffer->Update();
}

void CD3D1XRenderBuffersManager::UpdateWorldMatrix(RwMatrix *ltm)
{
	m_pCurrentWorldMatrix = ltm;
	m_pPerObjectMatrixBuffer->data.mWorld.right.x = ltm->right.x;
	m_pPerObjectMatrixBuffer->data.mWorld.right.y = ltm->right.y;
	m_pPerObjectMatrixBuffer->data.mWorld.right.z = ltm->right.z;
	m_pPerObjectMatrixBuffer->data.mWorld.up.x = ltm->up.x;
	m_pPerObjectMatrixBuffer->data.mWorld.up.y = ltm->up.y;
	m_pPerObjectMatrixBuffer->data.mWorld.up.z = ltm->up.z;
	m_pPerObjectMatrixBuffer->data.mWorld.at.x = ltm->at.x;
	m_pPerObjectMatrixBuffer->data.mWorld.at.y = ltm->at.y;
	m_pPerObjectMatrixBuffer->data.mWorld.at.z = ltm->at.z;
	m_pPerObjectMatrixBuffer->data.mWorld.pos.x = ltm->pos.x;
	m_pPerObjectMatrixBuffer->data.mWorld.pos.y = ltm->pos.y;
	m_pPerObjectMatrixBuffer->data.mWorld.pos.z = ltm->pos.z;
	m_pPerObjectMatrixBuffer->data.mWorld.flags = 0;
	m_pPerObjectMatrixBuffer->data.mWorld.pad1 = 0;
	m_pPerObjectMatrixBuffer->data.mWorld.pad2 = 0;
	m_pPerObjectMatrixBuffer->data.mWorld.pad3 = 0x3F800000;
}

void CD3D1XRenderBuffersManager::Multipy4x4Matrices(RwGraphicsMatrix * res, RwGraphicsMatrix * a, RwGraphicsMatrix * b)
{
	// renderware bullshit prevents from normal multiplication of homogenous matrices, so here is implementation to do that
	res->m[0].x =	a->m[0].x * b->m[0].x +
					a->m[0].y * b->m[1].x 	+
					a->m[0].z * b->m[2].x	+
					a->m[0].w * b->m[3].x;

	res->m[0].y = a->m[0].x * b->m[0].y +
				a->m[0].y * b->m[1].y	+
				a->m[0].z * b->m[2].y	+
				a->m[0].w * b->m[3].y;

	res->m[0].z = a->m[0].x * b->m[0].z +
				a->m[0].y * b->m[1].z +
				a->m[0].z * b->m[2].z +
				a->m[0].w * b->m[3].z;

	res->m[0].w = a->m[0].x * b->m[0].w +
				 a->m[0].y * b->m[1].w +
				 a->m[0].z * b->m[2].w +
				 a->m[0].w * b->m[3].w;
	// 2nd column
	res->m[1].x =	a->m[1].x * b->m[0].x +
					a->m[1].y * b->m[1].x +
					a->m[1].z * b->m[2].x +
					a->m[1].w * b->m[3].x;

	res->m[1].y = a->m[1].x * b->m[0].y +
				a->m[1].y * b->m[1].y +
				a->m[1].z * b->m[2].y +
				a->m[1].w * b->m[3].y;

	res->m[1].z = a->m[1].x * b->m[0].z +
				a->m[1].y * b->m[1].z +
				a->m[1].z * b->m[2].z +
				(a->m[1].w*b->m[3].z);

	res->m[1].w =	a->m[1].x * b->m[0].w +
					a->m[1].y * b->m[1].w +
					a->m[1].z * b->m[2].w +
					a->m[1].w * b->m[3].w;
	// 3rd column
	res->m[2].x =	a->m[2].x * b->m[0].x +
					a->m[2].y * b->m[1].x +
					a->m[2].z * b->m[2].x +
					a->m[2].w * b->m[3].x;

	res->m[2].y = a->m[2].x * b->m[0].y +
				a->m[2].y * b->m[1].y +
				a->m[2].z * b->m[2].y +
				a->m[2].w * b->m[3].y;

	res->m[2].z = a->m[2].x * b->m[0].z +
				a->m[2].y * b->m[1].z +
				a->m[2].z * b->m[2].z +
				a->m[2].w * b->m[3].z;

	res->m[2].w =	a->m[2].x * b->m[0].w +
					a->m[2].y * b->m[1].w +
					a->m[2].z * b->m[2].w +
					a->m[2].w*b->m[3].w;
	// 4th column
	res->m[3].x = a->m[3].x * b->m[0].x +
					a->m[3].y * b->m[1].x +
					a->m[3].z * b->m[2].x +
					a->m[3].w*b->m[3].x;

	res->m[3].y = a->m[3].x * b->m[0].y +
					a->m[3].y * b->m[1].y +
					a->m[3].z * b->m[2].y +
					a->m[3].w * b->m[3].y;

	res->m[3].z = a->m[3].x * b->m[0].z +
				a->m[3].y * b->m[1].z +
				a->m[3].z * b->m[2].z +
				a->m[3].w*b->m[3].z;

	res->m[3].w = a->m[3].x * b->m[0].w +
					a->m[3].y * b->m[1].w +
					a->m[3].z * b->m[2].w +
					a->m[3].w*b->m[3].w;
}

void CD3D1XRenderBuffersManager::Inverse4x4Matrix(RwGraphicsMatrix * a, RwGraphicsMatrix * b)
{
	// adapted code from here(https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix)
	float det;
	int i;

	a->m[0].x = b->m[1].y * b->m[2].z * b->m[3].w -
		b->m[1].y * b->m[2].w * b->m[3].z -
		b->m[2].y * b->m[1].z * b->m[3].w +
		b->m[2].y * b->m[1].w * b->m[3].z +
		b->m[3].y * b->m[1].z * b->m[2].w -
		b->m[3].y * b->m[1].w * b->m[2].z;

	a->m[1].x = -b->m[1].x * b->m[2].z * b->m[3].w +
		b->m[1].x * b->m[2].w * b->m[3].z +
		b->m[2].x * b->m[1].z * b->m[3].w -
		b->m[2].x * b->m[1].w * b->m[3].z -
		b->m[3].x * b->m[1].z * b->m[2].w +
		b->m[3].x * b->m[1].w * b->m[2].z;

	a->m[2].x = b->m[1].x * b->m[2].y * b->m[3].w -
		b->m[1].x * b->m[2].w * b->m[3].y -
		b->m[2].x * b->m[1].y * b->m[3].w +
		b->m[2].x * b->m[1].w * b->m[3].y +
		b->m[3].x * b->m[1].y * b->m[2].w -
		b->m[3].x * b->m[1].w * b->m[2].y;

	a->m[3].x = -b->m[1].x * b->m[2].y * b->m[3].z +
		b->m[1].x * b->m[2].z * b->m[3].y +
		b->m[2].x * b->m[1].y * b->m[3].z -
		b->m[2].x * b->m[1].z * b->m[3].y -
		b->m[3].x * b->m[1].y * b->m[2].z +
		b->m[3].x * b->m[1].z * b->m[2].y;

	a->m[0].y = -b->m[0].y * b->m[2].z * b->m[3].w +
		b->m[0].y * b->m[2].w * b->m[3].z +
		b->m[2].y * b->m[0].z * b->m[3].w -
		b->m[2].y * b->m[0].w * b->m[3].z -
		b->m[3].y * b->m[0].z * b->m[2].w +
		b->m[3].y * b->m[0].w * b->m[2].z;

	a->m[1].y = b->m[0].x * b->m[2].z * b->m[3].w -
		b->m[0].x * b->m[2].w * b->m[3].z -
		b->m[2].x * b->m[0].z * b->m[3].w +
		b->m[2].x * b->m[0].w * b->m[3].z +
		b->m[3].x * b->m[0].z * b->m[2].w -
		b->m[3].x * b->m[0].w * b->m[2].z;

	a->m[2].y = -b->m[0].x * b->m[2].y * b->m[3].w +
		b->m[0].x * b->m[2].w * b->m[3].y +
		b->m[2].x * b->m[0].y * b->m[3].w -
		b->m[2].x * b->m[0].w * b->m[3].y -
		b->m[3].x * b->m[0].y * b->m[2].w +
		b->m[3].x * b->m[0].w * b->m[2].y;

	a->m[3].y = b->m[0].x * b->m[2].y * b->m[3].z -
		b->m[0].x * b->m[2].z * b->m[3].y -
		b->m[2].x * b->m[0].y * b->m[3].z +
		b->m[2].x * b->m[0].z * b->m[3].y +
		b->m[3].x * b->m[0].y * b->m[2].z -
		b->m[3].x * b->m[0].z * b->m[2].y;

	a->m[0].z = b->m[0].y * b->m[1].z * b->m[3].w -
		b->m[0].y * b->m[1].w * b->m[3].z -
		b->m[1].y * b->m[0].z * b->m[3].w +
		b->m[1].y * b->m[0].w * b->m[3].z +
		b->m[3].y * b->m[0].z * b->m[1].w -
		b->m[3].y * b->m[0].w * b->m[1].z;

	a->m[1].z = -b->m[0].x * b->m[1].z * b->m[3].w +
		b->m[0].x * b->m[1].w * b->m[3].z +
		b->m[1].x * b->m[0].z * b->m[3].w -
		b->m[1].x * b->m[0].w * b->m[3].z -
		b->m[3].x * b->m[0].z * b->m[1].w +
		b->m[3].x * b->m[0].w * b->m[1].z;

	a->m[2].z = b->m[0].x * b->m[1].y * b->m[3].w -
		b->m[0].x * b->m[1].w * b->m[3].y -
		b->m[1].x * b->m[0].y * b->m[3].w +
		b->m[1].x * b->m[0].w * b->m[3].y +
		b->m[3].x * b->m[0].y * b->m[1].w -
		b->m[3].x * b->m[0].w * b->m[1].y;

	a->m[3].z = -b->m[0].x * b->m[1].y * b->m[3].z +
		b->m[0].x * b->m[1].z * b->m[3].y +
		b->m[1].x * b->m[0].y * b->m[3].z -
		b->m[1].x * b->m[0].z * b->m[3].y -
		b->m[3].x * b->m[0].y * b->m[1].z +
		b->m[3].x * b->m[0].z * b->m[1].y;

	a->m[0].w = -b->m[0].y * b->m[1].z * b->m[2].w +
		b->m[0].y * b->m[1].w * b->m[2].z +
		b->m[1].y * b->m[0].z * b->m[2].w -
		b->m[1].y * b->m[0].w * b->m[2].z -
		b->m[2].y * b->m[0].z * b->m[1].w +
		b->m[2].y * b->m[0].w * b->m[1].z;

	a->m[1].w = b->m[0].x * b->m[1].z * b->m[2].w -
		b->m[0].x * b->m[1].w * b->m[2].z -
		b->m[1].x * b->m[0].z * b->m[2].w +
		b->m[1].x * b->m[0].w * b->m[2].z +
		b->m[2].x * b->m[0].z * b->m[1].w -
		b->m[2].x * b->m[0].w * b->m[1].z;

	a->m[2].w = -b->m[0].x * b->m[1].y * b->m[2].w +
		b->m[0].x * b->m[1].w * b->m[2].y +
		b->m[1].x * b->m[0].y * b->m[2].w -
		b->m[1].x * b->m[0].w * b->m[2].y -
		b->m[2].x * b->m[0].y * b->m[1].w +
		b->m[2].x * b->m[0].w * b->m[1].y;

	a->m[3].w = b->m[0].x * b->m[1].y * b->m[2].z -
		b->m[0].x * b->m[1].z * b->m[2].y -
		b->m[1].x * b->m[0].y * b->m[2].z +
		b->m[1].x * b->m[0].z * b->m[2].y +
		b->m[2].x * b->m[0].y * b->m[1].z -
		b->m[2].x * b->m[0].z * b->m[1].y;

	det = b->m[0].x * a->m[0].x + b->m[0].y * a->m[1].x + b->m[0].z * a->m[2].x + b->m[0].w * a->m[3].x;

	det = 1.0 / det;

	for (i = 0; i < 4; i++) {
		a->m[i].x = a->m[i].x  * det;
		a->m[i].y = a->m[i].y  * det;
		a->m[i].z = a->m[i].z  * det;
		a->m[i].w = a->m[i].w  * det;
	}

}

void CD3D1XRenderBuffersManager::Transpose4x4Matrix(RwGraphicsMatrix * a, RwGraphicsMatrix * b)
{
	a->m[0] = { b->m[0].x,b->m[1].x,b->m[2].x,b->m[3].x };
	a->m[1] = { b->m[0].y,b->m[1].y,b->m[2].y,b->m[3].y };
	a->m[2] = { b->m[0].z,b->m[1].z,b->m[2].z,b->m[3].z };
	a->m[3] = { b->m[0].w,b->m[1].w,b->m[2].w,b->m[3].w };
}

void CD3D1XRenderBuffersManager::SetMatrixBuffer()
{
	if (m_pCurrentWorldMatrix != m_pOldWorldMatrix) {
		m_pPerObjectMatrixBuffer->Update();
		m_pOldWorldMatrix = m_pCurrentWorldMatrix;
	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialDiffuseColor(RwRGBA &color)
{
	auto currentColor= m_pPerMaterialBuffer->data.diffuseColor;

	if (RWRGBALONG(	(byte)(currentColor.red*255), (byte)(currentColor.green * 255),
					(byte)(currentColor.blue * 255), (byte)(currentColor.alpha * 255))
		!= RWRGBALONG(color.red, color.green, color.blue, color.alpha))
	{
		m_pPerMaterialBuffer->data.diffuseColor = RwRGBAReal{ (float)color.red/255.0f, (float)color.green / 255.0f, (float)color.blue / 255.0f, (float)color.alpha / 255.0f };
		m_bMaterialBufferRequiresUpdate = true;
	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialEmmissiveColor(RwRGBA & color)
{
	auto currentColor = m_pPerMaterialBuffer->data.diffuseColor;
	if (RWRGBALONG((byte)(currentColor.red / 16.0f * 255),
		(byte)(currentColor.green / 16.0f * 255),
		(byte)(currentColor.blue / 16.0f * 255), 
		(byte)(currentColor.alpha / 16.0f * 255))
		!= RWRGBALONG(color.red, color.green, color.blue, color.alpha))
	{
		m_pPerMaterialBuffer->data.diffuseColor = RwRGBAReal{ (float)color.red / 255.0f*16.0f, 
			(float)color.green / 255.0f*16.0f,
			(float)color.blue / 255.0f*16.0f,
			(float)color.alpha / 255.0f };
		m_bMaterialBufferRequiresUpdate = true;
	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialSpecularInt(float & intensity)
{
	if (abs(m_pPerMaterialBuffer->data.specularIntensity - intensity)> 0.001) {
		m_pPerMaterialBuffer->data.specularIntensity = intensity;
		m_bMaterialBufferRequiresUpdate = true;
	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialGlossiness(float & intensity)
{
	if (abs(m_pPerMaterialBuffer->data.glossiness - intensity)> 0.001) {
		m_pPerMaterialBuffer->data.glossiness = intensity;
		m_bMaterialBufferRequiresUpdate = true;
	}
}

void CD3D1XRenderBuffersManager::UpdateHasSpecTex(const int & hastex)
{
	if (m_pPerMaterialBuffer->data.hasSpecTex!= hastex) {
		m_pPerMaterialBuffer->data.hasSpecTex = hastex;
		m_bMaterialBufferRequiresUpdate = true;
	}
}

void CD3D1XRenderBuffersManager::FlushMaterialBuffer()
{
	if (m_bMaterialBufferRequiresUpdate) {
		m_pPerMaterialBuffer->Update();
		m_bMaterialBufferRequiresUpdate = false;
	}
}
