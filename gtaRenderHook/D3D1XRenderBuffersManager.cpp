#include "stdafx.h"
#include "D3D1XRenderBuffersManager.h"
#include "D3DRenderer.h"
#include "RwD3D1XEngine.h"
#include "CDebug.h"


CD3D1XRenderBuffersManager::CD3D1XRenderBuffersManager()
{
	m_MaterialBuffer = {};
	auto pd3dDevice = GET_D3D_DEVICE;
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MatrixBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	if (FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pMatrixCB)))
		g_pDebug->printError("Failed to create matrix constant buffer.");
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	if (FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pMaterialCB)))
		g_pDebug->printError("Failed to create material constant buffer");
	ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
	//matr
	pImmediateContext->VSSetConstantBuffers(0, 1, &m_pMatrixCB);
	pImmediateContext->DSSetConstantBuffers(0, 1, &m_pMatrixCB);
	pImmediateContext->GSSetConstantBuffers(0, 1, &m_pMatrixCB);
	pImmediateContext->PSSetConstantBuffers(0, 1, &m_pMatrixCB);
	pImmediateContext->CSSetConstantBuffers(0, 1, &m_pMatrixCB);
	//mat
	pImmediateContext->PSSetConstantBuffers(2, 1, &m_pMaterialCB);
}


CD3D1XRenderBuffersManager::~CD3D1XRenderBuffersManager()
{
	if (m_pMatrixCB) {
		m_pMatrixCB->Release();
		m_pMatrixCB = nullptr;
	}
	if (m_pMaterialCB) {
		m_pMaterialCB->Release();
		m_pMaterialCB = nullptr;
	}
}

void CD3D1XRenderBuffersManager::UpdateViewProjMatricles(RwMatrix & view, RwMatrix & proj)
{
	m_MatrixBuffer.mView = view;
	m_MatrixBuffer.mProjection = proj;
	RwMatrixInvert(&m_MatrixBuffer.mInvView, &view);
	RwMatrix ViewProj = {};
	/*DirectX::XMMATRIX viewMat = {	view.right.x,view.right.y,view.right.z,static_cast<RwReal>(view.flags),
									view.up.x ,view.up.y ,view.up.z,static_cast<RwReal>(view.pad1),
									view.at.x ,view.at.y ,view.at.z,static_cast<RwReal>(view.pad2),
									view.pos.x ,view.pos.y ,view.pos.z, static_cast<RwReal>(view.pad3) };
	//viewMat=DirectX::XMMatrixTranspose(viewMat);
	DirectX::XMMATRIX projMat = { proj.right.x,proj.right.y,proj.right.z,static_cast<RwReal>(proj.flags),
		proj.up.x ,proj.up.y ,proj.up.z,static_cast<RwReal>(proj.pad1),
		proj.at.x ,proj.at.y ,proj.at.z,static_cast<RwReal>(proj.pad2),
		proj.pos.x ,proj.pos.y ,proj.pos.z, static_cast<RwReal>(proj.pad3) };
	//projMat = DirectX::XMMatrixTranspose(projMat);
	auto vp = DirectX::XMMatrixMultiply(viewMat, projMat);
	m_MatrixBuffer.mInvViewProj =*(RwMatrix*)&DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, vp));*/
	
	_RwMatrixMultiply(&ViewProj, &view, &proj);
	RwMatrixInvert(&m_MatrixBuffer.mInvViewProj, &ViewProj);
	
}

void CD3D1XRenderBuffersManager::UpdateViewMatrix(RwMatrix & view)
{
	m_MatrixBuffer.mView = view;
	RwMatrixInvert(&m_MatrixBuffer.mInvView, &view);
	RwMatrix ViewProj = {};
	_RwMatrixMultiply(&ViewProj, &view, &m_MatrixBuffer.mProjection);
	RwMatrixInvert(&m_MatrixBuffer.mInvViewProj, &ViewProj);
}

void CD3D1XRenderBuffersManager::UpdateViewMatrixLookAtDX(DirectX::XMMATRIX * view)
{
	m_MatrixBuffer.mView = *(RwMatrix*)((void*)view);
	RwMatrixInvert(&m_MatrixBuffer.mInvView, (RwMatrix*)view);
	RwMatrix ViewProj = {};
	_RwMatrixMultiply(&ViewProj, (RwMatrix*)view, &m_MatrixBuffer.mProjection);
	RwMatrixInvert(&m_MatrixBuffer.mInvViewProj, &ViewProj);
}

void CD3D1XRenderBuffersManager::UpdateViewMatrixLookAtProjDX(DirectX::XMMATRIX * view, DirectX::XMMATRIX * proj)
{
	m_MatrixBuffer.mView = *(RwMatrix*)((void*)view);
	m_MatrixBuffer.mProjection = *(RwMatrix*)((void*)proj);
	RwMatrixInvert(&m_MatrixBuffer.mInvView, (RwMatrix*)view);
	RwMatrix ViewProj = {};
	_RwMatrixMultiply(&ViewProj, (RwMatrix*)view, (RwMatrix*)proj);
	RwMatrixInvert(&m_MatrixBuffer.mInvViewProj, &ViewProj);
}

void CD3D1XRenderBuffersManager::UpdateWorldMatrix(RwMatrix *ltm)
{
	m_pCurrentWorldMatrix = ltm;
	m_MatrixBuffer.mWorld.right.x = ltm->right.x;
	m_MatrixBuffer.mWorld.right.y = ltm->right.y;
	m_MatrixBuffer.mWorld.right.z = ltm->right.z;
	m_MatrixBuffer.mWorld.up.x = ltm->up.x;
	m_MatrixBuffer.mWorld.up.y = ltm->up.y;
	m_MatrixBuffer.mWorld.up.z = ltm->up.z;
	m_MatrixBuffer.mWorld.at.x = ltm->at.x;
	m_MatrixBuffer.mWorld.at.y = ltm->at.y;
	m_MatrixBuffer.mWorld.at.z = ltm->at.z;
	m_MatrixBuffer.mWorld.pos.x = ltm->pos.x;
	m_MatrixBuffer.mWorld.pos.y = ltm->pos.y;
	m_MatrixBuffer.mWorld.pos.z = ltm->pos.z;
	m_MatrixBuffer.mWorld.flags = 0;
	m_MatrixBuffer.mWorld.pad1 = 0;
	m_MatrixBuffer.mWorld.pad2 = 0;
	m_MatrixBuffer.mWorld.pad3 = 0x3F800000;
}

void CD3D1XRenderBuffersManager::SetMatrixBuffer()
{
	if (m_pCurrentWorldMatrix != m_pOldWorldMatrix) {
		ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->UpdateSubresource(m_pMatrixCB, 0, nullptr, &m_MatrixBuffer, 0, 0);
		m_pOldWorldMatrix = m_pCurrentWorldMatrix;
	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialDiffuseColor(RwRGBA &color)
{
	if (RWRGBALONG(	(byte)(m_MaterialBuffer.diffuseColor.red*255), (byte)(m_MaterialBuffer.diffuseColor.green * 255), 
					(byte)(m_MaterialBuffer.diffuseColor.blue * 255), (byte)(m_MaterialBuffer.diffuseColor.alpha * 255))
		!= RWRGBALONG(color.red, color.green, color.blue, color.alpha))
	{
		m_MaterialBuffer.diffuseColor = RwRGBAReal{ (float)color.red/255.0f, (float)color.green / 255.0f, (float)color.blue / 255.0f, (float)color.alpha / 255.0f };
		ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->UpdateSubresource(m_pMaterialCB, 0, nullptr, &m_MaterialBuffer, 0, 0);
		
	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialEmmissiveColor(RwRGBA & color)
{
	if (RWRGBALONG((byte)(m_MaterialBuffer.diffuseColor.red * 255), (byte)(m_MaterialBuffer.diffuseColor.green * 255),
		(byte)(m_MaterialBuffer.diffuseColor.blue * 255), (byte)(m_MaterialBuffer.diffuseColor.alpha * 255))
		!= RWRGBALONG(color.red, color.green, color.blue, color.alpha))
	{
		m_MaterialBuffer.diffuseColor = RwRGBAReal{ (float)color.red / 255.0f*16.0f, (float)color.green / 255.0f*16.0f, (float)color.blue / 255.0f*16.0f, (float)color.alpha / 255.0f };
		ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->UpdateSubresource(m_pMaterialCB, 0, nullptr, &m_MaterialBuffer, 0, 0);

	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialSpecularInt(float & intensity)
{
	if (abs(m_MaterialBuffer.specularIntensity - intensity)> 0.001) {
		m_MaterialBuffer.specularIntensity = intensity;
		ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->UpdateSubresource(m_pMaterialCB, 0, nullptr, &m_MaterialBuffer, 0, 0);
	}
}

void CD3D1XRenderBuffersManager::UpdateMaterialGlossiness(float & intensity)
{
	if (abs(m_MaterialBuffer.glossiness - intensity)> 0.001) {
		m_MaterialBuffer.glossiness = intensity;
		ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->UpdateSubresource(m_pMaterialCB, 0, nullptr, &m_MaterialBuffer, 0, 0);
	}
}

void CD3D1XRenderBuffersManager::UpdateHasSpecTex(const int & hastex)
{
	if (m_MaterialBuffer.hasSpecTex!= hastex) {
		m_MaterialBuffer.hasSpecTex = hastex;
		ID3D11DeviceContext* pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->UpdateSubresource(m_pMaterialCB, 0, nullptr, &m_MaterialBuffer, 0, 0);
	}
}
