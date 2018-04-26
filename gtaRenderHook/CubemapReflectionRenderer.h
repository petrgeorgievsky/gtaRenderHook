#pragma once
#include "RwVectorMath.h"
struct CBReflections
{
	RwMatrix View[6];
};
class CCubemapReflectionRenderer
{
public:
	CCubemapReflectionRenderer(int size);
	~CCubemapReflectionRenderer();
	void RenderToCubemap(void(*renderCB)());
	void RenderOneFace(void(*renderCB)(),int id, float angleA, RwV3d axisA, float angleB, RwV3d axisB, RwV3d camPos/*,const RW::V3d&At, const RW::V3d&Up, const RW::V3d&Right, RW::V3d&Pos*/);
	void SetCubemap();
public:
	CBReflections cb_refl;
	RwCamera* m_pReflCamera;
	RwFrame* m_pReflCameraFrame;
	int m_nCubemapSize;
private:
	ID3D11Buffer* m_pReflCB=nullptr;
	ID3D11Texture2D*                    g_pEnvMap;          // Environment map
	ID3D11RenderTargetView*             g_pEnvMapRTV;       // Render target view for the alpha map
	ID3D11RenderTargetView*             g_apEnvMapOneRTV[6];// 6 render target view, each view is used for 1 face of the env map
	ID3D11ShaderResourceView*           g_pEnvMapSRV;       // Shader resource view for the cubic env map
	ID3D11ShaderResourceView*           g_apEnvMapOneSRV[6];// Single-face shader resource view
	ID3D11Texture2D*                    g_pEnvMapDepth;     // Depth stencil for the environment map
	ID3D11DepthStencilView*             g_pEnvMapDSV;       // Depth stencil view for environment map for all 6 faces
	ID3D11DepthStencilView*             g_pEnvMapOneDSV;    // Depth stencil view for environment map for all 1 face
};

