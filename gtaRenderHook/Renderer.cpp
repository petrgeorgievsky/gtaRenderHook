// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "Renderer.h"
#include "RwD3D1XEngine.h"
#include "RwMethods.h"
#include "ShadowRenderer.h"
#include "RwVectorMath.h"
#include <game_sa\CCamera.h>
#include <game_sa\common.h>
#include <game_sa\CStreaming.h>
#include <game_sa\CWorld.h>
#include <game_sa\CEntity.h>
#include <game_sa\CVisibilityPlugins.h>
#include <game_sa\CClock.h>
#include <game_sa\CGame.h>
#include "DebugRendering.h"
#include <game_sa\CGeneral.h>
#include <game_sa\CVehicle.h>
#include "DeferredRenderer.h"
#include "CVisibilityPluginsRH.h"
#include "D3D1XStateManager.h"
#include "SAIdleHook.h"
#include "DebugBBox.h"
bool		&CRendererRH::ms_bRenderTunnels = *(bool*)0xB745C0;
bool		&CRendererRH::ms_bRenderOutsideTunnels = *(bool*)0xB745C1;
bool&		CRendererRH::ms_bInTheSky = *(bool*)0xB76851;
UINT&		CRendererRH::ms_nNoOfVisibleEntities = *(UINT*)0xB76844;
UINT&		CRendererRH::ms_nNoOfVisibleLods = *(UINT*)0xB76840;
UINT&		CRendererRH::ms_nNoOfInVisibleEntities = *(UINT*)0xB7683C;
UINT&		CRendererRH::m_loadingPriority = *(UINT*)0xB76850;
CEntity*	CRendererRH::m_pFirstPersonVehicle = (CEntity*)0xB745D4;
RwPlane CRendererRH::ms_aShadowCasterBoundingPlanes[4][5] = { {}, {}, {}, {} };

sLodListEntry*&	CRendererRH::ms_pLodRenderList = *(sLodListEntry**)0xB745D0;
sLodListEntry*&	CRendererRH::ms_pLodDontRenderList = *(sLodListEntry**)0xB745CC;

CEntity**	CRendererRH::ms_aVisibleEntityPtrs = (CEntity**)0xB75898;
CEntity**	CRendererRH::ms_aInVisibleEntityPtrs = (CEntity**)0xB745D8;

CEntity**	CRendererRH::ms_aVisibleLodPtrs = (CEntity**)0xB748F8;
//CBaseModelInfo**	CRenderer::ms_modelInfoPtrs = (CBaseModelInfo**)0xA9B0C8;
CVector&			CRendererRH::ms_vecCameraPosition = *(CVector*)0xB76870;
float&				CRendererRH::ms_fCameraHeading = *(float*)0xB7684C;
float&				CRendererRH::ms_lowLodDistScale = *(float*)0x8CD804;
float&				CRendererRH::ms_fFarClipPlane = *(float*)0xB76848;
bool CRendererRH::TOBJpass = false;
std::list<CEntity*> CRendererRH::ms_aVisibleEntities{};
std::list<CEntity*> CRendererRH::ms_aVisibleLods{};
std::vector<CEntity*> CRendererRH::ms_aVisibleShadowCasters[4] = { {}, {},{},{} };
std::vector<CEntity*> CRendererRH::ms_aVisibleReflectionObjects{};

//9654B0     ; CStreaming::ms_disableStreaming
#define CStreaming__ms_disableStreaming (*(unsigned char *)0x9654B0)
//8E4CB8     ; CStreaming::ms_numPriorityRequests
#define CStreaming__ms_numPriorityRequests (*(int *)0x8E4CB8)
//8E4CC0     ; CStreamingInfo CStreaming::ms_aInfoForModel
#define CStreaming__ms_aInfoForModel ((CStreamingInfo *)0x9654B0)

#define CRenderer__SetupEntityVisibility(a1,a2) ((int (__cdecl *)(CEntity *, float *))0x554230)(a1,a2)
#define CRenderer__ProcessLodRenderLists() ((void (__cdecl *)())0x553770)()
#define CRenderer__ResetLodRenderLists() ((void (__cdecl *)())0x5536F0)()
#define COcclusion__ProcessBeforeRendering() ((void (__cdecl *)())0x7201C0)()
#define CVehicle__SetupRender(veh) ((void(__thiscall *)(void*))0x6D64F0)(veh)
#define CVehicle__ResetAfterRender(veh) ((void(__thiscall *)(void*))0x6D0E20)(veh)
//56E0D0     ; CVehicle *__cdecl FindPlayerVehicle(int playerNum, char includeRemote)
#define FindPlayerVehicle(playerNum, includeRemote) ((CVehicle* (__cdecl *)(int, bool))0x56E0D0)(playerNum,includeRemote)
void CRendererRH::RenderRoads()
{
	// Set road render-states.
	//g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEFOGENABLE, 1);
	//g_pRwCustomEngine->RenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, 1);
	// Draw roads.
	//if (ms_nNoOfVisibleEntities == 0) {
	for (auto entity : ms_aVisibleEntities) {
		auto st = CModelInfo::ms_modelInfoPtrs[entity->m_nModelIndex]->GetModelType();
		//if (st != 3) {
			RenderOneNonRoad(entity);
		//}
	}
	//for (auto entity : ms_aVisibleLods)
	//	RenderOneRoad(entity);
	//}
	//else {
	for (size_t i = 0; i < ms_nNoOfVisibleEntities; ++i) {
		auto entity = ms_aVisibleEntityPtrs[i];
		auto st = CModelInfo::ms_modelInfoPtrs[entity->m_nModelIndex]->GetModelType();
		//if (st != 3)
			RenderOneNonRoad(ms_aVisibleEntityPtrs[i]);
	}
	//}
	for (size_t i = 0; i < ms_nNoOfVisibleLods; ++i) {
		auto entity = ms_aVisibleLodPtrs[i];
		auto st = CModelInfo::ms_modelInfoPtrs[entity->m_nModelIndex]->GetModelType();
		//if (st != 3)
			RenderOneNonRoad(ms_aVisibleLodPtrs[i]);
	}
}

void CRendererRH::RenderCubemapEntities()
{
	g_pStateMgr->SetAlphaTestEnable(true);
	g_pStateMgr->SetAlphaTestRef(0.8f);
	for (auto entity : ms_aVisibleReflectionObjects)
		RenderReflectionEntity(entity);
}

void CRendererRH::RenderEverythingBarRoads()
{
	//for (size_t i = 0; i < ms_nNoOfVisibleEntities; ++i)
	//	RenderOneNonRoad(ms_aVisibleEntityPtrs[i]);
	
	// Set non-road render-states.
	// Draw non-roads.
}

void CRendererRH::RenderTOBJs()
{
	TOBJpass = true;
	for (size_t i = 0; i < ms_nNoOfVisibleEntities; ++i) {

		auto st = CModelInfo::ms_modelInfoPtrs[ms_aVisibleEntityPtrs[i]->m_nModelIndex]->GetModelType();
		if (st == 3) {
			((void(__cdecl *)(CEntity *))0x553230)(ms_aVisibleEntityPtrs[i]);
		}
	}
	for (size_t i = 0; i < ms_nNoOfVisibleLods; ++i) {

		auto st = CModelInfo::ms_modelInfoPtrs[ms_aVisibleLodPtrs[i]->m_nModelIndex]->GetModelType();
		if (st == 3) {
			((void(__cdecl *)(CEntity *))0x553230)(ms_aVisibleLodPtrs[i]);
		}
	}
	TOBJpass = false;
}

void CRendererRH::RenderShadowCascade(int i)
{
	for (auto shadowEntity : ms_aVisibleShadowCasters[gShadowSettings.CullPerCascade ? i : 0])
		RenderShadowCasterEntity(shadowEntity);//shadowEntity->Render();
	
}

void CRendererRH::ConstructShadowList()
{
	// First get true BBox depth
	/*for (int i = 0; i < ms_nNoOfVisibleEntities; i++)
	{
		auto entity = ms_aVisibleEntityPtrs[i];

		int modelID = entity->entity.m_wModelIndex;
		CBaseModelInfo* modelInfo = CModelInfo__ms_modelInfoPtrs[modelID];

		for (int j = 0; j < 4; j++)
		{
			RW::BBox lightSpaceBBox = CShadowRenderer::m_LightBBox[j];
			RW::V3d entityPos = { entity->entity.placeable.m_pCoords ? entity->entity.placeable.m_pCoords->matrix.pos : entity->entity.placeable.placement.pos };

			RwBBox		entityBBox = modelInfo->m_pColModel->m_BBox.m_BBox;
			RwSphere entityBSphere = modelInfo->m_pColModel->m_BBox.m_BSphere;
			RW::V3d sphereOffset = { entityBSphere.center };

			//if (i == 0)
			//	DebugRendering::AddToRenderList(new DebugBBox(RW::BBox(entityBBox)));

			RW::Matrix entityTransform;
			if (entity->entity.placeable.m_pCoords)
				entityTransform = { entity->entity.placeable.m_pCoords->matrix };

			auto entityBBoxVerticles = (RW::BBox(entityBBox)).getVerticles();

			for (int j = 0; j < entityBBoxVerticles.size(); j++) {
				if (entity->entity.placeable.m_pCoords)
					entityBBoxVerticles[j] = entityBBoxVerticles[j] * entityTransform;
				entityBBoxVerticles[j] = entityBBoxVerticles[j] * CShadowRenderer::m_LightSpaceMatrix[j];
			}

			RW::BBox entityBBox2 = RW::BBox(entityBBoxVerticles.data(), entityBBoxVerticles.size());
			if (entity->entity.placeable.m_pCoords == nullptr)
				entityBBox2 += entityPos;
			// if entity is fully inside bbox than add it to render list
			if (lightSpaceBBox.intersects2D(entityBBox2)) {
				// Extend light BBox z, to caputure objects that is far(or not so far) behind
				CShadowRenderer::m_LightBBox[j].extendZ(entityBBox2.getMin());
				CShadowRenderer::m_LightBBox[j].extendZ(entityBBox2.getMax());
			}
		}
	}*/
	/*for (int i = 0; i < ms_nNoOfVisibleEntities; i++)
	{
		auto entity = ms_aVisibleEntityPtrs[i];
		int modelID = entity->m_nModelIndex;
		CBaseModelInfo* modelInfo = CModelInfo::ms_modelInfoPtrs[modelID];
		for (int j = 0; j < 4; j++)
		{
			RW::BBox lightSpaceBBox = CShadowRenderer::m_LightBBox[j];
			RW::V3d entityPos = { entity->GetPosition() };

			RwBBox		entityBBox = modelInfo->m_pColModel->m_boundBox.m_BBox;
			RwSphere entityBSphere = modelInfo->m_pColModel->m_BBox.m_BSphere;

			RW::Matrix entityTransform;
			if (entity->entity.placeable.m_pCoords)
				entityTransform = { entity->entity.placeable.m_pCoords->matrix };

			auto entityBBoxVerticles = (RW::BBox(entityBBox)).getVerticles();

			for (int j = 0; j < entityBBoxVerticles.size(); j++) {
				if (entity->entity.placeable.m_pCoords)
					entityBBoxVerticles[j] = entityBBoxVerticles[j] * entityTransform;
				entityBBoxVerticles[j] = entityBBoxVerticles[j] * CShadowRenderer::m_LightSpaceMatrix[j];
			}

			RW::BBox entityBBox2 = RW::BBox(entityBBoxVerticles.data(), entityBBoxVerticles.size());
			if (entity->entity.placeable.m_pCoords == nullptr)
				entityBBox2 += entityPos;
			// if entity is fully inside bbox than add it to render list
			if (lightSpaceBBox.intersects2D(entityBBox2)) {
				ms_aVisibleShadowCasters[j].push_back(entity);
			}

		}
		
	}*/
}


void CRendererRH::ConstructRenderList()
{
	// auto cullzoneattribs=CCullZones::FindTunnelAttributesForCoors(cameraPos);
	ms_bRenderTunnels = true;
	ms_bRenderOutsideTunnels = true;
	ms_bInTheSky = false;
	ms_lowLodDistScale = 1.0;
	COcclusion__ProcessBeforeRendering();

	ms_vecCameraPosition = TheCamera.GetPosition();
	ms_fCameraHeading	 = TheCamera.GetHeading();
	ms_fFarClipPlane	 = TheCamera.m_pRwCamera->farPlane;

	ms_nNoOfVisibleEntities = 0;
	ms_nNoOfVisibleLods = 0;
	ms_nNoOfInVisibleEntities = 0;

	ResetLodRenderLists();
	
	ScanWorld();
	ProcessLodRenderLists();

	CStreaming::StartRenderEntities();
}

void CRendererRH::RenderFadingInEntities()
{
	// Set fading entity render-states.
	// Draw fading entities.
}

void CRendererRH::AddEntityToRenderList(CEntity * entity, float renderDistance)
{
	/*if (ms_nNoOfVisibleEntities < 1000) {
		ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities] = entity;
		ms_nNoOfVisibleEntities++;
	}
	if(ms_aVisibleEntities.size()<4000)
		ms_aVisibleEntities.push_back(entity);
	*/
	CRenderer__AddEntityToRenderList(entity, renderDistance);
}

void CRendererRH::AddEntityToReflectionList(CEntity * entity, float renderDistance)
{
	// Make lod everything >50 units away and cull everything >100 units away
	if (ms_aVisibleReflectionObjects.size() > 500)
		return;
	if (renderDistance < 50) 
		ms_aVisibleReflectionObjects.push_back(entity);
	else if(entity->m_pLod!=nullptr&&renderDistance < 100)
		ms_aVisibleReflectionObjects.push_back(entity->m_pLod);
}

char CRendererRH::AddEntityToShadowCasterList(CEntity * entity, float renderDistance, int shadowCascade)
{
	if (ms_aVisibleShadowCasters[shadowCascade].size() > 1000)
		return true;
	ms_aVisibleShadowCasters[shadowCascade].push_back(entity);
	return true;
}

char CRendererRH::AddToLodRenderList(CEntity * entity, float renderDistance)
{
	/*for (int i = 0; i < 4; i++)
	{
		int modelID = entity->entity.m_wModelIndex;
		CBaseModelInfo* modelInfo = CModelInfo__ms_modelInfoPtrs[modelID];
		RW::BBox lightSpaceBBox = CShadowRenderer::m_LightBBox[i];
		RW::V3d entityPos = { entity->entity.placeable.m_pCoords ? entity->entity.placeable.m_pCoords->matrix.pos : entity->entity.placeable.placement.pos };
		RwBBox		entityBBox = modelInfo->m_pColModel->m_BBox.m_BBox;
		RwSphere	entityBSphere = modelInfo->m_pColModel->m_BBox.m_BSphere;
		entityBBox.inf = { entityPos.getX() + entityBSphere.center.x + entityBBox.inf.x,
			entityPos.getY() + entityBSphere.center.y + entityBBox.inf.y,
			entityPos.getZ() + entityBSphere.center.z + entityBBox.inf.z };
		entityBBox.sup = { entityPos.getX() + entityBSphere.center.x + entityBBox.sup.x,
			entityPos.getY() + entityBSphere.center.y + entityBBox.sup.y,
			entityPos.getZ() + entityBSphere.center.z + entityBBox.sup.z };
		auto entityBBoxVerticles = (RW::BBox(entityBBox)).getVerticles();
		for (int j = 0; j<entityBBoxVerticles.size(); j++)
			entityBBoxVerticles[j] = entityBBoxVerticles[j] * CShadowRenderer::m_LightSpaceMatrix[i];
		RW::BBox entityBBox2 = RW::BBox(entityBBoxVerticles.data(), entityBBoxVerticles.size());
		// if entity is fully inside bbox than add it to render list
		if (lightSpaceBBox.intersects2D(entityBBox2)) {
			// Extend light BBox z, to caputure objects that is far(or not so far) behind
			CShadowRenderer::m_LightBBox[i].extendZ(entityBBox2.getMin());
			CShadowRenderer::m_LightBBox[i].extendZ(entityBBox2.getMax());
			ms_aVisibleShadowCasters[i].push_back(entity);
		}
	}*/
	
	//if (ms_aVisibleLods.size()<4000)
	//	ms_aVisibleLods.push_back(entity);
	
	return CRenderer__AddToLodRenderList(entity, renderDistance);
}

void CRendererRH::AddEntityToShadowCastersIfNeeded(CEntity * entity, bool checkBoundBox)
{
	// TODO: cleanup, improve performance, improve culling heuristics, perhaps add occlusion checks
	int modelID = entity->m_nModelIndex;
	CVector pos = entity->GetPosition();
	float xDist = ms_vecCameraPosition.x - pos.x;
	float yDist = ms_vecCameraPosition.y - pos.y;
	float zDist = ms_vecCameraPosition.z - pos.z;
	float lodStartDist = gShadowSettings.LodShadowsMinDistance*gShadowSettings.LodShadowsMinDistance;
	float renderDistance = xDist * xDist + yDist * yDist + zDist * zDist;

	CBaseModelInfo* modelInfo = CModelInfo::ms_modelInfoPtrs[modelID];
	auto shadowEntity = entity;
	// We add entity to render list only if it has bbox, because we need to cull entities that don't affect objects on screen
	if (shadowEntity!=nullptr && modelInfo != nullptr && modelInfo->m_pColModel != nullptr) {
		// Use lods if entity is not road and it's outside of lod rendering distance
		//if (renderDistance > lodStartDist && !modelInfo->bIsRoad&&shadowEntity->m_pLod!=nullptr)
		//	shadowEntity = shadowEntity->m_pLod;
		if(modelInfo->m_nAlpha==0 && shadowEntity->m_pLod!=nullptr)
			shadowEntity = shadowEntity->m_pLod;
		//if (shadowEntity->m_pRwObject == nullptr)
		//	shadowEntity->CreateRwObject();
		auto entityPos = shadowEntity->GetPosition().ToRwV3d();
		auto entityRadius = modelInfo->m_pColModel->m_boundSphere.m_fRadius;

		// Get entity bounding box
		RW::BBox entityBBox = { RW::V3d{ modelInfo->m_pColModel->m_boundBox.m_vecMin.ToRwV3d() },
			RW::V3d{ modelInfo->m_pColModel->m_boundBox.m_vecMax.ToRwV3d() } };
		// Transform it to world space
		entityBBox += entityPos;
		//DebugRendering::AddToRenderList(new DebugBBox(entityBBox));
		
		
		// Add entity to shadow caster list if it exists, inside frustum bbox and bigger than minimum shadow caster size
		bool isInsideLightBBox=false;
		if (!gShadowSettings.CullPerCascade) {
			// Check if entity is inside any of existing light bounding volumes
			for (auto i = 0; i < gShadowSettings.ShadowCascadeCount; i++)
				isInsideLightBBox |= IsAABBInsideBoundingVolume(ms_aShadowCasterBoundingPlanes[i], 4, entityBBox);
			
			if (isInsideLightBBox)
				AddEntityToShadowCasterList(shadowEntity, renderDistance, 0);
		}
		else {
			for (auto i = 0; i < gShadowSettings.ShadowCascadeCount; i++)
			{
				// Check if entity is inside light bounding volume 
				isInsideLightBBox = IsAABBInsideBoundingVolume(ms_aShadowCasterBoundingPlanes[i], 4, entityBBox);
				if (isInsideLightBBox)
					AddEntityToShadowCasterList(shadowEntity, renderDistance, i);
			}
		}
	}
}

bool CRendererRH::IsAABBInsideBoundingVolume(RwPlane * boundingPlanes, int planeCount, 
											const RW::BBox& b)
{

	for (auto i = 0; i < planeCount; i++)
	{
		RwV3d normal = boundingPlanes[i].normal;
		auto min = b.getMin();
		auto max = b.getMax();
		bool infIsOutside = (min.getX())*normal.x +
							(min.getY())*normal.y +
							(min.getZ())*normal.z -
			boundingPlanes[i].distance < 0;
		bool supIsOutside = max.getX()*normal.x + max.getY()*normal.y + max.getZ()*normal.z - boundingPlanes[i].distance < 0;
		if (infIsOutside && supIsOutside)
			return false;
	}
	return true;
}

RendererVisibility CRendererRH::SetupEntityVisibility(CEntity * entity, float * renderDistance)
{
	return (RendererVisibility)CRenderer__SetupEntityVisibility(entity, renderDistance);

	int modelID = entity->m_nModelIndex;
	CBaseModelInfo* modelInfo = CModelInfo::ms_modelInfoPtrs[modelID];
	CBaseModelInfo* atomicModelInfo = modelInfo->AsAtomicModelInfoPtr();

	int a4 = 1;

	// Don't render vehicles inside tunnels
	if (entity->m_nType == eEntityType::ENTITY_TYPE_VEHICLE && 
		IsCulledByTunnel(entity))
			return RendererVisibility::INVISIBLE;

	if (!atomicModelInfo)
	{
		// if model is not clump and not weapon
		if (modelInfo->GetModelType() != ModelInfoType::MODEL_INFO_CLUMP && modelInfo->GetModelType() != ModelInfoType::MODEL_INFO_WEAPON)
		{
			/*auto playerVeh = reinterpret_cast<CEntity*>(FindPlayerVehicle(-1, false));
			if (playerVeh == entity)
			{
				if (gbFirstPersonRunThisFrame)
				{
					if (CReplay__Mode != 1)
					{
						v9 = TheCamera->Cams[TheCamera->ActiveCam].m_dwDirectionWasLooking;
						if (FindPlayerVehicle(-1, 0)->Class != 9
							|| !(LOBYTE(FindPlayerVehicle(-1, 0)[1].__parent.m_vForce.x) & 0x80))
						{
							if (v9 == 3)
								return 2;
							v10 = a1->m_wModelIndex;
							if (v10 == 432 || v10 == 437 || TheCamera->m_bInATunnelAndABigVehicle)
								return 2;
							if (v9)
								goto LABEL_81;
							v11 = *(LODWORD(entity[16].__parent.placement.pos.x) + 204);
							if (BYTE1(v11) & 0x40)
								return 2;
							if (entity[25].m_pRWObject != 5 || v10 == 453 || v10 == 454 || v10 == 430 || v10 == 460)
							{
							LABEL_81:
								CRenderer::m_pFirstPersonVehicle = entity;
								return 2;
							}
						}
					}
				}
			}*/
			// Don't render object if entity doesn't have rwObject or is invisible or vehicle inside interior?
			if (!entity->m_pRwObject
				|| (!entity->m_bIsVisible && (/*!CMirrors__TypeOfMirror || */entity->m_nModelIndex!=0)) // is invisible and has model index
				|| (!(entity->m_nAreaCode != CGame::currArea && entity->m_nAreaCode != 13) && (entity->m_nType) == eEntityType::ENTITY_TYPE_VEHICLE))
			{
				return RendererVisibility::INVISIBLE;
			}
			if (!entity->GetIsOnScreen() || entity->IsEntityOccluded())
				return RendererVisibility::OFFSCREEN;
			if (!entity->m_bDistanceFade)// If entity is not distance fading, add it to render list.
				return RendererVisibility::VISIBLE;

			//entity->entity.m_Flags.m_dwFlags &= 0xFFFF7FFF;
			
			CVector pos = entity->GetPosition();
			float xDist = ms_vecCameraPosition.x - pos.x;
			float yDist = ms_vecCameraPosition.y - pos.y;
			float zDist = ms_vecCameraPosition.z - pos.z;

			CRendererRH::AddEntityToRenderList(entity, sqrt(xDist*xDist + yDist*yDist + zDist*zDist));
			return RendererVisibility::INVISIBLE;
		}
		goto LABEL_49;
	}

	// if it's time dependent model
	if (modelInfo->GetModelType() == ModelInfoType::MODEL_INFO_TIME)
	{
		/*v17 = (v4->vmt->GetTimeInfo)(v4);
		v18 = *(v17 + 2);
		if (CClock__GetIsTimeInRange(*v17, *(v17 + 1)))
		{
			if (v18 != -1 && CModelInfo::ms_modelInfoPtrs[v18]->m_pRwObject)
				v4->m_Alpha = -1;
		}
		else
		{
			if (v18 == -1 || CModelInfo::ms_modelInfoPtrs[v18]->m_pRwObject)
			{
				(a1->__parent.__vmt.entity->DeleteRwObject)(a1);
				return 0;
			}
			a4 = 0;
		}*/
	LABEL_49:
		//if (entity->entity.m_nbInterior != CGame__currArea && entity->entity.m_nbInterior != 13)
		//	return 0;
		CEntity* lod = entity->m_pLod;

		CVector pos;
		// if lod exists use it's position to calculate distance
		if (lod)
			pos = lod->GetPosition();
		else
			pos = entity->GetPosition();

		float xDist = ms_vecCameraPosition.x - pos.x;
		float yDist = ms_vecCameraPosition.y - pos.y;
		float zDist = ms_vecCameraPosition.z - pos.z;
		*renderDistance = sqrt(xDist*xDist + yDist*yDist + zDist*zDist);
		
		//if (*renderDistance <= 300.0)
			return SetupMapEntityVisibility(entity, modelInfo, *renderDistance, a4);

		float dist = TheCamera.m_fLODDistMultiplier * modelInfo->m_fDrawDistance;
		if (dist > 300.0 && dist + 20.0 > *renderDistance)
			*renderDistance += dist - 300.0f;
		return SetupMapEntityVisibility(entity, modelInfo, *renderDistance, a4);
	}

	//if (!(entity->m_Flags.m_dwFlags & 0x80000))
	//	goto LABEL_49;
	//if (!entity->entity.m_pRWObject || entity->entity.m_Flags.m_dwFlags >= 0 && (entity->entity.m_wModelIndex))
	//	return 0;
	//if (!CEntity::GetIsOnScreen(a1) || CEntity::IsEntityOccluded(a1))
	//	return 2;
	//if (!(BYTE1(entity->entity.m_Flags.m_dwFlags) & 0x40))
	//	return RendererVisibility::VISIBLE;
	CVector pos = entity->GetPosition();
	float xDist = ms_vecCameraPosition.x - pos.x;
	float yDist = ms_vecCameraPosition.y - pos.y;
	float zDist = ms_vecCameraPosition.z - pos.z;
	*renderDistance = sqrt(xDist*xDist + yDist*yDist + zDist*zDist);
	AddEntityToRenderList(entity, *renderDistance);
	//CVisibilityPlugins::InsertEntityIntoSortedList(a1, v27);
	//entity->entity.m_Flags.m_dwFlags &= 0xFFFF7FFF;
	return RendererVisibility::INVISIBLE;
}

RendererVisibility CRendererRH::SetupBigBuildingVisibility(CEntity * entity, float * renderDistance)
{
	return (RendererVisibility)CRenderer__SetupBigBuildingVisibility(entity, renderDistance);
	auto modelInfo = CModelInfo::ms_modelInfoPtrs[entity->m_nModelIndex];
	bool b = 1;
	if (entity->m_nAreaCode != CGame::currArea && entity->m_nAreaCode != 13)
		return RendererVisibility::INVISIBLE;
	if (modelInfo->GetModelType() == ModelInfoType::MODEL_INFO_TIME)
	{
		auto timeInfo = modelInfo->GetTimeInfo();
		auto nextTimeID = timeInfo->m_wOtherTimeModel;
		if (CClock::GetIsTimeInRange(timeInfo->m_nTimeOff, timeInfo->m_nTimeOn))
		{
			if (nextTimeID != -1 && CModelInfo::ms_modelInfoPtrs[nextTimeID]->m_pRwObject)
				modelInfo->m_nAlpha = 0xFF;
		}
		else
		{
			if (nextTimeID == -1 || CModelInfo::ms_modelInfoPtrs[nextTimeID]->m_pRwObject)
			{
				entity->DeleteRwObject();
				return RendererVisibility::INVISIBLE;
			}
			b = 0;
		}
	}
	else if (modelInfo->GetModelType() == ModelInfoType::MODEL_INFO_VEHICLE)
	{
		return entity->IsVisible() && !entity->IsEntityOccluded() ? 
			RendererVisibility::VISIBLE : RendererVisibility::INVISIBLE;
	}
	auto entityPos = entity->m_pLod !=nullptr ? entity->m_pLod->GetPosition() : entity->GetPosition();

	auto xDist = entityPos.x - ms_vecCameraPosition.x;
	auto yDist = entityPos.y - ms_vecCameraPosition.y;
	auto zDist = entityPos.z - ms_vecCameraPosition.z;
	*renderDistance = sqrt(xDist * xDist	+ yDist* yDist + zDist* zDist);
	if (entity->m_nNumLodChildrenRendered <= 0)
	{
		auto mapVis = SetupMapEntityVisibility(entity, modelInfo, *renderDistance, b);
		if (mapVis != RendererVisibility::VISIBLE || entity->m_nNumLodChildren <= 1u)
			return mapVis;
		if (entity->m_pLod && modelInfo->m_nAlpha == 0xFFu)
			++entity->m_pLod->m_nNumLodChildrenRendered;
		AddToLodRenderList(entity, *renderDistance);
		return RendererVisibility::INVISIBLE;
	}
	if (entity->m_pLod)
		++entity->m_pLod->m_nNumLodChildrenRendered;
	if (entity->m_nNumLodChildren <= 1u)
	{
		entity->m_nNumLodChildrenRendered = 0;
	}
	else
	{
		ms_pLodRenderList->entity = entity;
		ms_pLodRenderList->distance = *renderDistance;
		ms_pLodRenderList++;
	}
	if (!entity->m_pRwObject)
		entity->CreateRwObject();
	return RendererVisibility::NOT_LOADED;
}

RendererVisibility CRendererRH::SetupMapEntityVisibility(CEntity * entity, CBaseModelInfo * mapInfo, float renderDistance, char a4)
{
	//return CRenderer__SetupMapEntityVisibility(entity,mapInfo,renderDistance,a4);
	CEntity* pLOD_Object = entity->m_pLod;
	RwObject* rwObject   = mapInfo->m_pRwObject;
	float maxDrawDist = mapInfo->m_pColModel->m_boundSphere.m_fRadius + CRenderer__ms_fFarClipPlane;
	float distance = 20.0;
	float drawDist = *&TheCamera.m_fLODDistMultiplier * mapInfo->m_fDrawDistance;

	if (drawDist >= maxDrawDist)
		drawDist = maxDrawDist;
	UINT flags = 0;//entity->m_Flags.m_dwFlags;
	// Cull everything that is outside
	if (IsCulledByTunnel(entity))
		return RendererVisibility::INVISIBLE;
	if (!pLOD_Object)
	{
		float mapInfoDrawDist = mapInfo->m_fDrawDistance;
		if (mapInfoDrawDist >= drawDist)
			mapInfoDrawDist = drawDist;
		if (mapInfoDrawDist > 150.0f)
			distance = mapInfoDrawDist / 15.0f + 10.0f;

		//if (entity->entity.m_Flags.m_bIsBIGBuilding)
			//drawDist *= CRenderer::ms_lowLodDistScale;
	}
	if (!rwObject)
	{
		if (pLOD_Object && pLOD_Object->m_nNumLodChildren > 1u && distance + renderDistance - 20.0 < drawDist)
		{
			CRendererRH::AddToLodRenderList(entity, renderDistance);
			return RendererVisibility::NOT_LOADED;
		}
		goto LABEL_39;
	}
	if (distance + renderDistance - 20.0 >= drawDist)
	{
	LABEL_39:
		//if (entity->entity.m_Flags.m_bDontStream)
		//	return 0;
		if (rwObject && renderDistance - 20.0 < drawDist)
		{
			// If rwObject inside entity is none, try to create one
			if (!entity->m_pRwObject)
			{
				entity->CreateRwObject();
				if(!entity->m_pRwObject)
					return RendererVisibility::INVISIBLE;
			}
			//if (flags&0xFFFF >= 0)
			//	return 0;
			/*if (!CEntity::GetIsOnScreen(v4) || CEntity::IsEntityOccluded(v4))
			{
				v17 = mapInfo->m_wFlags;
				if (!(v17 & 1))
					mapInfo->m_Alpha = -1;
				mapInfo->m_wFlags = v17 & 0xFFFE;
				result = 0;
			}
			else
			{*/
				//entity->entity.m_Flags.m_bDistanceFade = true;
				if (entity->m_pLod && entity->m_pLod->m_nNumLodChildren > 1u)
					AddToLodRenderList(entity, renderDistance);
				else
					AddEntityToRenderList(entity, renderDistance);
			//}
			return RendererVisibility::INVISIBLE;
		}
		//if (renderDistance - 50.0 >= drawDist || !a4 /*|| flags >= 0*/)
		//	return 0;
		if (!entity->m_pRwObject)
			entity->CreateRwObject();
		return RendererVisibility::NOT_LOADED;
	}
	if (!entity->m_pRwObject)
	{
		entity->CreateRwObject();
		if (!entity->m_pRwObject)
			return RendererVisibility::INVISIBLE;
	}
	//if (entity->entity.m_Flags.m_dwFlags&0xFFFF >= 0)
	//	return 0;
	//if (CEntity::GetIsOnScreen(v4) && !CEntity::IsEntityOccluded(v4))
	//{
		/*if (mapInfo->m_nAlpha == 0xFF)
			entity->entity.m_Flags.m_bDistanceFade = false;
		else
			entity->entity.m_Flags.m_bDistanceFade = true;*/
		if (!pLOD_Object)
			return RendererVisibility::VISIBLE;
		if (mapInfo->m_nAlpha == 0xFF)
			++pLOD_Object->m_nNumLodChildrenRendered;
		if (pLOD_Object->m_nNumLodChildren <= 1u)
			return RendererVisibility::VISIBLE;
		//CRenderer::AddEntityToRenderList(entity, renderDistance);
		CRendererRH::AddToLodRenderList(entity, renderDistance);
		return RendererVisibility::INVISIBLE;
	//}
	if (!(mapInfo->m_nFlags & 1))
		mapInfo->m_nAlpha = 0xFF;
	mapInfo->m_nFlags = mapInfo->m_nFlags & 0xFFFE;
	return RendererVisibility::OFFSCREEN;
}

void CRendererRH::RenderOneRoad(CEntity * entity)
{
	if (entity->m_nType == eEntityType::ENTITY_TYPE_VEHICLE)
		((CVehicle*)entity)->SetupRender();
	
	((void(__cdecl *)(CEntity *))0x553230)(entity);
	if (entity->m_nType == eEntityType::ENTITY_TYPE_VEHICLE)
		((CVehicle*)entity)->ResetAfterRender();
	
}

//6D64F0     ; void __thiscall CVehicle::SetupRender(CVehicle *this)
void CRendererRH::RenderOneNonRoad(CEntity * entity)
{
	if ((entity->m_nType) == eEntityType::ENTITY_TYPE_VEHICLE) {
		CVehicle__SetupRender(entity);
	}
	entity->Render();
	if ((entity->m_nType) == eEntityType::ENTITY_TYPE_VEHICLE) {
		CVehicle__ResetAfterRender(entity);
	}
}

bool CRendererRH::IsCulledByTunnel(CEntity * entity)
{
	return  (!CRenderer__ms_bRenderTunnels		 &&	 entity->m_bTunnel ||
			!CRenderer__ms_bRenderOutsideTunnels && !entity->m_bTunnel);
}

void CRendererRH::ScanSectorList(int x, int y)
{
	// Currently standard version is used, to many bugs so far
	//CRenderer__ScanSectorList(x, y);
	//return;
	
	bool loadIfRequired = false;
	bool dontRenderSectorEntities = false;
	// If sector id is wrong than quit scanning.
	if (x < 0 || y < 0 || x >= 120 || y >= 120)
		dontRenderSectorEntities = true;
	CSector*	  sector	   = GetSector(x, y);
	CRepeatSector* repeatSector = GetRepeatSector(x,y);
	// Calculate sector world space position, there are 120 sectors with size of 50 units 
	// TODO: move it out of this method
	float sectorPosX = (x - 60) * 50.0f + 25.0f;
	float sectorPosY = (y - 60) * 50.0f + 25.0f;

	// Calculate distance to camera
	float camDistX = sectorPosX - ms_vecCameraPosition.x;
	float camDistY = sectorPosY - ms_vecCameraPosition.y;
	if (camDistY * camDistY + camDistX * camDistX < 10000.0f
		|| (fabs(CGeneral::LimitRadianAngle(atan2(camDistY, -camDistX) - ms_fCameraHeading)) < 0.36f))
	{
		loadIfRequired = true;
	}

	if (!dontRenderSectorEntities) 
		ScanPtrList(&sector->m_buildings, loadIfRequired);
	
	ScanPtrList(&repeatSector->m_lists[REPEATSECTOR_OBJECTS], loadIfRequired);
	ScanPtrList(&repeatSector->m_lists[REPEATSECTOR_VEHICLES], loadIfRequired);
	ScanPtrList(&repeatSector->m_lists[REPEATSECTOR_PEDS], loadIfRequired);

	if (!dontRenderSectorEntities)
		ScanPtrList(&sector->m_dummies, loadIfRequired);
	
}

// Scans entity pointer list, and adds models to render list.
void CRendererRH::ScanPtrList(CPtrList* ptrList, bool loadIfRequired)
{
	// If entity list is empty, than return. 
	if (ptrList == nullptr)
		return;
	auto current = ptrList->GetNode();
	CEntity* entity;
	while (current!=nullptr)
	{
		entity = (CEntity*)current->pItem;
		// If this entity has the same scan code as current, than we could skip it
		if (entity->m_nScanCode == CWorld__ms_nCurrentScanCode) {
			current = current->pNext;
			continue;
		}
		float renderDistance;

		entity->m_nScanCode = CWorld__ms_nCurrentScanCode;
		
		// entity is on screen by default
		entity->m_bOffscreen = false;
			
		RendererVisibility visibility = SetupEntityVisibility(entity, &renderDistance);
		if(visibility != RendererVisibility::NOT_LOADED)
			AddEntityToShadowCastersIfNeeded(entity, false);
		int modelID = entity->m_nModelIndex;
		
		// Depending on visibility value, request model, add to render list or invisible render list
		switch (visibility)
		{
		case RendererVisibility::NOT_LOADED:
			// don't request model if streaming is disabled or entity is offscreen or we are in the sky
			if (CStreaming::ms_disableStreaming || !entity->GetIsOnScreen() || ms_bInTheSky)
				break;
			// if model is loaded and loading priority is 0 or there is no priority requests
			if ((CStreaming::ms_aInfoForModel[modelID].m_nLoadState == eStreamingLoadState::LOADSTATE_LOADED)
				&&
				(loadIfRequired || m_loadingPriority == 0 || CStreaming::ms_numPriorityRequests < 1))
			{
				CStreaming::RequestModel(modelID, 0);
				break;
			}
			// if entity is not occluded
			if (!entity->IsEntityOccluded() && loadIfRequired)
			{
				m_loadingPriority = 1;
				CStreaming::RequestModel(modelID, 0);
				break;
			}
			if (m_loadingPriority == 0 || CStreaming::ms_numPriorityRequests < 1)
				CStreaming::RequestModel(modelID, 0);
			break;
		case RendererVisibility::VISIBLE:
			AddEntityToRenderList(entity, renderDistance);
			break;
		case RendererVisibility::INVISIBLE:
			if (entity->m_nType == eEntityType::ENTITY_TYPE_OBJECT)
			{
				CBaseModelInfo* modelInfo = CModelInfo::ms_modelInfoPtrs[modelID];
				CBaseModelInfo* atomicModelInfo = modelInfo->AsAtomicModelInfoPtr();
				if (atomicModelInfo)
				{
					auto modelInfoFlags = atomicModelInfo->m_nFlags & 0x7800;
					if (modelInfoFlags & 0x2000 || modelInfoFlags & 0x2800)
					{
						entity->m_bOffscreen = true;
						if (entity->m_bHasPreRenderEffects)
						{
							auto pos = entity->GetPosition();
							auto maxDist = 30.0f;

							if (abs(pos.x - ms_vecCameraPosition.x) < maxDist && abs(pos.y - ms_vecCameraPosition.y) < maxDist) {
								if (ms_nNoOfInVisibleEntities < 149)
								{
									ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities] = entity;
									ms_nNoOfInVisibleEntities++;
								}
							}

						}
					}
				}
			}else
				AddEntityToRenderList(entity, renderDistance);
			break;
		case RendererVisibility::OFFSCREEN:
			entity->m_bOffscreen = true;
			//AddEntityToShadowCasterList(entity, renderDistance);
			if (entity->m_bHasPreRenderEffects) {
				auto pos = entity->GetPosition();
				auto maxDist = 30.0;
				if ((entity->m_nType == eEntityType::ENTITY_TYPE_VEHICLE &&
					((CVehicle*)entity)->m_nFlags.bAlwaysSkidMarks))
					maxDist = 200.0f;
				if (abs(pos.x - ms_vecCameraPosition.x) < maxDist&&abs(pos.y - ms_vecCameraPosition.y) < maxDist) {
					if (ms_nNoOfInVisibleEntities < 149)
					{
						ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities] = entity;
						ms_nNoOfInVisibleEntities++;
					}
				}
			}
			break;
		default:
			break;
		}
		current = current->pNext;
	}
}
// Scans entity pointer list, and adds models to render list.
void CRendererRH::ScanPtrListForShadows(CPtrList* ptrList, bool loadIfRequired)
{
	// If entity list is empty, than return. 
	if (ptrList == nullptr)
		return;
	auto current = ptrList->GetNode();
	while (current != nullptr)
	{
		CEntity* entity = (CEntity*)current->pItem;
		// If this entity has the same scan code as current, than we could skip it
		if (entity->m_nScanCode == CWorld__ms_nCurrentScanCode) {
			current = current->pNext;
			continue;
		}
		entity->m_nScanCode = CWorld__ms_nCurrentScanCode;

		AddEntityToShadowCastersIfNeeded(entity,true);
		current = current->pNext;
	}
}

void CRendererRH::ScanPtrListForReflections(CPtrList * ptrList, bool loadIfRequired)
{
	// If entity list is empty, than return. 
	if (ptrList == nullptr)
		return;
	auto current = ptrList->GetNode();
	CEntity* entity;
	while (current != nullptr)
	{
		entity = (CEntity*)current->pItem;
		// If this entity has the same scan code as current, than we could skip it
		if (entity->m_nScanCode == CWorld__ms_nCurrentScanCode) {
			current = current->pNext;
			continue;
		}
		entity->m_nScanCode = CWorld__ms_nCurrentScanCode;
		if (entity->m_nType == eEntityType::ENTITY_TYPE_BUILDING || entity->m_nType == eEntityType::ENTITY_TYPE_OBJECT) {
			CVector entityPos = entity->m_pLod != nullptr ? entity->m_pLod->GetPosition() : entity->GetPosition();
			CVector playerPos = FindPlayerCoors(0);
			auto xDist = entityPos.x - playerPos.x;
			auto yDist = entityPos.y - playerPos.y;
			auto zDist = entityPos.z - playerPos.z;
			float renderDistance = sqrt(xDist * xDist + yDist * yDist + zDist * zDist);
			AddEntityToReflectionList(entity, renderDistance);
		}
		current = current->pNext;
	}
}

void CRendererRH::ScanBigBuildingList(int x, int y)
{
	//CRenderer__ScanBigBuildingList(x, y);
	//return;
	bool loadIfRequired = false;
	if (x < 0 || y < 0 || x >= 30 || y >= 30)
		return;
	
	auto lodPtrList = GetLodPtrList(x,y);
	auto camDistX = (x - 15) * 200.0f + 100.0f - ms_vecCameraPosition.x;
	auto camDistY = (y - 15) * 200.0f + 100.0f - ms_vecCameraPosition.y;
	if (camDistX * camDistX + camDistY * camDistY < 80000.0f
		|| fabs(CGeneral::LimitRadianAngle(atan2(camDistY, -camDistX) - ms_fCameraHeading)) <= 0.7f)
	{
		loadIfRequired = true;
	}
	auto current = lodPtrList->GetNode();
	for ( ; current!=nullptr; current = current->pNext)
	{
		auto entity = (CEntity*)current->pItem;
		if (entity->m_nScanCode != CWorld__ms_nCurrentScanCode)
		{
			entity->m_nScanCode = CWorld__ms_nCurrentScanCode;
			float distance;
			
			auto visibility = SetupBigBuildingVisibility(entity, &distance)-1;
			if (visibility) {
				if (visibility == 2 && !CStreaming::ms_disableStreaming)
				{
					if (loadIfRequired)
						CStreaming::RequestModel(entity->m_nModelIndex, 0);
				}
			}
			else
			{
				float dist = distance + 0.01f;
				AddEntityToShadowCastersIfNeeded(entity, true);
				AddEntityToRenderList(entity, dist);
				entity->m_bOffscreen = false;
			}
		}
	}
	//ScanSectorList(x, y);
	
	
}

void CRendererRH::ScanSectorListForReflections(int x, int y)
{
	CSector*	  sector = GetSector(x, y);
	CRepeatSector* repeatSector = GetRepeatSector(x, y);

	ScanPtrListForReflections(&sector->m_buildings, false);
	ScanPtrListForReflections(&repeatSector->m_lists[REPEATSECTOR_OBJECTS], false);
}

void CRendererRH::ScanSectorListForShadowCasters(int x, int y)
{
	bool dontRenderSectorEntities = false;
	// If sector id is wrong than quit scanning.
	if (x < 0 || y < 0 || x >= 120 || y >= 120)
		dontRenderSectorEntities = true;
	float sectorPosX = (x - 60) * 50.0f + 25.0f;
	float sectorPosY = (y - 60) * 50.0f + 25.0f;

	// Calculate distance to camera
	float camDistX = sectorPosX - ms_vecCameraPosition.x;
	float camDistY = sectorPosY - ms_vecCameraPosition.y;
	float distanceToSector = sqrt(camDistY * camDistY + camDistX * camDistX);
//	RW::BBox	 sectorBBox = { { sectorPosX + 25.0 , sectorPosY + 25.0,3000 },{ sectorPosX - 25.0 , sectorPosY - 25.0,-3000 } };

	/*if (distanceToSector >= CRenderer__ms_fFarClipPlane)
		return;*/
	CSector*	  sector		= GetSector(x, y);
	CRepeatSector* repeatSector = GetRepeatSector(x, y);
	// TODO: Add sector skip to improve performance
	if (!dontRenderSectorEntities)
		ScanPtrListForShadows(&sector->m_buildings, false);

	//ScanPtrListForShadows(&repeatSector->m_lists[0], loadIfRequired);
	//ScanPtrListForShadows(&repeatSector->m_lists[1], loadIfRequired);
	//ScanPtrListForShadows(&repeatSector->m_lists[2], loadIfRequired);

	//if (!dontRenderSectorEntities)
	//	ScanPtrListForShadows(sector->m_dummies, loadIfRequired);
}

void CRendererRH::PreRender()
{
	//if (ms_nNoOfVisibleEntities == 0) {
		for (auto entity : ms_aVisibleEntities)
			entity->PreRender();
		//for (auto entity : ms_aVisibleLods)
		//	((void(__thiscall *)(CEntity *))entity->_vmt->preRender)(entity);
	/*}
	else {*/
		for (size_t i = 0; i < ms_nNoOfVisibleEntities; ++i)
			ms_aVisibleEntityPtrs[i]->PreRender();
		for (size_t i = 0; i < ms_nNoOfVisibleLods; ++i)
			ms_aVisibleLodPtrs[i]->PreRender();
	//}
	
}

void CRendererRH::ProcessLodRenderLists()
{
	if (ms_pLodRenderList == GetLodRenderListBase())
		return;
	// first remove all invisible entities from lod list
	for (auto current = GetLodRenderListBase(); current != ms_pLodRenderList; current++) {
		auto entity = current->entity;
		if (entity && !entity->m_bIsVisible) {
			entity->m_nNumLodChildrenRendered = 0;
			current->entity = nullptr;
		}
	}
	// than remove all rendered lods and request all unloaded lods
	bool renderedLodsExist = true;
	while(renderedLodsExist){
		renderedLodsExist = false;
		for (auto current = GetLodRenderListBase(); current != ms_pLodRenderList; current++) {
			auto entity = current->entity;

			if (entity) {
				auto numLodChildren = (char)entity->m_nNumLodChildren;
				// if every lod children has been rendered, than
				if (numLodChildren > 0 && entity->m_nNumLodChildrenRendered == numLodChildren)
				{
					entity->m_nNumLodChildrenRendered = 0;
					current->entity = nullptr;
					renderedLodsExist = true;
				}
				else {
					auto lod = entity->m_pLod;
					if (lod)
					{
						if (CModelInfo::ms_modelInfoPtrs[entity->m_nModelIndex]->m_nAlpha < 0xFFu && 
							lod->m_nNumLodChildrenRendered != 0x80u && lod->m_bDisplayedSuperLowLOD)
							lod->m_nNumLodChildrenRendered = 0;

						if (entity->m_pRwObject == nullptr)
						{
							if (lod->m_bDisplayedSuperLowLOD)
								lod->m_nNumLodChildrenRendered = 0x80u;
							current->entity = nullptr;
							entity->m_nNumLodChildrenRendered = 0;
							CStreaming::RequestModel(entity->m_nModelIndex, 0);
						}
					}
				}
			}
		}
	}
	// remove all rendered lods
	for (auto current = GetLodRenderListBase(); current != ms_pLodRenderList; current++) {
		auto entity = current->entity;
		if (entity && entity->m_nNumLodChildrenRendered > 0) {
			entity->m_bDisplayedSuperLowLOD = false;
			entity->m_nNumLodChildrenRendered = 0;
			current->entity = nullptr;
		}
	}
	// add last lods to renderlist
	for (auto current = GetLodRenderListBase(); current != ms_pLodRenderList; current++) {
		auto entity = current->entity;
		if (entity) {
			if (entity->m_nNumLodChildrenRendered == 0x80u) {
				entity->m_bDisplayedSuperLowLOD = true;
				if (CModelInfo::ms_modelInfoPtrs[entity->m_nModelIndex]->m_nAlpha != 0xFF)
					entity->m_bDistanceFade = true;					
				AddEntityToRenderList(entity, current->distance);
			}
			entity->m_nNumLodChildrenRendered = 0;
		}
	}
}

static RwIm3DVertex LineList[14];

void CRendererRH::ScanWorld(RwCamera* camera, RwV3d* gameCamPos, float shadowStart, float shadowEnd)
{
	CRenderer__ScanWorld();
	return;
	auto farPlane = camera->farPlane;
	auto viewWindow = camera->viewWindow;
	//viewWindow.x = 1.0;
	//viewWindow.y = 1.0;
	RwV3d points[13];
	points[0].x = points[0].y = points[0].z = 0;
	
	points[1].x = -(farPlane * viewWindow.x);
	points[1].y = farPlane * viewWindow.y;
	points[1].z = farPlane;

	points[2].x = farPlane * viewWindow.x;
	points[2].y = farPlane * viewWindow.y;
	points[2].z = farPlane;

	points[3].x = farPlane * viewWindow.x;
	points[3].y = -(farPlane * viewWindow.y);
	points[3].z = farPlane;

	points[4].x = -(farPlane * viewWindow.x);
	points[4].y = -(farPlane * viewWindow.y);
	points[4].z = farPlane;
	memset(&points[5], 0, 96u);

	RwMatrix* m = &(((RwFrame*)camera->object.object.parent)->modelling);
	m_pFirstPersonVehicle = nullptr;
	CVisibilityPlugins__InitAlphaEntityList();
	// Resets scan codes if it exceeds 2^16
	SetNextScanCode();

	points[5].x = -(viewWindow.x) * 300.0f;
	points[5].y = viewWindow.y * 300.0f;
	points[5].z = 300.0f;

	points[6].x = viewWindow.x * 300.0f;
	points[6].y = viewWindow.y * 300.0f;
	points[6].z = 300.0f;

	points[7].x = -(viewWindow.x) * 60.0f;
	points[7].y = viewWindow.y * 60.0f;
	points[7].z = 300.0f;

	points[8].x = viewWindow.x * 60.0f;
	points[8].y = viewWindow.y * 60.0f;
	points[8].z = 300.0f;

	points[9].x = viewWindow.x * 300.0f;
	points[9].y = -(viewWindow.y) * 300.0f;
	points[9].z = 300.0f;

	points[10].x = -(viewWindow.x) * 300.0f;
	points[10].y = -(viewWindow.y) * 300.0f;
	points[10].z = 300.0f;

	points[11].x = viewWindow.x * 	60.0f;
	points[11].y = -(viewWindow.y) * 60.0f;
	points[11].z = 300.0f;

	points[12].x = -(viewWindow.x) * 60.0f;
	points[12].y = -(viewWindow.y) * 60.0f;
	points[12].z = 300.0f;

	RwV3dTransformPoints(points, points, 13, m);
	m_loadingPriority = 0;

	RwBBox bbox = camera->frustumBoundBox;

	RwV2d sector[5];
	sector[0].x = points[0].x * 0.02f + 60.0f;
	sector[0].y = points[0].y * 0.02f + 60.0f;
	sector[1].x = points[5].x * 0.02f + 60.0f;
	sector[1].y = points[5].y * 0.02f + 60.0f;
	sector[2].x = points[6].x * 0.02f + 60.0f;
	sector[2].y = points[6].y * 0.02f + 60.0f;
	sector[3].x = points[9].x * 0.02f + 60.0f;
	sector[3].y = points[9].y * 0.02f + 60.0f;
	sector[4].x = points[10].x * 0.02f + 60.0f;
	sector[4].y = points[10].y * 0.02f + 60.0f;
	ms_aVisibleEntities.clear();
	ms_aVisibleLods.clear();
	for (int i = 0; i < 4; i++)
		ms_aVisibleShadowCasters[i].clear();
	//RwBBox bbox;
	//GenerateCameraBBox(camera, bbox);
	/*for (int x = 0; x < 120; x++)
		for (int y = 0; y < 120; y++)
			CRenderer::ScanSectorList(camera, camera->frustumBoundBox, 0, CRenderer__ms_fFarClipPlane,x, y);*/
	CWorldScan__ScanWorld(sector, 5, CRendererRH::ScanSectorList);

	sector[0].x = points[0].x * 0.005f + 15.0f;
	sector[0].y = points[0].y * 0.005f + 15.0f;
	sector[1].x = points[1].x * 0.005f + 15.0f;
	sector[1].y = points[1].y * 0.005f + 15.0f;
	sector[2].x = points[2].x * 0.005f + 15.0f;
	sector[2].y = points[2].y * 0.005f + 15.0f;
	sector[3].x = points[3].x * 0.005f + 15.0f;
	sector[3].y = points[3].y * 0.005f + 15.0f;
	sector[4].x = points[4].x * 0.005f + 15.0f;
	sector[4].y = points[4].y * 0.005f + 15.0f;
	CWorldScan__ScanWorld(sector, 5, CRendererRH::ScanBigBuildingList);
	// Resets scan codes if it exceeds 2^16
	if (CWorld__ms_nCurrentScanCode >= 0xFFFF)
	{
		CWorld__ClearScanCodes();
		CWorld__ms_nCurrentScanCode = 1;
	}
	else
		++CWorld__ms_nCurrentScanCode;
	float minX = ms_vecCameraPosition.x- farPlane;
	float maxX = ms_vecCameraPosition.x+ farPlane;
	float minY = ms_vecCameraPosition.y- farPlane;
	float maxY = ms_vecCameraPosition.y+ farPlane;
	sector[0].x = minX * 0.02f + 60.0f;
	sector[0].y = minY * 0.02f + 60.0f;
	sector[1].x = minX * 0.02f + 60.0f;
	sector[1].y = maxY * 0.02f + 60.0f;
	sector[2].x = maxX * 0.02f + 60.0f;
	sector[2].y = minY * 0.02f + 60.0f;
	sector[3].x = maxX * 0.02f + 60.0f;
	sector[3].y = maxY * 0.02f + 60.0f;
	CWorldScan__ScanWorld(sector, 4, CRendererRH::ScanSectorListForShadowCasters);


	/*for (int x = 0; x < 120; x++)
		for (int y = 0; y < 120; y++)
			CRenderer::ScanSectorListForShadowCasters(x, y);*/
	/*for (int i = 0; i<5; i++)
	{
		RwIm3DVertexSetPos(&LineList[i],
			sector[i].x, sector[i].y, 0);
	}
	if (RwIm3DTransform(LineList, 5, nullptr, 0))
	{
		RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

		RwIm3DEnd();
	}*/
}

void CRendererRH::ScanWorld()
{
	RwV3d points[17];
	auto viewWindow = TheCamera.m_pRwCamera->viewWindow;
	float farPlane = TheCamera.m_pRwCamera->farPlane;
	points[0].x = 0;
	points[0].y = 0;
	points[0].z = 0;// -TheCamera.m_pRwCamera->farPlane/4;
	// first 4 far frustum plane points in camera space
	points[1].x = -(farPlane * viewWindow.x);
	points[1].y = farPlane * viewWindow.y;
	points[1].z = farPlane;

	points[2].x = farPlane * viewWindow.x;
	points[2].y = farPlane * viewWindow.y;
	points[2].z = farPlane;

	points[3].x = farPlane * viewWindow.x;
	points[3].y = -(farPlane * viewWindow.y);
	points[3].z = farPlane;

	points[4].x = -(farPlane * viewWindow.x);
	points[4].y = -(farPlane * viewWindow.y);
	points[4].z = farPlane;
	memset(&points[5], 0, (12*12));
	
	RwMatrix* m = &(RwCameraGetFrame(TheCamera.m_pRwCamera)->modelling);
	m_pFirstPersonVehicle = nullptr;
	CVisibilityPlugins::InitAlphaEntityList();
	
	points[5].x = -(viewWindow.x) * 300.0f;
	points[5].y = viewWindow.y * 300.0f;
	points[5].z = 300.0f;

	points[6].x = viewWindow.x * 300.0f;
	points[6].y = viewWindow.y * 300.0f;
	points[6].z = 300.0f;

	points[7].x = -(viewWindow.x) * 60.0f;
	points[7].y = viewWindow.y * 60.0f;
	points[7].z = 300.0f;

	points[8].x = viewWindow.x * 60.0f;
	points[8].y = viewWindow.y * 60.0f;
	points[8].z = 300.0f;

	points[9].x = viewWindow.x * 300.0f;
	points[9].y = -(viewWindow.y) * 300.0f;
	points[9].z = 300.0f;

	points[10].x = -(viewWindow.x) * 300.0f;
	points[10].y = -(viewWindow.y) * 300.0f;
	points[10].z = 300.0f;

	points[11].x = viewWindow.x * 	60.0f;
	points[11].y = -(viewWindow.y) * 60.0f;
	points[11].z = 300.0f;

	points[12].x = -(viewWindow.x) * 60.0f;
	points[12].y = -(viewWindow.y) * 60.0f;
	points[12].z = 300.0f;

	points[13].x = 0;
	points[13].y = 0;
	points[13].z = -300.0f;

	//points[13].x = -viewWindow.x  * 300.0f;
	//points[13].y = viewWindow.y * 300.0f;
	//points[13].z = -300.0f;

	points[14].x = viewWindow.x * 300.0f;
	points[14].y = viewWindow.y * 300.0f;
	points[14].z = -300.0f;

	points[15].x =   viewWindow.x  * 300.0f;
	points[15].y = -(viewWindow.y) * 300.0f;
	points[15].z = -300.0f;

	points[16].x = -(viewWindow.x) * 300.0f;
	points[16].y = -(viewWindow.y) * 300.0f;
	points[16].z = -300.0f;
	
	RwV3dTransformPoints(points, points, 17, m);
	m_loadingPriority = 0;
	ms_aVisibleReflectionObjects.clear();
	for (int i = 0; i < 4; i++)
		ms_aVisibleShadowCasters[i].clear();
	for (auto i = 0; i < 4; i++)
		CalculateShadowBoundingPlanes(i);
	int currentSectorX = (int)ceil((ms_vecCameraPosition.x - 25.0f) / 50.0f + 60.0f);
	int currentSectorY = (int)ceil((ms_vecCameraPosition.y - 25.0f) / 50.0f + 60.0f);
	// TODO: Add reflections culling
	// Resets scan codes if it exceeds 2^16
	SetNextScanCode();
	// TODO: replace to reflection settings
	for (int x = -gShadowSettings.MaxSectorsAroundPlayer; x < gShadowSettings.MaxSectorsAroundPlayer + 1; x++)
	{
		for (int y = -gShadowSettings.MaxSectorsAroundPlayer; y < gShadowSettings.MaxSectorsAroundPlayer + 1; y++) {
			CRendererRH::ScanSectorListForReflections(currentSectorX+x, currentSectorY+y);
		}
	}
	

	// Resets scan codes if it exceeds 2^16
	SetNextScanCode();
	m_loadingPriority = 0;
	CVisibilityPlugins::InitAlphaEntityList();
	RwV2d sector[5];
	sector[0].x = points[0].x * 0.02f + 60.0f;
	sector[0].y = points[0].y * 0.02f + 60.0f;
	sector[1].x = points[5].x * 0.02f + 60.0f;
	sector[1].y = points[5].y * 0.02f + 60.0f;
	sector[2].x = points[6].x * 0.02f + 60.0f;
	sector[2].y = points[6].y * 0.02f + 60.0f;
	sector[3].x = points[9].x * 0.02f + 60.0f;
	sector[3].y = points[9].y * 0.02f + 60.0f;
	sector[4].x = points[10].x * 0.02f + 60.0f;
	sector[4].y = points[10].y * 0.02f + 60.0f;

	CWorldScan__ScanWorld(sector, 5, CRendererRH::ScanSectorList);

	sector[0].x = points[0].x * 0.005f + 15.0f;
	sector[0].y = points[0].y * 0.005f + 15.0f;
	sector[1].x = points[1].x * 0.005f + 15.0f;
	sector[1].y = points[1].y * 0.005f + 15.0f;
	sector[2].x = points[2].x * 0.005f + 15.0f;
	sector[2].y = points[2].y * 0.005f + 15.0f;
	sector[3].x = points[3].x * 0.005f + 15.0f;
	sector[3].y = points[3].y * 0.005f + 15.0f;
	sector[4].x = points[4].x * 0.005f + 15.0f;
	sector[4].y = points[4].y * 0.005f + 15.0f;

	CWorldScan__ScanWorld(sector, 5, CRendererRH::ScanBigBuildingList);
	// Scan for shadowcasters
	
	//SetNextScanCode();
	float minX = ms_vecCameraPosition.x - farPlane / 2;
	float maxX = ms_vecCameraPosition.x + farPlane / 2;
	float minY = ms_vecCameraPosition.y - farPlane / 2;
	float maxY = ms_vecCameraPosition.y + farPlane / 2;
	sector[0].x = points[13].x * 0.005f + 15.0f;
	sector[0].y = points[13].y * 0.005f + 15.0f;
	sector[1].x = points[1].x * 0.005f + 15.0f;
	sector[1].y = points[1].y * 0.005f + 15.0f;
	sector[2].x = points[2].x * 0.005f + 15.0f;
	sector[2].y = points[2].y * 0.005f + 15.0f;
	sector[3].x = points[3].x * 0.005f + 15.0f;
	sector[3].y = points[3].y * 0.005f + 15.0f;
	sector[4].x = points[4].x * 0.005f + 15.0f;
	sector[4].y = points[4].y * 0.005f + 15.0f;
	
	if (CGame::currArea != 0 || (CSAIdleHook::m_fShadowDNBalance >= 1.0) || !gShadowSettings.ScanShadowsBehindPlayer)
		return;
	
	m_loadingPriority = 0;
	/*std::thread *tt = new std::thread[gShadowSettings.MaxSectorsAroundPlayer*2-1];
	for (int i = 0; i < gShadowSettings.MaxSectorsAroundPlayer * 2 - 1; i++)
	{
		tt[i] = std::thread(ScanShadowsMT, currentSectorX + i - gShadowSettings.MaxSectorsAroundPlayer, currentSectorY);
	}*/
	
	for (int x = -gShadowSettings.MaxSectorsAroundPlayer; x < gShadowSettings.MaxSectorsAroundPlayer+1; x++)
	{
		float sectorPosX = ((currentSectorX + x) - 60) * 50.0f + 25.0f;
		float sectorBoundXMin = min(sectorPosX - 25.0f, sectorPosX + 25.0f);
		float sectorBoundXMax = max(sectorPosX - 25.0f, sectorPosX + 25.0f);

		for (int y = -gShadowSettings.MaxSectorsAroundPlayer; y < gShadowSettings.MaxSectorsAroundPlayer+1; y++) {
			float sectorPosY = ((currentSectorY + y) - 60) * 50.0f + 25.0f;
			float sectorBoundYMin = min(sectorPosY - 25.0f, sectorPosY + 25.0f);
			float sectorBoundYMax = max(sectorPosY - 25.0f, sectorPosY + 25.0f);

			RW::BBox	 sectorBBox = { { sectorBoundXMin , sectorBoundYMin, -3000 },
										{ sectorBoundXMax , sectorBoundYMax, 3000 } };
			bool isInsideLightBBox = false;
			for (int i = 0; i < gShadowSettings.ShadowCascadeCount; i++)
				isInsideLightBBox = isInsideLightBBox || IsAABBInsideBoundingVolume(ms_aShadowCasterBoundingPlanes[i], 5, sectorBBox);
			
			if (isInsideLightBBox)
				CRendererRH::ScanSectorListForShadowCasters(currentSectorX + x, currentSectorY + y);
			
		}
	}
	/*for (size_t i = 0; i < gShadowSettings.MaxSectorsAroundPlayer * 2 - 1; i++)
	{
		tt[i].join();
	}
	delete[] tt;*/
	/*shadowSector[0].x = minX * 0.02 + 60.0;
	shadowSector[0].y = minY * 0.02 + 60.0;
	shadowSector[1].x = maxX * 0.02 + 60.0;
	shadowSector[1].y = maxY * 0.02 + 60.0;
	shadowSector[2].x = minX * 0.02 + 60.0;
	shadowSector[2].y = maxY * 0.02 + 60.0;
	shadowSector[3].x = maxX * 0.02 + 60.0;
	shadowSector[3].y = minY * 0.02 + 60.0;
	shadowSector[4].x = points[1].x * 0.02f + 60.0f;
	shadowSector[4].y = points[1].y * 0.02f + 60.0f;
	shadowSector[5].x = points[2].x * 0.02f + 60.0f;
	shadowSector[5].y = points[2].y * 0.02f + 60.0f;
	shadowSector[6].x = points[3].x * 0.02f + 60.0f;
	shadowSector[6].y = points[3].y * 0.02f + 60.0f;
	shadowSector[7].x = points[4].x * 0.02f + 60.0f;
	shadowSector[7].y = points[4].y * 0.02f + 60.0f;*/
	//CWorldScan__ScanWorld(sector, 5, CRenderer::ScanSectorListForShadowCasters);
	//m_loadingPriority = 0;
}

void CRendererRH::ResetLodRenderLists()
{
	ms_pLodRenderList = GetLodRenderListBase();
	ms_pLodDontRenderList = GetLodDontRenderListBase();
}

void CRendererRH::RenderShadowCasterEntity(CEntity *entity)
{
	if (entity->m_pRwObject == nullptr)
		return;
	entity->m_bImBeingRendered = true;
	// We need to render some sub-entities for peds
	if (entity->m_nType == eEntityType::ENTITY_TYPE_PED) {
		CPed* ped = (CPed*)entity;
		auto jetPackTask = ped->m_pIntelligence->GetTaskJetPack();
		if(jetPackTask && jetPackTask->m_pJetPackClump)
			_RpClumpRender(jetPackTask->m_pJetPackClump);
		CVisibilityPluginsRH::RenderWeaponsForPed(ped);
	}
	if (entity->m_pRwObject->type == rpATOMIC)
		entity->m_pRwAtomic->renderCallBack(entity->m_pRwAtomic);
	else
		_RpClumpRender(entity->m_pRwClump);
	entity->m_bImBeingRendered = false;
}

void CRendererRH::RenderReflectionEntity(CEntity *entity)
{
	if (entity->m_pRwObject == nullptr)
		return;
	entity->m_bImBeingRendered = true;
	if (entity->m_pRwObject->type == rpATOMIC)
		entity->m_pRwAtomic->renderCallBack(entity->m_pRwAtomic);
	else
		_RpClumpRender(entity->m_pRwClump);
	entity->m_bImBeingRendered = false;
}

sLodListEntry *CRendererRH::GetLodRenderListBase()
{
	return &(*(sLodListEntry *)0xC8E0E0);
}

sLodListEntry *CRendererRH::GetLodDontRenderListBase()
{
	return &(*(sLodListEntry *)0xC900C8);
}

void CRendererRH::CalculateShadowBoundingPlanes(int shadowCascade)
{
	auto lightSpaceFrustum = g_pDeferredRenderer->m_pShadowRenderer->m_LightSpaceMatrix[0];
	RW::V3d  LightAABBCenter = { g_pDeferredRenderer->m_pShadowRenderer->m_LightPos[0] };
	RW::BBox lightAABB = g_pDeferredRenderer->m_pShadowRenderer->m_LightBBox[shadowCascade];
	auto lightAABBX = lightAABB.getSizeX();
	auto lightAABBY = lightAABB.getSizeY();
	auto lightAABBZ = lightAABB.getSizeZ();
	auto rightAxis = lightSpaceFrustum.getRightv3();
	auto upAxis = lightSpaceFrustum.getUpv3();
	auto atAxis = lightSpaceFrustum.getAtv3();
	// Calculate light frustum planes
	ms_aShadowCasterBoundingPlanes[shadowCascade][0].normal = rightAxis.getRWVector();
	ms_aShadowCasterBoundingPlanes[shadowCascade][0].distance = (LightAABBCenter - rightAxis *lightAABBX).
		dot(rightAxis);

	ms_aShadowCasterBoundingPlanes[shadowCascade][1].normal = (-rightAxis).getRWVector();
	ms_aShadowCasterBoundingPlanes[shadowCascade][1].distance = (LightAABBCenter + rightAxis *lightAABBX).
		dot(-rightAxis);

	ms_aShadowCasterBoundingPlanes[shadowCascade][2].normal = upAxis.getRWVector();
	ms_aShadowCasterBoundingPlanes[shadowCascade][2].distance = (LightAABBCenter - upAxis * lightAABBY).
		dot(upAxis);

	ms_aShadowCasterBoundingPlanes[shadowCascade][3].normal = (-upAxis).getRWVector();
	ms_aShadowCasterBoundingPlanes[shadowCascade][3].distance = (LightAABBCenter + upAxis * lightAABBY).
		dot(-upAxis);

	ms_aShadowCasterBoundingPlanes[shadowCascade][4].normal = (-atAxis).getRWVector();
	ms_aShadowCasterBoundingPlanes[shadowCascade][4].distance = (LightAABBCenter + atAxis * lightAABBZ).
		dot(-atAxis);
}

void CRendererRH::ScanShadowsMT(const int& x, const int& cs_y)
{
	int currentSectorX = (int)ceil((ms_vecCameraPosition.x - 25.0f) / 50.0f + 60.0f);
	int currentSectorY = (int)ceil((ms_vecCameraPosition.y - 25.0f) / 50.0f + 60.0f);
	float sectorPosX = ((currentSectorX+x) - 60) * 50.0f + 25.0f;
	float sectorBoundXMin = min(sectorPosX - 25.0f, sectorPosX + 25.0f);
	float sectorBoundXMax = max(sectorPosX - 25.0f, sectorPosX + 25.0f);

	for (int y = -gShadowSettings.MaxSectorsAroundPlayer; y < gShadowSettings.MaxSectorsAroundPlayer + 1; y++) {
		float sectorPosY = ((currentSectorY + y) - 60) * 50.0f + 25.0f;
		float sectorBoundYMin = min(sectorPosY - 25.0f, sectorPosY + 25.0f);
		float sectorBoundYMax = max(sectorPosY - 25.0f, sectorPosY + 25.0f);

		RW::BBox	 sectorBBox = { { sectorBoundXMin , sectorBoundYMin, -3000 },
		{ sectorBoundXMax , sectorBoundYMax, 3000 } };
		bool isInsideLightBBox = false;
		for (int i = 0; i < gShadowSettings.ShadowCascadeCount; i++)
			isInsideLightBBox = isInsideLightBBox || IsAABBInsideBoundingVolume(ms_aShadowCasterBoundingPlanes[i], 5, sectorBBox);

		if (isInsideLightBBox)
			CRendererRH::ScanSectorListForShadowCasters(currentSectorX+x, currentSectorY + y);

	}
}
