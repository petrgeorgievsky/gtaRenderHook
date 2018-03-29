#include "stdafx.h"
#include "CustomBuildingPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XTexture.h"
#include "D3D1XStateManager.h"
#include "D3D1XEnumParser.h"
#include "RwD3D1XEngine.h"
#include "DeferredRenderer.h"
#include "D3D1XVertexDeclarationManager.h"
#include "D3D1XRenderBuffersManager.h"
#include "PBSMaterial.h"
#include "Renderer.h"
extern int drawCallCount;

std::list<AlphaMesh*> CCustomBuildingPipeline::m_aAlphaMeshList{};
CCustomBuildingPipeline::CCustomBuildingPipeline():
#ifndef DebuggingShaders
CDeferredPipeline("SACustomBuilding")
#else
CDeferredPipeline( L"SACustomBuilding")
#endif // !DebuggingShaders
{
	/*D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(RwV4d);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	m_pRenderer->getDevice()->CreateBuffer(&bd, nullptr, &m_pMaterialDataBuffer);*/
}


CCustomBuildingPipeline::~CCustomBuildingPipeline()
{
	//if (m_pMaterialDataBuffer) m_pMaterialDataBuffer->Release();
}
void CCustomBuildingPipeline__Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags) {
	g_pCustomBuildingPipe->Render(repEntry, object, type, flags);
}
void CCustomBuildingPipeline::Patch()
{
	SetPointer(0x5D7B0B, CCustomBuildingPipeline__Render);
	SetPointer(0x7578AE, CCustomBuildingPipeline__Render);
}
void CCustomBuildingPipeline::ResetAlphaList()
{
	for (auto mesh : m_aAlphaMeshList) {
		mesh->entryptr = nullptr;
		mesh->worldMatrix = nullptr;
		delete mesh;
	}
	m_aAlphaMeshList.clear();
}

// Render all collected alpha meshes
void CCustomBuildingPipeline::RenderAlphaList()
{
	
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pStateMgr->SetAlphaBlendEnable(true);
	for (auto mesh : m_aAlphaMeshList)
	{
		auto curmesh = mesh->entryptr->models[mesh->meshID];
		g_pStateMgr->SetInputLayout((ID3D11InputLayout*)mesh->entryptr->header.vertexDeclaration);
		g_pStateMgr->SetVertexBuffer((ID3D11Buffer*)mesh->entryptr->header.vertexStream[0].vertexBuffer, stride, offset);

		if (!mesh->entryptr->header.indexBuffer)
			g_pDebug->printMsg("CCustomBuildingPipeline: empty index buffer found", 2);
		g_pStateMgr->SetIndexBuffer((ID3D11Buffer*)mesh->entryptr->header.indexBuffer);
		g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology(mesh->entryptr->header.primType));
		m_pVS->Set();
		m_pPS->Set();
		if (curmesh.material->surfaceProps.ambient>1.0 || CRenderer::TOBJpass == true)
			g_pRenderBuffersMgr->UpdateMaterialEmmissiveColor(curmesh.material->color);
		else
			g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(curmesh.material->color);
		float fShininess = 1.0f - RpMaterialGetFxEnvShininess(curmesh.material);
		float fSpec =0.5f;
		//RwMatrix *ltm = RwFrameGetLTM((RwFrame*)((RpAtomic*)mesh->entryptr->owner)->object.object.parent);
		g_pRenderBuffersMgr->UpdateWorldMatrix(mesh->worldMatrix);
		g_pRenderBuffersMgr->SetMatrixBuffer();
		g_pRenderBuffersMgr->UpdateMaterialSpecularInt(fSpec);
		g_pRenderBuffersMgr->UpdateMaterialGlossiness(fShininess);

		if (curmesh.material->texture) {
			if (curmesh.material->texture->raster != nullptr) {
				g_pRwCustomEngine->SetTexture(curmesh.material->texture, 0);
			}
		}
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(curmesh.numIndex, curmesh.startIndex, curmesh.minVert);
	}
}
void CCustomBuildingPipeline::Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
	RpAtomic* atomic = (RpAtomic*)object;
	RxInstanceData* entryData = (RxInstanceData*)repEntry;
	if (entryData->header.totalNumIndex == 0)
		return;
	//if (entryData->header.primType != rwPRIMTYPETRISTRIP)
	//	return;
	// Render shit
	//if (CD3D1XVertexDeclarationManager::currentVDecl != entryData->header.vertexDeclaration) {
	g_pStateMgr->SetInputLayout((ID3D11InputLayout*)entryData->header.vertexDeclaration);
	//	CD3D1XVertexDeclarationManager::currentVDecl = entryData->header.vertexDeclaration;
	//}
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pStateMgr->SetVertexBuffer((ID3D11Buffer*)entryData->header.vertexStream[0].vertexBuffer, stride, offset);
	if (!entryData->header.indexBuffer)
		g_pDebug->printMsg("CCustomBuildingPipeline: empty index buffer found", 2);
	g_pStateMgr->SetIndexBuffer((ID3D11Buffer*)entryData->header.indexBuffer);
	g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology(entryData->header.primType));
	if (m_uiDeferredStage == 3|| m_uiDeferredStage == 4) {
		m_pVoxelVS->Set();
		m_pVoxelGS->Set();
	}
	else
		m_pVS->Set();
	if (m_uiDeferredStage == 1)
		m_pDeferredPS->Set();
	else if (m_uiDeferredStage == 2)
		m_pShadowPS->Set();
	else if (m_uiDeferredStage == 3)
		m_pVoxelPS->Set();
	else if (m_uiDeferredStage == 4)
		m_pVoxelEmmissivePS->Set();
	else
		m_pPS->Set();
	BOOL oldBlendState = g_pStateMgr->GetAlphaBlendEnable();
	RwUInt8 bAlphaEnable = 0;
	for (size_t i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
	{
		bAlphaEnable = 0;
		if (m_uiDeferredStage != 2) {
			if (entryData->models[i].material->surfaceProps.ambient>1.0 || CRenderer::TOBJpass == true)
				g_pRenderBuffersMgr->UpdateMaterialEmmissiveColor(entryData->models[i].material->color);
			else
				g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(entryData->models[i].material->color);
			float fShininess = 1.0f - RpMaterialGetFxEnvShininess(entryData->models[i].material);
			float fGloss = 0.5f;//1.0f - RpMaterialGetFxEnvShininess(entryData->models[i].material);
			g_pRenderBuffersMgr->UpdateMaterialSpecularInt(fShininess);
			g_pRenderBuffersMgr->UpdateMaterialGlossiness(fGloss);
		}
		bAlphaEnable |= entryData->models[i].material->color.alpha != 255 || entryData->models[i].vertexAlpha;

		if (entryData->models[i].material->texture) {
			bAlphaEnable |= GetD3D1XRaster(entryData->models[i].material->texture->raster)->alpha;
			g_pRwCustomEngine->SetTexture(entryData->models[i].material->texture, 0);
			CPBSMaterial* mat = nullptr;
			for (auto m : CPBSMaterialMgr::materials)
			{
				if (m->m_sName == entryData->models[i].material->texture->name) {
					mat = m;
					break;
				}
			}
			if (mat != nullptr) {
				g_pStateMgr->SetRaster(mat->m_tSpecRoughness->raster, 1);
				g_pRenderBuffersMgr->UpdateHasSpecTex(1);
			}
		}
		if (m_uiDeferredStage != 1)
			g_pStateMgr->SetAlphaBlendEnable(bAlphaEnable>0);
		else {
			g_pStateMgr->SetAlphaBlendEnable(FALSE);
			if (bAlphaEnable > 0) {
				m_aAlphaMeshList.push_back(new AlphaMesh{ entryData, RwFrameGetLTM(static_cast<RwFrame*>(atomic->object.object.parent)), (int)i });
				continue;
			}
		}
		drawCallCount++;
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(entryData->models[i].numIndex, entryData->models[i].startIndex, entryData->models[i].minVert);
		g_pRenderBuffersMgr->UpdateHasSpecTex(0);
	}
	if (m_uiDeferredStage == 3 || m_uiDeferredStage == 4)
		m_pVoxelGS->ReSet();
}
