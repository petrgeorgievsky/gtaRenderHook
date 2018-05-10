// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "LightManager.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3D1XStateManager.h"
#include "RwMethods.h"
#include <game_sa\CCamera.h>

CLight CLightManager::m_aLights[1024];
CD3D1XStructuredBuffer<CLight>* CLightManager::m_pLightBuffer = nullptr;
int CLightManager::m_nLightCount = 0;

void CLightManager::Init()
{
	m_pLightBuffer = new CD3D1XStructuredBuffer<CLight>(32);
}

void CLightManager::Shutdown()
{
	delete m_pLightBuffer;
}

void CLightManager::Reset()
{
	m_nLightCount = 0;
}

bool CLightManager::AddLight(const CLight& light) {
	if (m_nLightCount < 1024) {
		m_aLights[m_nLightCount] = light;
		m_nLightCount++;
		return true;
	}
	else
		return false;
}

void CLightManager::SortByDistance(const RwV3d& from)
{
	sort(&m_aLights[0], &m_aLights[1023], [&](const CLight &a, const CLight &b) {
		return (rwV3D_Dist(a.m_vPos,from)<rwV3D_Dist(b.m_vPos, from));
	});
}

CD3D1XStructuredBuffer<CLight> * CLightManager::GetBuffer()
{
	m_pLightBuffer->Update(&m_aLights[0]);
	g_pStateMgr->SetLightCount(min(m_nLightCount,32));
	return m_pLightBuffer;
}
