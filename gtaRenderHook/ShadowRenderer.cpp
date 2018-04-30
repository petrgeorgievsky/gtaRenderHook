#include "stdafx.h"
#include "ShadowRenderer.h"
#include "D3DRenderer.h"
#include "RwD3D1XEngine.h"
#include "D3D1XTexture.h"
#include "D3D1XStateManager.h"
#include "SettingsHolder.h"
#include "RwMethods.h"
#include "D3D1XBuffer.h"
#include <game_sa\CScene.h>
#include "DeferredRenderer.h"
#include "D3D1XRenderBuffersManager.h"
RW::BBox	CShadowRenderer::m_LightBBox[4];
RwV3d	CShadowRenderer::m_LightPos[4];
RW::Matrix	CShadowRenderer::m_LightSpaceMatrix[4];
RW::Matrix	CShadowRenderer::m_InvLightSpaceMatrix[4];
ShadowSettingsBlock gShadowSettings;

CShadowRenderer::CShadowRenderer()
{
	RwV2d vw;
	m_pShadowCamera	= RwCameraCreate();
	RwCameraSetProjection(m_pShadowCamera, rwPARALLEL);
	RwCameraSetNearClipPlane(m_pShadowCamera, -150);
	RwCameraSetFarClipPlane(m_pShadowCamera, 1500);
	RwCameraSetRaster(m_pShadowCamera, nullptr);
	int maxTexSize;
	if (g_pRwCustomEngine->GetMaxTextureSize(maxTexSize))
		gShadowSettings.Size =(UINT) min((int)gShadowSettings.Size, maxTexSize / gShadowSettings.ShadowCascadeCount);
	RwCameraSetZRaster(m_pShadowCamera, RwRasterCreate(gShadowSettings.Size * gShadowSettings.ShadowCascadeCount, gShadowSettings.Size, 32, rwRASTERTYPEZBUFFER));
	vw.x = vw.y = 40;
	RwCameraSetViewWindow(m_pShadowCamera, &vw);
	gDebugSettings.DebugRenderTargetList.push_back(m_pShadowCamera->zBuffer);
	RwObjectHasFrameSetFrame(m_pShadowCamera, RwFrameCreate());
	RpWorldAddCamera(Scene.m_pRpWorld, m_pShadowCamera);
	m_pLightViewProj = nullptr;

	m_pLightCB = new CD3D1XConstantBuffer<CBShadows>();
	m_pLightCB->SetDebugName("ShadowsCB");
	m_aShadowObjectCacheList = {};
}


CShadowRenderer::~CShadowRenderer()
{
	delete m_pLightCB;
	//RwRasterDestroy(m_pShadowCamera->frameBuffer);
	RwRasterDestroy(m_pShadowCamera->zBuffer);
	RwCameraDestroy(m_pShadowCamera);
}
// Transforms light camera to fit needed camera frustum and look in needed direction.
void CShadowRenderer::DirectionalLightTransform(RwCamera* mainCam, const RW::V3d & lightDir, int shadowCascade)
{
	RwFrame*	shadowCamFrame	= RwCameraGetFrame(m_pShadowCamera);
	RwMatrix*	shadowCamMatrix	= RwFrameGetMatrix(shadowCamFrame);
	//
	m_LightPos[shadowCascade] = CalculateCameraPos(mainCam, lightDir, shadowCascade).getRWVector(); //*RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame(mainCam)));

	
}

RW::V3d CShadowRenderer::CalculateCameraPos(RwCamera* mainCam, const RW::V3d & lightDir, int shadowCascade)
{
	RW::V3d vLightPos, vLightDir, vFrustrumCenter;
	vFrustrumCenter = RW::V3d{ 0, 0, 0 };
	RW::V3d vLightBasis[3];
	float faMinAABB[3];
	float faMaxAABB[3];

	for (int i = 0; i < 3; i++)
	{
		faMinAABB[i] = FLT_MAX;
		faMaxAABB[i] = -FLT_MAX;
	}
	vLightBasis[2] = -lightDir;
	vLightBasis[2].normalize();
	vLightBasis[1] = { 0, 1, 0 };
	vLightBasis[0] = vLightBasis[1].cross(vLightBasis[2]);
	vLightBasis[0].normalize();
	vLightBasis[1] = vLightBasis[0].cross(vLightBasis[2]);
	vLightBasis[1].normalize();

	m_LightSpaceMatrix[shadowCascade] = { { vLightBasis[0] }, { vLightBasis[1] }, { vLightBasis[2] }, {} };
	m_InvLightSpaceMatrix[shadowCascade] = m_LightSpaceMatrix[shadowCascade].inverse();

	auto oldFP = mainCam->farPlane;
	auto oldNP = mainCam->nearPlane;
	RwCameraSetNearClipPlane(mainCam, m_fShadowDistances[shadowCascade]);
	RwCameraSetFarClipPlane(mainCam, m_fShadowDistances[shadowCascade + 1]);

	RwCameraSync(mainCam);
	
	RW::V3d vFrustumCorners[8];
	CalculateFrustumPoints(m_fShadowDistances[shadowCascade], m_fShadowDistances[shadowCascade + 1], mainCam, vFrustumCorners);
	// Transform frustum corners in light space
	for (UINT i = 0; i < 8; i++)
		vFrustumCorners[i] = vFrustumCorners[i] * m_LightSpaceMatrix[shadowCascade];
	// Generate light-aligned Bounding Box from frustum corners in light space
	m_LightBBox[shadowCascade]={ vFrustumCorners, 8 };

	RwV2d vw{ m_LightBBox[shadowCascade].getSizeX()*0.5f , m_LightBBox[shadowCascade].getSizeY()*0.5f };
	vFrustrumCenter = m_LightBBox[shadowCascade].getCenter()*m_InvLightSpaceMatrix[shadowCascade];
	RwCameraSetViewWindow(m_pShadowCamera, &vw);
	float fLightZFar = m_LightBBox[shadowCascade].getSizeZ()*0.5f;//faLightDim[1];
	RwCameraSetNearClipPlane(m_pShadowCamera, -1500.0f/*min(-fLightZFar,-1500.0f)*/);
	RwCameraSetFarClipPlane(m_pShadowCamera, 1500.0f /*max(fLightZFar,1500.0f)*/);
	RwCameraSync(m_pShadowCamera);
	//RwCameraEndUpdate(mainCam);
	RwCameraSetNearClipPlane(mainCam, oldNP);
	RwCameraSetFarClipPlane(mainCam, oldFP);
	RwCameraSync(mainCam);
	
	return vFrustrumCenter;
}
void CShadowRenderer::CalculateFrustumPoints(RwReal fNear, RwReal fFar, RwCamera * camera, RW::V3d * points)
{
	RwFrame*	camFrame	= RwCameraGetFrame(camera);
	RwMatrix*	camMatrix	= RwFrameGetMatrix(camFrame);
	RW::V3d		camPos		= *RwMatrixGetPos(camMatrix);
	RW::V3d		camAt		= *RwMatrixGetAt(camMatrix);
	RW::V3d		camRight	= *RwMatrixGetRight(camMatrix);
	RW::V3d		camUp		= *RwMatrixGetUp(camMatrix);

	RW::V3d nearCenter		= camPos + camAt * fNear;
	RW::V3d farCenter		= camPos + camAt * fFar;

	RwReal nearHeight = 2 * camera->viewWindow.y * fNear;
	RwReal farHeight = 2 * camera->viewWindow.y * fFar;
	RwReal nearWidth = 2 * camera->viewWindow.x * fNear;
	RwReal farWidth = 2 * camera->viewWindow.x * fFar;

	RW::V3d nearBottom	= nearCenter - camUp * nearHeight*0.5f;
	RW::V3d nearTop		= nearCenter + camUp * nearHeight*0.5f;

	RW::V3d farBottom	= farCenter - camUp * farHeight*0.5f;
	RW::V3d farTop		= farCenter + camUp * farHeight*0.5f;

	RW::V3d farRightOffset  = camRight * (nearWidth*0.5f);
	RW::V3d nearRightOffset = camRight * (farWidth*0.5f);

	// near plane
	points[0] = nearBottom	- nearRightOffset;
	points[1] = nearBottom	+ nearRightOffset;
	points[2] = nearTop		- nearRightOffset;
	points[3] = nearTop		+ nearRightOffset;

	// far plane
	points[4] = farBottom	- farRightOffset;
	points[5] = farBottom	+ farRightOffset;
	points[6] = farTop		- farRightOffset;
	points[7] = farTop		+ farRightOffset;

}

void CShadowRenderer::RenderShadowToBuffer(int cascade,void(*render)(int cascade))
{
	CD3DRenderer* renderer = static_cast<CRwD3D1XEngine*>(g_pRwCustomEngine)->getRenderer();
	renderer->BeginDebugEvent(L"Shadow Buffer");
	if(cascade==0)
		m_aShadowObjectCacheList.clear();

	RwFrame*	shadowCamFrame = RwCameraGetFrame(m_pShadowCamera);
	RwMatrix*	shadowCamMatrix = RwFrameGetMatrix(shadowCamFrame);
	RwV3d lightPos = m_LightPos[cascade];//(m_LightBBox[cascade].getCenter()*m_InvLightSpaceMatrix[cascade]).getRWVector();
	// Transform shadow camera to main camera position. TODO: move it to the center of frustum to cover full scene.
	RwFrameTranslate(shadowCamFrame, &lightPos, rwCOMBINEREPLACE);

	RwV3d invCamPos;
	lightPos = *RwMatrixGetPos(shadowCamMatrix);
	RwV3dScale(&invCamPos, &lightPos, -1.0f);

	// Move camera to the origin position.
	RwFrameTranslate(shadowCamFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

	// Rotate camera towards light direction
	shadowCamMatrix->at = m_LightSpaceMatrix[cascade].getAt().getRW3Vector();
	shadowCamMatrix->up = m_LightSpaceMatrix[cascade].getUp().getRW3Vector();
	shadowCamMatrix->right = m_LightSpaceMatrix[cascade].getRight().getRW3Vector();
	shadowCamMatrix->pos = {};
	shadowCamMatrix->pad3 = 0x3F800000;

	// Move camera back to needed position.
	RwFrameTranslate(shadowCamFrame, &lightPos, rwCOMBINEPOSTCONCAT);
	// Set light orthogonal projection parameters.
	RwV2d vw{ m_LightBBox[cascade].getSizeX() * 0.65f, m_LightBBox[cascade].getSizeY() * 0.65f };
	RwCameraSetViewWindow(m_pShadowCamera, &vw);
	float fLightZFar = m_LightBBox[cascade].getSizeZ()*0.5f;//faLightDim[1];
	RwCameraSetNearClipPlane(m_pShadowCamera, -500 /*-fLightZFar-25*/);
	RwCameraSetFarClipPlane(m_pShadowCamera, fLightZFar+25);
	RwCameraSync(m_pShadowCamera);
	//
	RwCameraBeginUpdate(m_pShadowCamera);
	// Render in one of texture corners, this is done to use less textures.   
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(gShadowSettings.Size);
	vp.Height = static_cast<FLOAT>(gShadowSettings.Size);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float)(gShadowSettings.Size * cascade);
	vp.TopLeftY = 0;
	g_pStateMgr->SetViewport(vp);

	RwGraphicsMatrix* view = (RwGraphicsMatrix *)&RwD3D9D3D9ViewTransform;
	RwGraphicsMatrix* proj = (RwGraphicsMatrix *)&RwD3D9D3D9ProjTransform;
	g_pRenderBuffersMgr->Multipy4x4Matrices((RwGraphicsMatrix *)&m_pLightCB->data.ViewProj[cascade], view, proj);
	render(cascade);
	RwCameraEndUpdate(m_pShadowCamera);
	renderer->EndDebugEvent();
}
void CShadowRenderer::SetShadowBuffer() const
{
	m_pLightCB->data.ShadowSize = gShadowSettings.Size;
	m_pLightCB->data.CascadeCount = gShadowSettings.ShadowCascadeCount;
	for (auto i = 0; i < 4; i++)
		m_pLightCB->data.ShadowBias[i] = gShadowSettings.BiasCoefficients[i];
	m_pLightCB->Update();
	g_pStateMgr->SetConstantBufferPS(m_pLightCB, 4);
	g_pStateMgr->SetConstantBufferCS(m_pLightCB, 4);
	// If shadow rendering has not ended we don't need to set shadow buffer
	if (!m_bShadowsRendered)
		return;
	g_pStateMgr->SetRaster(m_pShadowCamera->zBuffer, 3);
}

void CShadowRenderer::CalculateShadowDistances(const RwReal fNear, const RwReal fFar)
{
	float farDist = min(gShadowSettings.MaxDrawDistance, fFar);
	m_fShadowDistances[0] = fNear;
	m_fShadowDistances[1] = fNear + farDist* gShadowSettings.DistanceCoefficients[0];
	m_fShadowDistances[2] = fNear + farDist* gShadowSettings.DistanceCoefficients[1];
	m_fShadowDistances[3] = fNear + farDist* gShadowSettings.DistanceCoefficients[2];
	m_fShadowDistances[4] = farDist;
	for(int i=0;i<5;i++)
		m_pLightCB->data.FadeDistances[i] = m_fShadowDistances[i];
}

void CShadowRenderer::QueueTextureReload()
{
	if (m_bRequiresReloading) {
		m_pShadowCamera->zBuffer->width = gShadowSettings.Size * gShadowSettings.ShadowCascadeCount;
		m_pShadowCamera->zBuffer->height = gShadowSettings.Size;
		CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;
		dxEngine->m_pRastersToReload.push_back(m_pShadowCamera->zBuffer);
	}
}

tinyxml2::XMLElement * ShadowSettingsBlock::Save(tinyxml2::XMLDocument * doc)
{
	// Shadow settings node.
	auto shadowSettingsNode = doc->NewElement(m_sName.c_str());


	shadowSettingsNode->SetAttribute("Enable", gShadowSettings.Enable);
	shadowSettingsNode->SetAttribute("CullPerCascade", gShadowSettings.CullPerCascade);
	shadowSettingsNode->SetAttribute("ScanShadowsBehindPlayer", gShadowSettings.ScanShadowsBehindPlayer);
	shadowSettingsNode->SetAttribute("Size", gShadowSettings.Size);
	shadowSettingsNode->SetAttribute("MaxDrawDistance", gShadowSettings.MaxDrawDistance);
	shadowSettingsNode->SetAttribute("MinOffscreenShadowCasterSize", gShadowSettings.MinOffscreenShadowCasterSize);
	shadowSettingsNode->SetAttribute("MaxSectorsAroundPlayer", gShadowSettings.MaxSectorsAroundPlayer);
	shadowSettingsNode->SetAttribute("LodShadowsMinDistance", gShadowSettings.LodShadowsMinDistance);
	shadowSettingsNode->SetAttribute("ShadowCascadeCount", gShadowSettings.ShadowCascadeCount);
	
	for (size_t i = 0; i < 4; i++)
	{
		auto shadowCascadesSettingsNode = doc->NewElement("Cascade");
		shadowCascadesSettingsNode->SetAttribute("ID", i + 1);
		if (i != 0)
			shadowCascadesSettingsNode->SetAttribute("StartDistanceMultiplier", gShadowSettings.DistanceCoefficients[i - 1]);
		shadowCascadesSettingsNode->SetAttribute("BiasCoefficients", gShadowSettings.BiasCoefficients[i]);

		shadowSettingsNode->InsertEndChild(shadowCascadesSettingsNode);
	}

	return shadowSettingsNode;
}

void ShadowSettingsBlock::Load(const tinyxml2::XMLDocument & doc)
{
	auto shadowSettingsNode = doc.FirstChildElement(m_sName.c_str());
	// Shadows
	Enable = shadowSettingsNode->BoolAttribute("Enable", true);
	CullPerCascade = shadowSettingsNode->BoolAttribute("CullPerCascade", false);
	ScanShadowsBehindPlayer = shadowSettingsNode->BoolAttribute("ScanShadowsBehindPlayer", true);
	
	Size = shadowSettingsNode->IntAttribute("Size", 1024);
	MaxDrawDistance = shadowSettingsNode->FloatAttribute("MaxDrawDistance", 500);
	MinOffscreenShadowCasterSize = shadowSettingsNode->FloatAttribute("MinOffscreenShadowCasterSize", 50);
	MaxSectorsAroundPlayer = shadowSettingsNode->IntAttribute("MaxSectorsAroundPlayer", 3);
	LodShadowsMinDistance = shadowSettingsNode->FloatAttribute("LodShadowsMinDistance", 150);
	ShadowCascadeCount =max(min(shadowSettingsNode->IntAttribute("ShadowCascadeCount", 3),4),1);
	
	// Read 4 shadow cascade parameters
	auto shadowCascadeNode = shadowSettingsNode->FirstChildElement();
	int id = shadowCascadeNode->IntAttribute("ID");
	if (id != 1)
		DistanceCoefficients[id - 2] = shadowCascadeNode->FloatAttribute("StartDistanceMultiplier");
	BiasCoefficients[id - 1] = shadowCascadeNode->FloatAttribute("BiasCoefficients");
	for (size_t i = 1; i < 4; i++)
	{
		shadowCascadeNode = shadowCascadeNode->NextSiblingElement("Cascade");
		id = shadowCascadeNode->IntAttribute("ID");
		if (id != 1)
			DistanceCoefficients[id - 2] = shadowCascadeNode->FloatAttribute("StartDistanceMultiplier");
		BiasCoefficients[id - 1] = shadowCascadeNode->FloatAttribute("BiasCoefficients");
	}
}

void ShadowSettingsBlock::Reset()
{
	// Shadows
	Enable = true;
	CullPerCascade = false;
	ScanShadowsBehindPlayer = true;
	Size = 1024;
	MaxDrawDistance = 500;
	BiasCoefficients[0] = 0.003f;
	BiasCoefficients[1] = 0.003f;
	BiasCoefficients[2] = 0.003f;
	BiasCoefficients[3] = 0.003f;
	DistanceCoefficients[0] = 0.01f;
	DistanceCoefficients[1] = 0.15f;
	DistanceCoefficients[2] = 0.45f;
	MinOffscreenShadowCasterSize=50;
	MaxSectorsAroundPlayer=3;
	LodShadowsMinDistance = 150;
	ShadowCascadeCount = 3;
}
void TW_CALL SetShadowSizeCallback(const void *value, void *clientData)
{
	int maxTexSize;
	g_pRwCustomEngine->GetMaxTextureSize(maxTexSize);
	gShadowSettings.Size = max(min(*(int*)value, maxTexSize / gShadowSettings.ShadowCascadeCount),16);
	g_pDeferredRenderer->m_pShadowRenderer->m_bRequiresReloading = true;
}
void TW_CALL GetShadowSizeCallback(void *value, void *clientData)
{
	*(int*)value = gShadowSettings.Size;
}
void TW_CALL SetCascadeCountCallback(const void *value, void *clientData)
{
	int maxTexSize;
	g_pRwCustomEngine->GetMaxTextureSize(maxTexSize);
	gShadowSettings.ShadowCascadeCount = max(min(*(int*)value, maxTexSize / gShadowSettings.Size),1);
	g_pDeferredRenderer->m_pShadowRenderer->m_bRequiresReloading = true;
}
void TW_CALL GetCascadeCountCallback(void *value, void *clientData)
{
	*(int*)value = gShadowSettings.ShadowCascadeCount;
}
void ShadowSettingsBlock::InitGUI(TwBar * bar)
{
	int maxTexSize;
	std::string shadowSettings = "min=16 max=";
	if (!g_pRwCustomEngine->GetMaxTextureSize(maxTexSize))
		shadowSettings += "1024";
	else
		shadowSettings += to_string(maxTexSize/2);
	shadowSettings += " step=2 help='Shadow map size, higher - less pixelation/blurry shadows, lower - higher performance' group=Shadows";
	TwAddVarCB(bar, "Cascade Size", TwType::TW_TYPE_UINT32,
		SetShadowSizeCallback, GetShadowSizeCallback, nullptr, shadowSettings.c_str());
	TwAddVarRW(bar, "Max DrawDistance", TwType::TW_TYPE_FLOAT,
		&MaxDrawDistance,
		" min=5 max=30000 step=5 help='Maximum shadow draw distance, limits how far shadows being drawn.' group=Shadows");
	TwAddVarRW(bar, "Maximum Offscreen Shadow LOD distance", TwType::TW_TYPE_FLOAT,
		&LodShadowsMinDistance,
		" min=2 max=3000 step=1 help='Maximum shadow lod distance, limits start of lods in shadows behind player.' group=Shadows");
	TwAddVarRW(bar, "Minimum Offscreen Shadow Caster radius", TwType::TW_TYPE_FLOAT,
		&MinOffscreenShadowCasterSize,
		" min=0 max=3000 step=1 help='Minimum bounding sphere for offscreen shadow casters.' group=Shadows");
	TwAddVarRW(bar, "Max Offscreen Shadow Sectors", TwType::TW_TYPE_UINT32,
		&MaxSectorsAroundPlayer,
		" min=1 max=10 step=1 help='Maximum sectors for offscreen shadow scan.' group=Shadows");
	
	TwAddVarRW(bar, "Cull per cascade", TwType::TW_TYPE_BOOL8, &CullPerCascade, "help='meh' group=Shadows");
	TwAddVarRW(bar, "Scan for shadows behind player", TwType::TW_TYPE_BOOL8, &ScanShadowsBehindPlayer, "help='meh' group=Shadows");
	TwAddVarCB(bar, "Cascade count", TwType::TW_TYPE_INT32, 
		SetCascadeCountCallback, GetCascadeCountCallback, nullptr, "min=1 max=4 help='meh' group=Shadows");

	TwAddVarRW(bar, "Distance multipier 1", TwType::TW_TYPE_FLOAT,
		&DistanceCoefficients[0],
		" min=0.00001 max=1.0 step=0.00001 group=Cascade_1 ");

	TwAddVarRW(bar, "Distance multipier 2", TwType::TW_TYPE_FLOAT,
		&DistanceCoefficients[1],
		" min=0.00001 max=1.0 step=0.00001  group=Cascade_2 ");

	TwAddVarRW(bar, "Distance multipier 3", TwType::TW_TYPE_FLOAT,
		&DistanceCoefficients[2],
		" min=0.00001 max=1.0 step=0.00001  group=Cascade_3 ");

	TwAddVarRW(bar, "Bias 0", TwType::TW_TYPE_FLOAT,
		&BiasCoefficients[0],
		" min=0.000001 max=1.0 step=0.000001 group=Cascade_0 ");


	TwAddVarRW(bar, "Bias 1", TwType::TW_TYPE_FLOAT,
		&BiasCoefficients[1],
		" min=0.0001 max=1.0 step=0.00001 group=Cascade_1 ");


	TwAddVarRW(bar, "Bias 2", TwType::TW_TYPE_FLOAT,
		&BiasCoefficients[2],
		" min=0.0001 max=1.0 step=0.00001 group=Cascade_2 ");


	TwAddVarRW(bar, "Bias 3", TwType::TW_TYPE_FLOAT,
		&BiasCoefficients[3],
		" min=0.0001 max=1.0 step=0.00001 group=Cascade_3 ");

	TwDefine(" Settings/Cascade_0   group=Shadows label='1st Cascade'");
	TwDefine(" Settings/Cascade_1   group=Shadows label='2nd Cascade'");
	TwDefine(" Settings/Cascade_2   group=Shadows label='3rd Cascade'");
	TwDefine(" Settings/Cascade_3   group=Shadows label='4th Cascade'");
}
