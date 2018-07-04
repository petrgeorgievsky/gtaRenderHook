// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "D3D1XIndexBuffer.h"
#include <game_sa\CWeather.h>
extern int drawCallCount;
CCustomBuildingDNPipeline::CCustomBuildingDNPipeline() :
#ifndef DebuggingShaders
	CDeferredPipeline("SACustomBuildingDN", GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0)
#else
	CDeferredPipeline(L"SACustomBuildingDN", GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0)
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
	g_pStateMgr->SetVertexBuffer(((CD3D1XBuffer*)entryData->header.vertexStream[0].vertexBuffer)->getBuffer(), sizeof(SimpleVertex), 0);

	if (!entryData->header.indexBuffer) {
		g_pDebug->printMsg("CustomBuildingDNPipeline: empty index buffer found", 0);
		return;
	}

	g_pStateMgr->SetIndexBuffer(((CD3D1XIndexBuffer*)entryData->header.indexBuffer)->getBuffer());

	g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology((RwPrimitiveType)entryData->header.primType));

	bool hasAlphaTextures = false;
	for (RwUInt32 i = 0; i < entryData->header.numMeshes; i++)
	{
		auto mesh = GetModelsData(entryData)[i];
		if (mesh.material->texture && mesh.material->texture->raster)
			hasAlphaTextures |= GetD3D1XRaster(mesh.material->texture->raster)->alpha;
	}

	if (m_uiDeferredStage == 3|| m_uiDeferredStage == 4) {
		m_pVoxelVS->Set();
		m_pVoxelGS->Set();
	}
	else
		m_pVS->Set();
	if (m_uiDeferredStage == 1)
		m_pDeferredPS->Set();
	else if (m_uiDeferredStage == 2)
		SetShadowPipeShader(hasAlphaTextures);
	else if (m_uiDeferredStage == 3)
		m_pVoxelPS->Set();
	else if (m_uiDeferredStage == 4)
		m_pVoxelEmmissivePS->Set();
	else
		m_pPS->Set();
		
	BOOL oldBlendState = g_pStateMgr->GetAlphaBlendEnable();
	
	//set<RpMaterial*> materialSet{};
	// To improve preformance we try to reduce texture set calls, to do that we need to sort every object by texture pointer.
	//list<RxD3D9InstanceData> meshList{};
	//for (size_t i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
	//	meshList.push_back(entryData->models[i]);
	//meshList.sort([](const RxD3D9InstanceData &a, const RxD3D9InstanceData &b) {return a.material->texture > b.material->texture; });
	for (RwUInt32 i=0; i<entryData->header.numMeshes;i++)
	{
		RwUInt8 bAlphaEnable = 0;
		auto mesh = GetModelsData(entryData)[i];
		bAlphaEnable = 0;
		//if (mesh.material->color.alpha == 0) continue;
		if (m_uiDeferredStage != 2) {
			RwRGBA color= mesh.material->color;
			if (m_uiDeferredStage == 1) {
				color.alpha = max(color.alpha, 2);
			}
			if (mesh.material->surfaceProps.ambient>1.0)
				g_pRenderBuffersMgr->UpdateMaterialEmmissiveColor(color);
			else
				g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(color);
			float fSpec =max(CWeather::WetRoads, CCustomCarEnvMapPipeline__GetFxSpecSpecularity(mesh.material));
			float fGlossiness = RpMaterialGetFxEnvShininess(mesh.material);
			g_pRenderBuffersMgr->UpdateMaterialSpecularInt(fSpec);
			g_pRenderBuffersMgr->UpdateMaterialGlossiness(fGlossiness);
		}
		bAlphaEnable |= mesh.material->color.alpha != 255 || mesh.vertexAlpha;

		if (mesh.material->texture && mesh.material->texture->raster) {
			bAlphaEnable |= GetD3D1XRaster(mesh.material->texture->raster)->alpha;
			
			g_pRwCustomEngine->SetTexture(mesh.material->texture, 0);
			if (m_uiDeferredStage != 2) {
				CPBSMaterial* mat = CPBSMaterialMgr::materials[mesh.material->texture->name];
				if (mat != nullptr) {
					g_pStateMgr->SetRaster(mat->m_tSpecRoughness->raster, 1);
					g_pRenderBuffersMgr->UpdateHasSpecTex(1);
				}
			}
		}
		else
			g_pRwCustomEngine->SetTexture(nullptr, 0);
		if (m_uiDeferredStage != 1)
			g_pStateMgr->SetAlphaBlendEnable(bAlphaEnable>0);
		else
			g_pStateMgr->SetAlphaBlendEnable(FALSE);
		drawCallCount++;
		g_pRenderBuffersMgr->FlushMaterialBuffer();
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(mesh.numIndex, mesh.startIndex, mesh.minVert);
		g_pRenderBuffersMgr->UpdateHasSpecTex(0);
	}
	
	if (m_uiDeferredStage == 3|| m_uiDeferredStage == 4)
		m_pVoxelGS->ReSet();
	
	g_pStateMgr->SetAlphaBlendEnable(oldBlendState);
}
