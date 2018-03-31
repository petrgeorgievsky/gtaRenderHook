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
	_RwMatrixMultiply(&m_pPerFrameMatrixBuffer->data.mViewProjection, &m_pPerFrameMatrixBuffer->data.mView, &m_pPerFrameMatrixBuffer->data.mProjection);
	RwMatrixInvert(&m_pPerFrameMatrixBuffer->data.mInvViewProj, &m_pPerFrameMatrixBuffer->data.mViewProjection);
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

void CD3D1XRenderBuffersManager::Multipy4x4Matrices(RwMatrix * res, RwMatrix * a, RwMatrix * b)
{
	// renderware bullshit prevents from normal multiplication of homogenous matrices, so here is implementation to do that
	auto matrix = DirectX::XMMatrixMultiplyTranspose((*(DirectX::XMMATRIX*)a), *((DirectX::XMMATRIX*)b));
	*res = (*(RwMatrix*)&matrix);
	return;

	res->right.x =	a->right.x * b->right.x + 
					a->right.y * b->up.x 	+
					a->right.z * b->at.x	+
					(static_cast<float>(a->flags)*b->pos.x);

	res->up.x = a->right.x * b->right.y +
				a->right.y * b->up.y	+
				a->right.z * b->at.y	+
				(static_cast<float>(a->flags)*b->pos.y);

	res->at.x = a->right.x * b->right.z +
				a->right.y * b->up.z +
				a->right.z * b->at.z +
				(static_cast<float>(a->flags)*b->pos.z);

	res->pos.x = a->right.x * static_cast<float>(b->flags) +
				 a->right.y * static_cast<float>(b->pad1) +
				 a->right.z * static_cast<float>(b->pad2) +
				 (static_cast<float>(a->flags)*static_cast<float>(b->pad3));
	// 2nd column
	res->right.y =	a->up.x * b->right.x +
					a->up.y * b->up.x +
					a->up.z * b->at.x +
					(static_cast<float>(a->pad1)*b->pos.x);

	res->up.y = a->up.x * b->right.y +
				a->up.y * b->up.y +
				a->up.z * b->at.y +
				(static_cast<float>(a->pad1)*b->pos.y);

	res->at.y = a->up.x * b->right.z +
				a->up.y * b->up.z +
				a->up.z * b->at.z +
				(static_cast<float>(a->pad1)*b->pos.z);

	res->pos.y =	a->up.x * static_cast<float>(b->flags) +
					a->up.y * static_cast<float>(b->pad1) +
					a->up.z * static_cast<float>(b->pad2) +
					(static_cast<float>(a->pad1)*static_cast<float>(b->pad3));
	// 3rd column
	res->right.z =	a->at.x * b->right.x +
					a->at.y * b->up.x +
					a->at.z * b->at.x +
					(static_cast<float>(a->pad2)*b->pos.x);

	res->up.z = a->at.x * b->right.y +
				a->at.y * b->up.y +
				a->at.z * b->at.y +
				(static_cast<float>(a->pad2)*b->pos.y);

	res->at.z = a->at.x * b->right.z +
				a->at.y * b->up.z +
				a->at.z * b->at.z +
				(static_cast<float>(a->pad2)*b->pos.z);

	res->pos.z =	a->at.x * static_cast<float>(b->flags) +
					a->at.y * static_cast<float>(b->pad1) +
					a->at.z * static_cast<float>(b->pad2) +
					(static_cast<float>(a->pad2)*static_cast<float>(b->pad3));
	// 4th column
	res->flags = static_cast<RwUInt32>(a->pos.x * b->right.x +
					a->pos.y * b->up.x +
					a->pos.z * b->at.x +
					(static_cast<float>(a->pad3)*b->pos.x));

	res->pad1 = static_cast<RwUInt32>(a->pos.x * b->right.y +
				a->pos.y * b->up.y +
				a->pos.z * b->at.y +
				(static_cast<float>(a->pad3)*b->pos.y));

	res->pad2 = static_cast<RwUInt32>(a->pos.x * b->right.z +
				a->pos.y * b->up.z +
				a->pos.z * b->at.z +
				(static_cast<float>(a->pad3)*b->pos.z));

	res->pad3 = static_cast<RwUInt32>(a->pos.x * static_cast<float>(b->flags) +
					a->pos.y * static_cast<float>(b->pad1) +
					a->pos.z * static_cast<float>(b->pad2) +
					(static_cast<float>(a->pad3)*static_cast<float>(b->pad3)));
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
