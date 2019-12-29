// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "VolumetricLighting.h"
#include "D3D1XShader.h"
#include "D3D1XShaderDefines.h"
#include "RwRenderEngine.h"
#include "D3D1XStateManager.h"
#include "FullscreenQuad.h"
#include "D3DRenderer.h"
#include <game_sa\CScene.h>

CD3D1XShader* CVolumetricLighting::m_pVolumetricSunlightPS = nullptr;
CD3D1XShader* CVolumetricLighting::m_pVolumetricCombinePS = nullptr;
RwRaster*	 CVolumetricLighting::m_pVolumeLightingRaster = nullptr;
CD3D1XConstantBuffer<CBVolumetricLighting>* CVolumetricLighting::m_pVolumeLightingCB = nullptr;
bool	CVolumetricLighting::m_bRequiresReloading = false;
VolumetricLightingSettingsBlock gVolumetricLightingSettings;

void CVolumetricLighting::Init()
{
    m_pVolumetricSunlightPS = new CD3D1XPixelShader( "shaders/VolumetricLighting.hlsl", "VolumetricSunlightPS", &gVolumetricLightingSettings.m_ShaderDefineList );
    m_pVolumetricCombinePS = new CD3D1XPixelShader( "shaders/VolumetricLighting.hlsl", "VolumetricCombinePS", &gVolumetricLightingSettings.m_ShaderDefineList );
    float renderingScale = gVolumetricLightingSettings.GetFloat( "VolumetricRenderingScale" );
    m_pVolumeLightingRaster = RwRasterCreate( (RwInt32)( RsGlobal.maximumWidth*renderingScale ),
        (RwInt32)( RsGlobal.maximumHeight*renderingScale ),
                                              32, rwRASTERTYPECAMERATEXTURE );
    m_pVolumeLightingCB = new CD3D1XConstantBuffer<CBVolumetricLighting>();
    gVolumetricLightingSettings.m_aShaderPointers.push_back( m_pVolumetricSunlightPS );
    gVolumetricLightingSettings.m_aShaderPointers.push_back( m_pVolumetricCombinePS );
    gDebugSettings.DebugRenderTargetList.push_back( m_pVolumeLightingRaster );

    m_pVolumeLightingCB->data.RaymarchingDistance = gVolumetricLightingSettings.GetFloat( "RaymarchingDistance" );
    m_pVolumeLightingCB->data.SunlightBlendOffset = gVolumetricLightingSettings.GetFloat( "SunlightBlendOffset" );
    m_pVolumeLightingCB->data.SunlightIntensity = gVolumetricLightingSettings.GetFloat( "SunlightIntensity" );
    m_pVolumeLightingCB->Update();
}

void CVolumetricLighting::Shutdown()
{
    delete m_pVolumeLightingCB;
    delete m_pVolumetricSunlightPS;
    delete m_pVolumetricCombinePS;
    if ( m_pVolumeLightingRaster )
        RwRasterDestroy( m_pVolumeLightingRaster );
}

void CVolumetricLighting::RenderVolumetricEffects( RwRaster* normalsDepth, RwRaster* cascadeShadowMap, RwRaster* from, RwRaster* to )
{
    m_pVolumeLightingCB->data.RaymarchingDistance = gVolumetricLightingSettings.GetFloat( "RaymarchingDistance" );
    m_pVolumeLightingCB->data.SunlightBlendOffset = gVolumetricLightingSettings.GetFloat( "SunlightBlendOffset" );
    m_pVolumeLightingCB->data.SunlightIntensity = gVolumetricLightingSettings.GetFloat( "SunlightIntensity" );
    m_pVolumeLightingCB->Update();

    g_pStateMgr->SetConstantBufferPS( m_pVolumeLightingCB, 7 );
    // First render all volumetric lighting(sun light, volume streetlights, clouds etc.)
    g_pRwCustomEngine->SetRenderTargets( &m_pVolumeLightingRaster, nullptr, 1 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetRaster( cascadeShadowMap, 3 );
    m_pVolumetricSunlightPS->Set();
    CFullscreenQuad::Draw();
    // Than combine them with 
    g_pRwCustomEngine->SetRenderTargets( &to, Scene.m_pRwCamera->zBuffer, 1 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetRaster( from );
    g_pStateMgr->SetRaster( m_pVolumeLightingRaster, 2 );
    m_pVolumetricCombinePS->Set();
    CFullscreenQuad::Draw();
}

void CVolumetricLighting::QueueTextureReload()
{
    CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;

    if ( m_bRequiresReloading || dxEngine->m_bScreenSizeChanged )
    {
        float renderingScale = gVolumetricLightingSettings.GetFloat( "VolumetricRenderingScale" );
        m_pVolumeLightingRaster->width = (int)( RsGlobal.maximumWidth*renderingScale );
        m_pVolumeLightingRaster->height = (int)( RsGlobal.maximumHeight*renderingScale );
        dxEngine->m_pRastersToReload.push_back( m_pVolumeLightingRaster );
    }
}

void VolumetricLightingSettingsBlock::Load( const tinyxml2::XMLDocument & doc )
{
    SettingsBlock::Load( doc );
    m_ShaderDefineList.AddDefine( "SUNLIGHT_RM_STEPS", std::to_string( gVolumetricLightingSettings.GetUInt( "SunlightRaymarchingSteps" ) ) );
}

void TW_CALL ReloadVolumetricShadersCallBack( void *value )
{
    gVolumetricLightingSettings.m_bShaderReloadRequired = true;
    gVolumetricLightingSettings.m_ShaderDefineList.Reset();
    gVolumetricLightingSettings.m_ShaderDefineList.AddDefine( "SUNLIGHT_RM_STEPS", std::to_string( gVolumetricLightingSettings.GetUInt( "SunlightRaymarchingSteps" ) ) );
}
void TW_CALL ReloadVolumetricTexturesCallBack( void *value )
{
    CVolumetricLighting::m_bRequiresReloading = true;
}
void VolumetricLightingSettingsBlock::InitGUI( TwBar * mainBar )
{
    SettingsBlock::InitGUI( mainBar );
    TwAddButton( mainBar, "Reload volume shaders", ReloadVolumetricShadersCallBack, nullptr, "group=VolumetricLighting" );
    TwAddButton( mainBar, "Reload volume textures", ReloadVolumetricTexturesCallBack, nullptr, "group=VolumetricLighting" );
}
