#pragma once
#include "D3D1XStructuredBuffer.h"
#include "D3D1XConstantBuffer.h"
struct CLight {
	RwV3d m_vPos;
	int   m_nLightType;
	RwV3d m_vDir;
	float m_fRange;
};

class CLightManager
{
public:
	static CLight m_aLights[1024];
	static int m_nLightCount;
private:
	static CD3D1XStructuredBuffer<CLight>* m_pLightBuffer;
public:
	static void Init();
	static void Shutdown();
	static void Reset();
	static bool AddLight(const CLight &light);
	static void SortByDistance(const RwV3d& from);
	static CD3D1XStructuredBuffer<CLight>* GetBuffer();
};

