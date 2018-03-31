#include "stdafx.h"
#include "D3D1XDefaultPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XTexture.h"
#include "D3D1XStateManager.h"
#include "D3D1XEnumParser.h"
#include "RwD3D1XEngine.h"
#include "D3D1XRenderBuffersManager.h"
#include "D3D1XVertexDeclarationManager.h"
#include "D3D1XVertexDeclaration.h"
#include "D3D1XVertexBufferManager.h"
#include "RwVectorMath.h"

CD3D1XDefaultPipeline::CD3D1XDefaultPipeline() : 
	CD3D1XPipeline("RwMain")
{
}


CD3D1XDefaultPipeline::~CD3D1XDefaultPipeline()
{
}

bool CD3D1XDefaultPipeline::Instance(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance) const
{
	RpAtomic* atomic = static_cast<RpAtomic*>(object);
	RpGeometry* geom = atomic->geometry;
	resEntryHeader->totalNumVertex = geom->numVertices;
	// Create Vertex Declarations and Buffers
	{
		auto vdeclPtr = CD3D1XVertexDeclarationManager::AddNew(m_pVS, geom->flags | rpGEOMETRYNORMALS | 
											rpGEOMETRYPRELIT | rpGEOMETRYTEXTURED | rpGEOMETRYPOSITIONS);
		resEntryHeader->vertexDeclaration = vdeclPtr->getInputLayout();
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = static_cast<UINT>(sizeof(SimpleVertex)) * resEntryHeader->totalNumVertex;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;

		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource{};

			SimpleVertex* vertexData = new SimpleVertex[static_cast<size_t>(resEntryHeader->totalNumVertex)];
			
			for (size_t i = 0; i < static_cast<size_t>(resEntryHeader->totalNumVertex); i++)
			{
				vertexData[i].pos = geom->morphTarget[0].verts[i];
				if (geom->morphTarget[0].normals)
					vertexData[i].normal = geom->morphTarget[0].normals[i];
				else
					vertexData[i].normal = { 0,0,0 };
				if (geom->texCoords[0])
					vertexData[i].uv = geom->texCoords[0][i];
				else
					vertexData[i].uv = { 0,0 };
				if (geom->preLitLum&&geom->flags&rpGEOMETRYPRELIT)
					vertexData[i].color = geom->preLitLum[i];
				else
					vertexData[i].color = { 255,255,255,255 };
			}

			if (geom->morphTarget[0].normals == nullptr) {
				for (int i = 0; i < geom->numTriangles; i++)
				{
					RwV3d firstvec = {	
						vertexData[geom->triangles[i].vertIndex[1]].pos.x - vertexData[geom->triangles[i].vertIndex[0]].pos.x,
						vertexData[geom->triangles[i].vertIndex[1]].pos.y - vertexData[geom->triangles[i].vertIndex[0]].pos.y,
						vertexData[geom->triangles[i].vertIndex[1]].pos.z - vertexData[geom->triangles[i].vertIndex[0]].pos.z 
					};
					RwV3d secondvec = {
						vertexData[geom->triangles[i].vertIndex[0]].pos.x - vertexData[geom->triangles[i].vertIndex[2]].pos.x,
						vertexData[geom->triangles[i].vertIndex[0]].pos.y - vertexData[geom->triangles[i].vertIndex[2]].pos.y,
						vertexData[geom->triangles[i].vertIndex[0]].pos.z - vertexData[geom->triangles[i].vertIndex[2]].pos.z
					};
					RwV3d normal = {
						firstvec.y*secondvec.z - firstvec.z*secondvec.y,
						firstvec.z*secondvec.x - firstvec.x*secondvec.z,
						firstvec.x*secondvec.y - firstvec.y*secondvec.x
					};// (firstvec, secondvec);
					RwReal length = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
					normal = { normal.x / length,normal.y / length,normal.z / length };
					
					vertexData[geom->triangles[i].vertIndex[0]].normal = {
						vertexData[geom->triangles[i].vertIndex[0]].normal.x + normal.x,
						vertexData[geom->triangles[i].vertIndex[0]].normal.y + normal.y,
						vertexData[geom->triangles[i].vertIndex[0]].normal.z + normal.z
					};
					vertexData[geom->triangles[i].vertIndex[1]].normal = {
						vertexData[geom->triangles[i].vertIndex[1]].normal.x + normal.x,
						vertexData[geom->triangles[i].vertIndex[1]].normal.y + normal.y,
						vertexData[geom->triangles[i].vertIndex[1]].normal.z + normal.z
					};
					vertexData[geom->triangles[i].vertIndex[2]].normal = {
						vertexData[geom->triangles[i].vertIndex[2]].normal.x + normal.x,
						vertexData[geom->triangles[i].vertIndex[2]].normal.y + normal.y,
						vertexData[geom->triangles[i].vertIndex[2]].normal.z + normal.z
					};
				}
				for (size_t i = 0; i < static_cast<size_t>(resEntryHeader->totalNumVertex); i++)
				{
					RwReal length = sqrt(vertexData[i].normal.x*vertexData[i].normal.x + vertexData[i].normal.y*vertexData[i].normal.y + vertexData[i].normal.z*vertexData[i].normal.z);
					vertexData[i].normal = { vertexData[i].normal.x / length,vertexData[i].normal.y / length,vertexData[i].normal.z / length };
				}
			}

			InitData.pSysMem = vertexData;
			
			
			if (FAILED(GET_D3D_DEVICE->CreateBuffer(&bd, &InitData, (ID3D11Buffer**)&resEntryHeader->vertexStream[0].vertexBuffer)))
				g_pDebug->printError("Failed to create vertex buffer");
			ID3D11Buffer* buffptr = static_cast<ID3D11Buffer*>(resEntryHeader->vertexStream[0].vertexBuffer);
			CD3D1XVertexBufferManager::AddNew(buffptr);
			//g_pDebug->SetD3DName((ID3D11DeviceChild*)resEntryHeader->vertexStream[0].vertexBuffer, "VertexBuffer::" + std::to_string(resEntryHeader->serialNumber));
			delete[] vertexData;
		}

	}
	return true;
}

void CD3D1XDefaultPipeline::Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
	RpAtomic* atomic = static_cast<RpAtomic*>(object);
	RxInstanceData* entryData = static_cast<RxInstanceData*>(repEntry);
	if (entryData->header.totalNumIndex == 0)
		return;
	//if (entryData->header.primType != rwPRIMTYPETRISTRIP)
	//	return;
	// Render shit
	g_pStateMgr->SetInputLayout(static_cast<ID3D11InputLayout*>(entryData->header.vertexDeclaration));

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	g_pStateMgr->SetVertexBuffer((ID3D11Buffer*)entryData->header.vertexStream[0].vertexBuffer, stride, offset);
	if (!entryData->header.indexBuffer)
		g_pDebug->printMsg("CD3D1XDefaultPipeline: empty index buffer found", 2);
	g_pStateMgr->SetIndexBuffer((ID3D11Buffer*)entryData->header.indexBuffer);
	g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology(entryData->header.primType));
	m_pVS->Set();
	m_pPS->Set();
	//m_pDS->Set();
	//m_pHS->Set();
	BOOL oldBlendState= g_pStateMgr->GetAlphaBlendEnable();
	for (size_t i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
	{
		RwUInt8 bAlphaEnable = 0;
		bAlphaEnable |= entryData->models[i].material->color.alpha!=255 || entryData->models[i].vertexAlpha;

		g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(entryData->models[i].material->color);
		if (entryData->models[i].material->texture) {
			bAlphaEnable |= GetD3D1XRaster(entryData->models[i].material->texture->raster)->alpha;
			g_pRwCustomEngine->SetTexture(entryData->models[i].material->texture, 0);
		}
		g_pStateMgr->SetAlphaBlendEnable(bAlphaEnable>0);
		g_pRenderBuffersMgr->FlushMaterialBuffer();
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(entryData->models[i].numIndex, entryData->models[i].startIndex, entryData->models[i].minVert);
	}
	g_pStateMgr->SetAlphaBlendEnable(oldBlendState);
	
	//m_pDS->ReSet();
	//m_pHS->ReSet();
}
