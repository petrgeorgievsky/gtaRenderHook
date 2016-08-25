#include "stdafx.h"
#include "D3D1XDefaultPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XTexture.h"
#include "D3D1XStateManager.h"
#include "D3D1XEnumParser.h"
#include "RwRenderEngine.h"

CD3D1XDefaultPipeline::CD3D1XDefaultPipeline(CD3DRenderer* pRenderer) : 
#ifndef DebuggingShaders
	CD3D1XPipeline(pRenderer, "RwMainTesselation")
#else
	CD3D1XPipeline(pRenderer, L"RwMainTesselation")
#endif // !DebuggingShaders
{
#ifndef DebuggingShaders
	m_pDS = new CD3D1XShader(m_pRenderer, RwD3D1XShaderType::DS, "shaders/RwMainTesselation.fx", "DS");
	m_pHS = new CD3D1XShader(m_pRenderer, RwD3D1XShaderType::HS, "shaders/RwMainTesselation.fx", "HS");
#else
	m_pDS = new CD3D1XShader(m_pRenderer->getDevice(), RwD3D1XShaderType::DS, L"shaders/RwMainTesselation.fx", "DS");
	m_pHS = new CD3D1XShader(m_pRenderer->getDevice(), RwD3D1XShaderType::HS, L"shaders/RwMainTesselation.fx", "HS");
#endif // !DebuggingShaders
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(RwV4d);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	m_pRenderer->getDevice()->CreateBuffer(&bd, nullptr, &m_pMaterialDataBuffer);
}


CD3D1XDefaultPipeline::~CD3D1XDefaultPipeline()
{
	if (m_pMaterialDataBuffer) m_pMaterialDataBuffer->Release();
	if (m_pDS) delete m_pDS;
	if (m_pHS) delete m_pHS;
}

bool CD3D1XDefaultPipeline::Instance(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance) 
{
	RpAtomic* atomic = (RpAtomic*)object;
	RpGeometry* geom = atomic->geometry;
	resEntryHeader->totalNumVertex = geom->numVertices;
	// Create Vertex Declarations and Buffers
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, 32,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		if (FAILED(m_pRenderer->getDevice()->CreateInputLayout(layout, numElements, 
			m_pVS->getBlob()->GetBufferPointer(), m_pVS->getBlob()->GetBufferSize(), (ID3D11InputLayout**)&resEntryHeader->vertexDeclaration)))
		{
			g_pDebug->printError("failed to create Input Layout");
			return false;
		}
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = static_cast<UINT>(sizeof(simpleVertex)) * resEntryHeader->totalNumVertex;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;

		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource{};

			simpleVertex* vertexData = new simpleVertex[static_cast<size_t>(resEntryHeader->totalNumVertex)];
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

			if (FAILED(m_pRenderer->getDevice()->CreateBuffer(&bd, &InitData, (ID3D11Buffer**)&resEntryHeader->vertexStream[0].vertexBuffer)))
				g_pDebug->printError("Failed to create vertex buffer");
			delete[] vertexData;
		}

	}
	return true;
}

void CD3D1XDefaultPipeline::Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
	RpAtomic* atomic = (RpAtomic*)object;
	rxInstanceData* entryData = (rxInstanceData*)repEntry;
	if (entryData->header.totalNumIndex == 0)
		return;
	//if (entryData->header.primType != rwPRIMTYPETRISTRIP)
	//	return;
	ID3D11DeviceContext* devContext = m_pRenderer->getContext();
	// Render shit
	devContext->IASetInputLayout((ID3D11InputLayout*)entryData->header.vertexDeclaration);

	UINT stride = sizeof(simpleVertex);
	UINT offset = 0;
	devContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&entryData->header.vertexStream[0].vertexBuffer, &stride, &offset);
	if (!entryData->header.indexBuffer)
		g_pDebug->printMsg("no IB");
	devContext->IASetIndexBuffer((ID3D11Buffer*)entryData->header.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	devContext->IASetPrimitiveTopology(/*CD3D1XEnumParser::ConvertPrimTopology(entryData->header.primType)*/D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	m_pVS->Set();
	m_pPS->Set();
	m_pDS->Set();
	m_pHS->Set();
	BOOL oldBlendState= g_pStateMgr->GetAlphaBlendEnable();
	RwUInt8 bAlphaEnable = 0;
	for (size_t i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
	{
		bAlphaEnable = 0;
		RwV4d vec{ entryData->models[i].material->color.red / 255.0f,entryData->models[i].material->color.green / 255.0f,entryData->models[i].material->color.blue / 255.0f,entryData->models[i].material->color.alpha / 255.0f };
		bAlphaEnable |= entryData->models[i].material->color.alpha!=255 || entryData->models[i].vertexAlpha;

		devContext->UpdateSubresource(m_pMaterialDataBuffer, 0, nullptr, &vec, 0, 0);
		devContext->PSSetConstantBuffers(2, 1, &m_pMaterialDataBuffer);

		if (entryData->models[i].material->texture) {
			bAlphaEnable |= GetD3D1XRaster(entryData->models[i].material->texture->raster)->alpha;
			g_pRwCustomEngine->SetTexture(entryData->models[i].material->texture, 0);
		}
		g_pStateMgr->SetAlphaBlendEnable(bAlphaEnable>0);
		devContext->DrawIndexed(entryData->models[i].numIndex, entryData->models[i].startIndex, entryData->models[i].minVert);
	}
	g_pStateMgr->SetAlphaBlendEnable(oldBlendState);
	
	m_pDS->ReSet();
	m_pHS->ReSet();
}
