#include "stdafx.h"
#include "D3D1XSkinPipeline.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XEnumParser.h"
#include "CDebug.h"
#include "RwRenderEngine.h"
#include "D3D1XStateManager.h"
#include "DeferredRenderer.h"
#include "RwD3D1XEngine.h"
#include "D3D1XVertexDeclarationManager.h"
#include "D3D1XVertexBufferManager.h"
#include "D3D1XIndexBuffer.h"
#include "PBSMaterial.h"
#include "D3D1XRenderBuffersManager.h"

CD3D1XSkinPipeline::CD3D1XSkinPipeline(): 
#ifndef DebuggingShaders
	CDeferredPipeline("RwSkin",false)
#else
	CDeferredPipeline(L"RwSkin")
#endif // !DebuggingShaders
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 32,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONES",		0, DXGI_FORMAT_R8G8B8A8_UINT,		0, 52,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	m_pVertexDeclaration = new CD3D1XVertexDeclaration(layout, sizeof(SimpleVertexSkin), m_pVS);
	m_pSkinningDataBuffer = new CD3D1XConstantBuffer<PerSkinMatrixBuffer>();
	m_pSkinningDataBuffer->SetDebugName("SkinningCB");
}


CD3D1XSkinPipeline::~CD3D1XSkinPipeline()
{
	if (m_pSkinningDataBuffer) delete m_pSkinningDataBuffer;
	if (m_pVertexDeclaration) delete m_pVertexDeclaration;
}

bool CD3D1XSkinPipeline::Instance(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
{
	RpAtomic* atomic = (RpAtomic*)object;
	RpGeometry* geom = atomic->geometry;
	RpHAnimHierarchy* atomicHier = AtomicGetHAnimHier(atomic);
	RpSkin* geomskin = GeometryGetSkin(geom);

	resEntryHeader->totalNumVertex = geom->numVertices;
	// for some reason cutscene crashes with kernel error if we set vertex declaration
	resEntryHeader->vertexDeclaration = nullptr;//m_pVertexDeclaration->getInputLayout();
	
	SimpleVertexSkin* vertexData = new SimpleVertexSkin[resEntryHeader->totalNumVertex];

	RwUInt8	indexRemap[252];
	memset(indexRemap, 0, sizeof(indexRemap));

	// TODO: rewrite rle bone remapping
	if (geomskin->meshBoneRLECount > 0) {
		g_pDebug->printMsg("D3D1XSkinPipeline: skin data has run length encoding bones",2);
	}
	else {
		if (atomicHier->numNodes > 0) {
			for (RwUInt8 i = 0; i < static_cast<RwUInt8>(atomicHier->numNodes); i++)
				indexRemap[i] = i;
		}
		else {
			g_pDebug->printMsg("D3D1XSkinPipeline: skin data has zero bone indices",2);
		}
	}

	bool	hasNormals = geom->morphTarget[0].normals != nullptr,
			hasTexCoords = geom->texCoords[0] != nullptr,
			hasColors = geom->preLitLum&&geom->flags&rpGEOMETRYPRELIT;
	// copy vertex data to data structure that represents vertex in shader
	for (RwUInt32 i = 0; i < resEntryHeader->totalNumVertex; i++)
	{
		vertexData[i].pos = geom->morphTarget[0].verts[i];
		vertexData[i].normal = hasNormals ? geom->morphTarget[0].normals[i] : RwV3d{ 1, 1, 1 };
		vertexData[i].uv = hasTexCoords ? geom->texCoords[0][i] : RwTexCoords{ 0, 0 };
		vertexData[i].color = hasColors ? geom->preLitLum[i] : RwRGBA{ 255, 255, 255, 255 };
		vertexData[i].weights = geomskin->vertexBoneWeights[i];

		RwUInt8* indicesToRemap = (RwUInt8*)&geomskin->vertexBoneIndices[i];
		RwUInt8* remapedIndices = (RwUInt8*)&vertexData[i].indices;
		remapedIndices[0] = indexRemap[indicesToRemap[0]];
		remapedIndices[1] = indexRemap[indicesToRemap[1]];
		remapedIndices[2] = indexRemap[indicesToRemap[2]];
		remapedIndices[3] = indexRemap[indicesToRemap[3]];
	}
	// if mesh doesn't have normals generate them on the fly
	if (geom->morphTarget[0].normals == nullptr) 
		GenerateNormals(vertexData, resEntryHeader->totalNumVertex, geom->triangles, geom->numTriangles);
			
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	InitData.pSysMem = vertexData;
			
	auto buffer = new CD3D1XVertexBuffer(sizeof(SimpleVertexSkin) * resEntryHeader->totalNumVertex,&InitData);
	resEntryHeader->vertexStream[0].vertexBuffer = buffer;
	CD3D1XVertexBufferManager::AddNew(buffer);

	delete[] vertexData;
	
	return true;
}

void CD3D1XSkinPipeline::Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
	// TODO: replace this ugly hardcoded stuff with something nice, e.g. don't render skinned objects in these stages 
	if (m_uiDeferredStage == 3|| m_uiDeferredStage==5)
		return;
	RpAtomic* atomic = (RpAtomic*)object;

	RxInstanceData* entryData = (RxInstanceData*)repEntry;
	if (entryData->header.totalNumIndex == 0)
		return;
	// initialize mesh states
	g_pStateMgr->SetInputLayout(m_pVertexDeclaration->getInputLayout());
	g_pStateMgr->SetVertexBuffer(((CD3D1XVertexBuffer*)entryData->header.vertexStream[0].vertexBuffer)->getBuffer(), sizeof(SimpleVertexSkin), 0);
	g_pStateMgr->SetIndexBuffer(((CD3D1XIndexBuffer*)entryData->header.indexBuffer)->getBuffer());
	g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology((RwPrimitiveType)entryData->header.primType));
	m_pVS->Set();
	// TODO: replace checks with different pipelines or pipeline variants or something like that
	if (m_uiDeferredStage == 1)
		m_pDeferredPS->Set();
	else if (m_uiDeferredStage == 2)
		m_pShadowPS->Set();
	else
		m_pPS->Set();

	// update skin bone matrices
	RpSkin* geomSkin;
	geomSkin = GeometryGetSkin(atomic->geometry);
	RpHAnimHierarchy* hier = AtomicGetHAnimHier(atomic);
	_rwD3D9VSSetActiveWorldMatrix(RwFrameGetLTM((RwFrame*)atomic->object.object.parent));
	rpD3D9SkinVertexShaderMatrixUpdate((RwMatrix*)&RpSkinGlobals.alignedMatrixCache[0],atomic, geomSkin);
	memcpy(&m_pSkinningDataBuffer->data.mSkinToWorldMatrices, &RpSkinGlobals.alignedMatrixCache[0], sizeof(m_pSkinningDataBuffer->data.mSkinToWorldMatrices));
	m_pSkinningDataBuffer->Update();

	g_pStateMgr->SetConstantBufferVS(m_pSkinningDataBuffer, 3);

	// iterate over materials and draw them
	RwUInt8 alphaBlend;
	RxD3D9InstanceData* model;
	RpMaterial* material;
	for (size_t i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
	{
		model = &entryData->models[i];
		material = model->material;
		alphaBlend = material->color.alpha!=255 || model->vertexAlpha;
		// set texture
		if (material->texture && material->texture->raster)
		{
			alphaBlend |= GetD3D1XRaster(material->texture->raster)->alpha;
			g_pRwCustomEngine->SetTexture(material->texture, 0);
			g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(material->color);
			auto glossiness = 0.1f;
			auto intensity = 0.2f;
			g_pRenderBuffersMgr->UpdateMaterialGlossiness(glossiness);
			g_pRenderBuffersMgr->UpdateMaterialSpecularInt(intensity);
			
			// set physically based shading material params
			// TODO: add a method for material initialization
			CPBSMaterial* mat = CPBSMaterialMgr::materials[material->texture->name];
			if (mat != nullptr) {
				g_pStateMgr->SetRaster(mat->m_tSpecRoughness->raster, 1);
				
				g_pRenderBuffersMgr->UpdateHasSpecTex(1);
			}
			else
				g_pRenderBuffersMgr->UpdateHasSpecTex(0);
		}
		// set alpha blending
		g_pStateMgr->SetAlphaBlendEnable(alphaBlend > 0);
		// flush and draw
		g_pRenderBuffersMgr->FlushMaterialBuffer();
		g_pStateMgr->FlushStates();
		GET_D3D_RENDERER->DrawIndexed(model->numIndex, model->startIndex, model->minVert);
	}
}

void CD3D1XSkinPipeline::GenerateNormals(SimpleVertexSkin * verticles, unsigned int vertexCount, RpTriangle* triangles, unsigned int triangleCount)
{
	// generate normal for each triangle and vertex in mesh
	for (RwUInt32 i = 0; i < triangleCount; i++)
	{
		auto triangle = triangles[i];
		auto iA = triangle.vertIndex[0], iB = triangle.vertIndex[1], iC = triangle.vertIndex[2];
		auto vA = verticles[iA],
			vB = verticles[iB],
			vC = verticles[iC];
		// tangent vector
		RwV3d tangent = {
			vB.pos.x - vA.pos.x,
			vB.pos.y - vA.pos.y,
			vB.pos.z - vA.pos.z
		};
		// bitangent vector
		RwV3d bitangent = {
			vA.pos.x - vC.pos.x,
			vA.pos.y - vC.pos.y,
			vA.pos.z - vC.pos.z
		};
		// normal vector as cross product of (tangent X bitangent)
		RwV3d normal = {
			tangent.y*bitangent.z - tangent.z*bitangent.y,
			tangent.z*bitangent.x - tangent.x*bitangent.z,
			tangent.x*bitangent.y - tangent.y*bitangent.x
		};
		// increase normals of each vertex in triangle 
		verticles[iA].normal = {
			verticles[iA].normal.x + normal.x,
			verticles[iA].normal.y + normal.y,
			verticles[iA].normal.z + normal.z
		};
		verticles[iB].normal = {
			verticles[iB].normal.x + normal.x,
			verticles[iB].normal.y + normal.y,
			verticles[iB].normal.z + normal.z
		};
		verticles[iC].normal = {
			verticles[iC].normal.x + normal.x,
			verticles[iC].normal.y + normal.y,
			verticles[iC].normal.z + normal.z
		};
	}
	// normalize normals
	for (RwUInt32 i = 0; i < vertexCount; i++)
	{
		RwReal length = sqrt(verticles[i].normal.x*verticles[i].normal.x + verticles[i].normal.y*verticles[i].normal.y + verticles[i].normal.z*verticles[i].normal.z);
		verticles[i].normal = { verticles[i].normal.x / length, verticles[i].normal.y / length, verticles[i].normal.z / length };
	}
}
