#include "stdafx.h"
#include "CubemapReflectionRenderer.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "RwMethods.h"
#include "D3D1XRenderBuffersManager.h"
#include <game_sa\CScene.h>

#define ENVMAPSIZE 128
#define MIPLEVELS 1

CCubemapReflectionRenderer::CCubemapReflectionRenderer()
{
	ID3D11Device* device = GET_D3D_DEVICE;

	m_pReflCamera = RwCameraCreate();
	RwCameraSetProjection(m_pReflCamera, RwCameraProjection::rwPERSPECTIVE);

	RwCameraSetNearClipPlane(m_pReflCamera, 0.1f);
	RwCameraSetFarClipPlane(m_pReflCamera, 100);
	RwV2d vw;
	vw.x = vw.y = tan(3.14f / 4.0f);
	RwCameraSetViewWindow(m_pReflCamera, &vw);
	
	//m_pShadowCamera->projectionType = rwPARALLEL;

	RwCameraSetRaster(m_pReflCamera, nullptr);
	RwCameraSetZRaster(m_pReflCamera, RwRasterCreate(ENVMAPSIZE, ENVMAPSIZE, 32, rwRASTERTYPEZBUFFER));

	//CameraSize(m_pReflCamera, 0, tan(3.14f / 2), 1.0f);
	RwObjectHasFrameSetFrame(m_pReflCamera, RwFrameCreate());
	RpWorldAddCamera(Scene.m_pRpWorld, m_pReflCamera);
	/*
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBReflections);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	renderer->getDevice()->CreateBuffer(&bd, nullptr, &m_pReflCB);*/

	// Create cubic depth stencil texture.
	D3D11_TEXTURE2D_DESC dstex{};
	dstex.Width = ENVMAPSIZE;
	dstex.Height = ENVMAPSIZE;
	dstex.MipLevels = 1;
	dstex.ArraySize = 6;
	dstex.SampleDesc.Count = 1;
	dstex.SampleDesc.Quality = 0;
	dstex.Format = DXGI_FORMAT_D32_FLOAT;
	dstex.Usage = D3D11_USAGE_DEFAULT;
	dstex.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dstex.CPUAccessFlags = 0;
	dstex.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (FAILED(device->CreateTexture2D(&dstex, NULL, &g_pEnvMapDepth)))
		g_pDebug->printMsg("Failed to create depth env map.", 0);

	// Create the depth stencil view for the entire cube
	D3D11_DEPTH_STENCIL_VIEW_DESC DescDS{};
	DescDS.Format = DXGI_FORMAT_D32_FLOAT;
	DescDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	DescDS.Texture2DArray.FirstArraySlice = 0;
	DescDS.Texture2DArray.ArraySize = 6;
	DescDS.Texture2DArray.MipSlice = 0;
	if(FAILED(device->CreateDepthStencilView(g_pEnvMapDepth, &DescDS, &g_pEnvMapDSV)))
		g_pDebug->printMsg("Failed to create depth env map.",0);

	// Create the depth stencil view for single face rendering
	DescDS.Texture2DArray.ArraySize = 1;
	if(FAILED(device->CreateDepthStencilView(g_pEnvMapDepth, &DescDS, &g_pEnvMapOneDSV)))
		g_pDebug->printMsg("Failed to create depth env map.", 0);

	// Create the cube map for env map render target
	dstex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dstex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	dstex.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
	dstex.MipLevels = MIPLEVELS;
	if(FAILED(device->CreateTexture2D(&dstex, NULL, &g_pEnvMap)))
		g_pDebug->printMsg("Failed to create depth env map.", 0);

	// Create the 6-face render target view
	D3D11_RENDER_TARGET_VIEW_DESC DescRT{};
	DescRT.Format = dstex.Format;
	DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	DescRT.Texture2DArray.FirstArraySlice = 0;
	DescRT.Texture2DArray.ArraySize = 6;
	DescRT.Texture2DArray.MipSlice = 0;
	if(FAILED(device->CreateRenderTargetView(g_pEnvMap, &DescRT, &g_pEnvMapRTV)))
		g_pDebug->printMsg("Failed to create depth env map.", 0);
	// Create the one-face render target views
	DescRT.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < 6; ++i)
	{
		DescRT.Texture2DArray.FirstArraySlice = i;
		if(FAILED(device->CreateRenderTargetView(g_pEnvMap, &DescRT, &g_apEnvMapOneRTV[i])))
			g_pDebug->printMsg("Failed to create depth env map.", 0);
	}

	// Create the shader resource view for the cubic env map
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = dstex.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.TextureCube.MipLevels = MIPLEVELS;
	SRVDesc.TextureCube.MostDetailedMip = 0;
	if(FAILED(device->CreateShaderResourceView(g_pEnvMap, &SRVDesc, &g_pEnvMapSRV)))
		g_pDebug->printMsg("Failed to create depth env map.", 0);
}


CCubemapReflectionRenderer::~CCubemapReflectionRenderer()
{
	RwRasterDestroy(m_pReflCamera->zBuffer);
	RwCameraDestroy(m_pReflCamera);
	if (m_pReflCB) m_pReflCB->Release();
	if (g_pEnvMap) g_pEnvMap->Release();
	if (g_pEnvMapRTV) g_pEnvMapRTV->Release();
	for (int i = 0; i < 6; ++i)
	{
		if (g_apEnvMapOneRTV[i]) g_apEnvMapOneRTV[i]->Release();
	}
	if (g_pEnvMapSRV) g_pEnvMapSRV->Release();
	if (g_pEnvMapDepth) g_pEnvMapDepth->Release();
	if (g_pEnvMapDSV) g_pEnvMapDSV->Release();
	if (g_pEnvMapOneDSV) g_pEnvMapOneDSV->Release();
}

#define FindPlayerCoors(outPoint, playerIndex) ((RwV3d * (__cdecl *)(RwV3d *,int))0x56E010)(outPoint, playerIndex)
void CCubemapReflectionRenderer::RenderToCubemap(void(*renderCB)())
{
	// Move reflection camera to game camera position.
	auto s_cam_frame = RwCameraGetFrame(m_pReflCamera);
	RwV3d campos;
	FindPlayerCoors(&campos, 0);
	RW::V3d pos = { campos };
	//=*RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame(Scene.curCamera)));
	//RwFrameTranslate(s_cam_frame, &campos, rwCOMBINEREPLACE);


	//renderer->BeginDebugEvent(L"Reflection");
	// Setup values
	bool rotate2[] = { false, false,false,false,true,true };
	float angle[] = { 0, 180,-90,180,90,180 };
	// впереди, сзади, справа, слева, снизу, сверху
	// Render each face.
	RW::V3d At, Up, Right;
	At = { 1.0f,0.0f,0.0f };
	Up = { 0.0f,1.0f,0.0f };
	Right = { 0.0f,0.0f,1.0f };
	RenderOneFace(renderCB, 0,At,Up,Right, pos);
	At = { -1.0f,0.0f,0.0f };
	Up = { 0.0f,1.0f,0.0f };
	Right = { 0.0f,0.0f,1.0f };
	RenderOneFace(renderCB, 1,At,Up, Right, pos);
	At = { 0.0f,1.0f,0.0f };	   
	Up = { 0.0f,0.0f,-1.0f };
	Right = { 1.0f,0.0f,0.0f };
	RenderOneFace(renderCB, 2,At,Up, Right, pos);
	At = { 0.0f,-1.0f,0.0f };	   
	Up = { 0.0f,0.0f,1.0f };
	Right = { 1.0f,0.0f,0.0f };
	RenderOneFace(renderCB, 3,At,Up, Right, pos);
	At = { 0.0f,0.0f,1.0f };	   
	Up = { 0.0f,1.0f,0.0f };
	Right = { 1.0f,0.0f,0.0f };
	RenderOneFace(renderCB, 4,At,Up, Right, pos);
	At = { 0.0f,0.0f,-1.0f };	   
	Up = { 0.0f,1.0f,0.0f };
	Right = { 1.0f,0.0f,1.0f };
	RenderOneFace(renderCB, 5,At,Up, Right, pos);
	/*for (int i = 0; i < 6; i++)
	{
		if (angle != 0) {
			if (!rotate2[i])
				CameraRotate(m_pReflCamera, nullptr, angle[i]);
			else
				CameraRotate2(m_pReflCamera, nullptr, angle[i]);
		}
		RenderOneFace(renderCB, i);
	}*/

	//renderer->EndDebugEvent();
}

void CCubemapReflectionRenderer::RenderOneFace(void(*renderCB)(), int id, const RW::V3d&At, const RW::V3d&Up, const RW::V3d&Right, RW::V3d&  Pos)
{
	ID3D11DeviceContext* context = GET_D3D_CONTEXT;

	RwFrame*	reflCamFrame = RwCameraGetFrame(m_pReflCamera);
	RwMatrix*	reflCamMatrix = RwFrameGetMatrix(reflCamFrame);
	//
	RwV3d		camPos = *RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame(Scene.m_pRwCamera)));

																							 // Transform shadow camera to main camera position. TODO: move it to the center of frustum to cover full scene.
	RwFrameTranslate(reflCamFrame, &camPos, rwCOMBINEREPLACE);

	RwV3d invCamPos;
	camPos = *RwMatrixGetPos(reflCamMatrix);
	RwV3dScale(&invCamPos, &camPos, -1.0f);

	// Move camera to the origin position.
	RwFrameTranslate(reflCamFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

	// Rotate camera towards light direction
	RW::V3d upAxis = Up;
	RW::V3d zaxis = At;

	RW::V3d	xaxis = Right;//upAxis.cross(zaxis);
	//xaxis.normalize();
	RW::V3d	yaxis = zaxis.cross(xaxis);


	reflCamMatrix->at = zaxis.getRWVector();
	reflCamMatrix->up = upAxis.getRWVector();
	reflCamMatrix->right = xaxis.getRWVector();
	reflCamMatrix->pos = {};
	reflCamMatrix->pad3 = 0x3F800000;
	// Move camera back to needed position.
	RwFrameTranslate(reflCamFrame, &camPos, rwCOMBINEPOSTCONCAT);

	RwCameraClear(m_pReflCamera, gColourTop, rwCAMERACLEARZ);

	RwCameraBeginUpdate(m_pReflCamera);
	//RwMatrixLookAtLH(Pos, { At.x + Pos.x,At.y + Pos.y,At.z + Pos.z }, Up);

	float ClearColor[4] = { 0.0, 0.0, 0.0, 0.0 };
	context->ClearRenderTargetView(g_apEnvMapOneRTV[id], ClearColor);
	context->ClearDepthStencilView(g_pEnvMapOneDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
	ID3D11RenderTargetView* aRTViews[1] = { g_apEnvMapOneRTV[id] };
	context->OMSetRenderTargets(sizeof(aRTViews) / sizeof(aRTViews[0]), aRTViews, g_pEnvMapOneDSV);

	renderCB();
	RwCameraEndUpdate(m_pReflCamera);

}

void CCubemapReflectionRenderer::SetCubemap()
{
	ID3D11DeviceContext* context = GET_D3D_CONTEXT;
	context->PSSetShaderResources(6, 1, &g_pEnvMapSRV);
}
