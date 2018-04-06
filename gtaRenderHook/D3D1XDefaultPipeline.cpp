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
#include "D3D1XVertexBuffer.h"
#include "D3D1XIndexBuffer.h"
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
	// create vertex declaration
	// TODO: add more robust vertex declaration generation
	auto vdeclPtr = CD3D1XVertexDeclarationManager::AddNew(m_pVS, geom->flags | rpGEOMETRYNORMALS | 
										rpGEOMETRYPRELIT | rpGEOMETRYTEXTURED | rpGEOMETRYPOSITIONS);
	resEntryHeader->vertexDeclaration = vdeclPtr->getInputLayout();

	// copy vertex data to data structure that represents vertex in shader
	// TODO: add ability to create custom verticle types on fly,
	// currently even if mesh doesn't have colors/textures they are still filled, that could be potentional performance hit.
	SimpleVertex* vertexData = new SimpleVertex[resEntryHeader->totalNumVertex];
	
	bool	hasNormals = geom->morphTarget[0].normals != nullptr,
			hasTexCoords = geom->texCoords[0] != nullptr,
			hasColors = geom->preLitLum&&geom->flags&rpGEOMETRYPRELIT;

	for (RwUInt32 i = 0; i < resEntryHeader->totalNumVertex; i++)
	{
		vertexData[i].pos = geom->morphTarget[0].verts[i];
		vertexData[i].normal = hasNormals ? geom->morphTarget[0].normals[i] : RwV3d{ 0, 0, 0 };
		vertexData[i].uv = hasTexCoords? geom->texCoords[0][i] : RwTexCoords{ 0, 0 };
		vertexData[i].color = hasColors? geom->preLitLum[i]: RwRGBA{ 255, 255, 255, 255 };
	}
	// if mesh doesn't have normals generate them on the fly
	if (!hasNormals)
		GenerateNormals(vertexData, resEntryHeader->totalNumVertex, geom->triangles, geom->numTriangles);
	
	D3D11_SUBRESOURCE_DATA InitData;

	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	InitData.pSysMem = vertexData;

	auto buffer = new CD3D1XVertexBuffer(sizeof(SimpleVertex) * resEntryHeader->totalNumVertex, &InitData);
	resEntryHeader->vertexStream[0].vertexBuffer = buffer;
	CD3D1XVertexBufferManager::AddNew(buffer);

	delete[] vertexData;
	
	return true;
}

void CD3D1XDefaultPipeline::Render(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
	RpAtomic* atomic = static_cast<RpAtomic*>(object);
	RxInstanceData* entryData = static_cast<RxInstanceData*>(repEntry);
	// early return
	if (entryData->header.totalNumIndex == 0)
		return;
	// initialize mesh states
	// TODO: reduce casts
	g_pStateMgr->SetInputLayout(static_cast<ID3D11InputLayout*>(entryData->header.vertexDeclaration));
	g_pStateMgr->SetVertexBuffer(((CD3D1XVertexBuffer*)entryData->header.vertexStream[0].vertexBuffer)->getBuffer(), sizeof(SimpleVertex), 0);
	g_pStateMgr->SetIndexBuffer(((CD3D1XIndexBuffer*)entryData->header.indexBuffer)->getBuffer());
	g_pStateMgr->SetPrimitiveTopology(CD3D1XEnumParser::ConvertPrimTopology((RwPrimitiveType)entryData->header.primType));
	m_pVS->Set();
	m_pPS->Set();

	// iterate over materials and draw them
	RwUInt8 alphaBlend;
	RxD3D9InstanceData* model;
	RpMaterial* material;
	for (RwUInt32 i = 0; i < static_cast<size_t>(entryData->header.numMeshes); i++)
	{
		model = &entryData->models[i];
		material = model->material;
		alphaBlend = material->color.alpha!=255 || model->vertexAlpha;

		// set albedo(diffuse) color
		g_pRenderBuffersMgr->UpdateMaterialDiffuseColor(material->color);
		// set texture
		if (material->texture) {
			alphaBlend |= GetD3D1XRaster(material->texture->raster)->alpha;
			g_pRwCustomEngine->SetTexture(material->texture, 0);
		}
		// set alpha blending
		g_pStateMgr->SetAlphaBlendEnable(alphaBlend>0);
		// flush and draw
		g_pStateMgr->FlushStates();
		g_pRenderBuffersMgr->FlushMaterialBuffer();
		GET_D3D_RENDERER->DrawIndexed(model->numIndex, model->startIndex, model->minVert);
	}
}

void CD3D1XDefaultPipeline::GenerateNormals(SimpleVertex * verticles, unsigned int vertexCount, RpTriangle* triangles, unsigned int triangleCount)
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
