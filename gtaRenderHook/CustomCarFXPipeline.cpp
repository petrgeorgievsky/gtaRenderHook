// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "CustomCarFXPipeline.h"
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
#include "D3D1XIndexBuffer.h"
#include <game_sa\CVehicleModelInfo.h>

std::list<AlphaMesh*> CCustomCarFXPipeline::m_aAlphaMeshList{};
CCustomCarFXPipeline::CCustomCarFXPipeline() :
#ifndef DebuggingShaders
	CDeferredPipeline("SACustomCarFX", GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0)
#else
	CDeferredPipeline(L"SACustomCarFX", GET_D3D_FEATURE_LVL >= D3D_FEATURE_LEVEL_11_0)
#endif // !DebuggingShaders
{
}


CCustomCarFXPipeline::~CCustomCarFXPipeline()
{
}

void CCustomCarFXPipeline__Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags) {
	g_pCustomCarFXPipe->Render(repEntry, object, type, flags);
}
void CCustomCarFXPipeline::Patch()
{
	SetPointer(0x5D9FE4, CCustomCarFXPipeline__Render);
}

void CCustomCarFXPipeline::ResetAlphaList()
{
	for (auto mesh : m_aAlphaMeshList) {
		mesh->entryptr = nullptr;
		mesh->worldMatrix = nullptr;
		delete mesh;
	}
	m_aAlphaMeshList.clear();
}

// Render all collected alpha meshes
void CCustomCarFXPipeline::RenderAlphaList()
{
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pStateMgr->SetAlphaBlendEnable(true);
	for (auto mesh: m_aAlphaMeshList)
	{
		auto curmesh = GetModelsData(mesh->entryptr)[mesh->meshID];
		g_pStateMgr->SetInputLayout((ID3D11InputLayout*)mesh->entryptr->header.vertexDeclaration);
		g_pStateMgr->SetVertexBuffer(((CD3D1XBuffer*)mesh->entryptr->header.vertexStream[0].vertexBuffer)->getBuffer(), stride, offset);

		if (!mesh->entryptr->header.indexBuffer)
			g_pDebug->printMsg("CCustomCarFXPipeline: empty index buffer found", 2);
		g_pStateMgr->SetIndexBuffer(((CD3D1XIndexBuffer*)mesh->entryptr->header.indexBuffer)->getBuffer());
		g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology((RwPrimitiveType)mesh->entryptr->header.primType));
		m_pVS->Set();
		m_pPS->Set();
		RwRGBA paintColor;
		paintColor.red		= curmesh.material->color.red;
		paintColor.green	= curmesh.material->color.green;
		paintColor.blue		= curmesh.material->color.blue;
		paintColor.alpha	= curmesh.material->color.alpha;
		UINT colorHEX = (*(UINT*)&paintColor) & 0xFFFFFF;

		if (colorHEX < 0xAF00FF)
		{
			if (colorHEX < 0xFF3C || colorHEX == 0xFFB9 || colorHEX == 0xFF3C)
			{
				paintColor.blue = 0;
				paintColor.green = 0;
				paintColor.red = 0;
			}
		}
		if (colorHEX == 0xC8FF00 || colorHEX == 0xFF00FF || colorHEX == 0xFFFF00 || colorHEX == 0xAF00FF)
		{
			paintColor.blue = 0;
			paintColor.green = 0;
			paintColor.red = 0;
		}
		if (curmesh.material->surfaceProps.ambient>1.0)
			g_pRenderBuffersMgr->UpdateMaterialEmmissiveColor(paintColor);
		else
			g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(paintColor);
		//g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(paintColor);
		float fShininess = 1.0f - RpMaterialGetFxEnvShininess(curmesh.material);
		float fSpec = CCustomCarEnvMapPipeline__GetFxSpecSpecularity(curmesh.material);
		float fMetal = curmesh.material->surfaceProps.specular;
		//RwMatrix *ltm = RwFrameGetLTM((RwFrame*)((RpAtomic*)mesh->entryptr->owner)->object.object.parent);
		g_pRenderBuffersMgr->UpdateWorldMatrix(mesh->worldMatrix);
		g_pRenderBuffersMgr->SetMatrixBuffer();
		g_pRenderBuffersMgr->UpdateMaterialSpecularInt(fSpec);
		g_pRenderBuffersMgr->UpdateMaterialGlossiness(fShininess);
		g_pRenderBuffersMgr->UpdateMaterialMetalness(fMetal);

		if (curmesh.material->texture && curmesh.material->texture->raster != nullptr)
			g_pRwCustomEngine->SetTexture(curmesh.material->texture, 0);
		else
			g_pRwCustomEngine->SetTexture(nullptr, 0);

		g_pRenderBuffersMgr->FlushMaterialBuffer();
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(curmesh.numIndex, curmesh.startIndex, curmesh.minVert);
	}
}

void CCustomCarFXPipeline::Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
	if (m_uiDeferredStage == 5)
		return;
	RpAtomic* atomic = (RpAtomic*)object;
	RxInstanceData* entryData = (RxInstanceData*)repEntry;

	if (entryData->header.totalNumIndex == 0)
		return;
	if (!entryData->header.indexBuffer) {
		g_pDebug->printMsg("CCustomCarFXPipeline: empty index buffer found", 0);
		return;
	}

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	// Init model states 
	g_pStateMgr->SetInputLayout((ID3D11InputLayout*)entryData->header.vertexDeclaration);
	g_pStateMgr->SetVertexBuffer(((CD3D1XBuffer*)entryData->header.vertexStream[0].vertexBuffer)->getBuffer(), stride, offset);
	g_pStateMgr->SetIndexBuffer(((CD3D1XIndexBuffer*)entryData->header.indexBuffer)->getBuffer());
	g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology((RwPrimitiveType)entryData->header.primType));
	
	// Set apropriate shaders
	if (m_uiDeferredStage == 3 || m_uiDeferredStage == 4) {
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


	

	// Render each material in model
	for (size_t i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
	{
		RwUInt8 bAlphaEnable = 0;
		auto mesh = GetModelsData(entryData)[i];
		bAlphaEnable = 0;
		// Setup material properties
		if (m_uiDeferredStage != 2) {
			RwRGBA paintColor;
			paintColor.red = mesh.material->color.red;
			paintColor.green = mesh.material->color.green;
			paintColor.blue = mesh.material->color.blue;
			paintColor.alpha = mesh.material->color.alpha;
			UINT colorHEX = (*(UINT*)&paintColor) & 0xFFFFFF;
			
			if (colorHEX < 0xAF00FF)
			{
				if (colorHEX < 0xFF3C || colorHEX == 0xFFB9 || colorHEX == 0xFF3C)
				{
					paintColor.blue = 0;
					paintColor.green = 0;
					paintColor.red = 0;
				}
			}
			if (colorHEX == 0xC8FF00 || colorHEX == 0xFF00FF || colorHEX == 0xFFFF00|| colorHEX == 0xAF00FF)
			{
				paintColor.blue = 0;
				paintColor.green = 0;
				paintColor.red = 0;
			}

			float fShininess = 1.0f - RpMaterialGetFxEnvShininess(mesh.material);
			float fSpec = CCustomCarEnvMapPipeline__GetFxSpecSpecularity(mesh.material);

			if(mesh.material->surfaceProps.ambient>1.0)
				g_pRenderBuffersMgr->UpdateMaterialEmmissiveColor(paintColor);
			else
				g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(paintColor);
			g_pRenderBuffersMgr->UpdateMaterialSpecularInt(fSpec);
			g_pRenderBuffersMgr->UpdateMaterialGlossiness(fShininess);
		}
		bAlphaEnable |= mesh.material->color.alpha != 255 || mesh.vertexAlpha;
		g_pRenderBuffersMgr->UpdateHasSpecTex(0);
		// Setup texture
		if (mesh.material->texture && mesh.material->texture->raster != nullptr)
			g_pRwCustomEngine->SetTexture(mesh.material->texture, 0);
		else
			g_pRwCustomEngine->SetTexture(nullptr, 0);

		// Setup alpha blending
		if (m_uiDeferredStage != 1 && m_uiDeferredStage != 2)
			g_pStateMgr->SetAlphaBlendEnable(bAlphaEnable>0);
		else {
			g_pStateMgr->SetAlphaBlendEnable(FALSE);			
			if (bAlphaEnable > 0 && mesh.material->color.alpha!=0)// If mesh has alpha chanel and it's not hiden than we need to add it to alpha render list  
			{
				m_aAlphaMeshList.push_back(new AlphaMesh{ entryData, RwFrameGetLTM(static_cast<RwFrame*>(atomic->object.object.parent)), (int)i });
				continue;
			}
		}
		// Draw mesh
		g_pRenderBuffersMgr->FlushMaterialBuffer();
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(mesh.numIndex, mesh.startIndex, mesh.minVert);
	}
	if (m_uiDeferredStage == 3 || m_uiDeferredStage == 4)
		m_pVoxelGS->ReSet();
}
