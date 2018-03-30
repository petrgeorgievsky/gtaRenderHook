#include "stdafx.h"
#include "CustomBuildingDNPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XTexture.h"
#include "D3D1XStateManager.h"
#include "D3D1XEnumParser.h"
#include "RwRenderEngine.h"
#include "DeferredRenderer.h"
#include "D3D1XVertexDeclarationManager.h"
#include "D3D1XRenderBuffersManager.h"
#include "Renderer.h"
#include "RwD3D1XEngine.h"
#include "PBSMaterial.h"
#include <game_sa\CWeather.h>
extern int drawCallCount;
CCustomBuildingDNPipeline::CCustomBuildingDNPipeline() :
#ifndef DebuggingShaders
	CDeferredPipeline("SACustomBuildingDN")
#else
	CDeferredPipeline(L"SACustomBuildingDN")
#endif // !DebuggingShaders
{
}


CCustomBuildingDNPipeline::~CCustomBuildingDNPipeline()
{
}
void CCustomBuildingDNPipeline__Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags) {
	g_pCustomBuildingDNPipe->Render(repEntry, object, type, flags);
}
void CCustomBuildingDNPipeline::Patch()
{
	SetPointer(0x5D67F4, CCustomBuildingDNPipeline__Render);
	SetPointer(0x815EDF, CCustomBuildingDNPipeline__Render);
	SetPointer(0x815F65, CCustomBuildingDNPipeline__Render);
}

void CCustomBuildingDNPipeline::Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
	RpAtomic* atomic		  = (RpAtomic*)object;
	RxInstanceData* entryData = (RxInstanceData*)repEntry;


	if (entryData->header.totalNumIndex == 0)
		return;

	g_pStateMgr->SetInputLayout((ID3D11InputLayout*)entryData->header.vertexDeclaration);
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pStateMgr->SetVertexBuffer((ID3D11Buffer*)entryData->header.vertexStream[0].vertexBuffer, stride, offset);
	if (!entryData->header.indexBuffer)
		g_pDebug->printMsg("CustomBuildingDNPipeline: empty index buffer found", 2);
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
	set<RpMaterial*> materialSet{};
	// To improve preformance we try to reduce texture set calls, to do that we need to sort every object by texture pointer.
	list<RxD3D9InstanceData> meshList{};
	for (size_t i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
		meshList.push_back(entryData->models[i]);
	meshList.sort([](const RxD3D9InstanceData &a, const RxD3D9InstanceData &b) {return a.material->texture > b.material->texture; });
	for (auto mesh: meshList)
	{
		bAlphaEnable = 0;
		if (m_uiDeferredStage != 2) {
			RwRGBA color= mesh.material->color;
			if (m_uiDeferredStage == 1) {
				color.alpha = max(color.alpha, 2);
			}
			if (mesh.material->surfaceProps.ambient>1.0|| CRenderer::TOBJpass == true)
				g_pRenderBuffersMgr->UpdateMaterialEmmissiveColor(color);
			else
				g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(color);
			float fSpec =max(CWeather::WetRoads, CCustomCarEnvMapPipeline__GetFxSpecSpecularity(mesh.material));
			float fGlossiness = RpMaterialGetFxEnvShininess(mesh.material);
			g_pRenderBuffersMgr->UpdateMaterialSpecularInt(fSpec);
			g_pRenderBuffersMgr->UpdateMaterialGlossiness(fGlossiness);
		}
		bAlphaEnable |= mesh.material->color.alpha != 255 || mesh.vertexAlpha;

		if (mesh.material->texture&&mesh.material->texture->raster) {
			bAlphaEnable |= GetD3D1XRaster(mesh.material->texture->raster)->alpha;
			
			g_pRwCustomEngine->SetTexture(mesh.material->texture, 0);
			CPBSMaterial* mat = nullptr;
			for (auto m : CPBSMaterialMgr::materials)
			{
				if (m->m_sName == mesh.material->texture->name) {
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
		else
			g_pStateMgr->SetAlphaBlendEnable(FALSE);
		drawCallCount++;
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(mesh.numIndex, mesh.startIndex, mesh.minVert);
		g_pRenderBuffersMgr->UpdateHasSpecTex(0);
	}
	
	if (m_uiDeferredStage == 3|| m_uiDeferredStage == 4)
		m_pVoxelGS->ReSet();
	
	g_pStateMgr->SetAlphaBlendEnable(oldBlendState);
}