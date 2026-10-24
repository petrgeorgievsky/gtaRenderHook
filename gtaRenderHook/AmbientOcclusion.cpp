// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "AmbientOcclusion.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "D3D1XShaderDefines.h"
#include "D3DRenderer.h"
#include <game_sa\CScene.h>
#include "FullscreenQuad.h"
#include <fstream>
#include <iterator>
AmbientOcclusionSettingsBlock gAmbientOcclusionSettings;
RwRaster*	 CAmbientOcclusion::m_pAORaser = nullptr;
RwRaster*	 CAmbientOcclusion::m_pAOTempRaster = nullptr;
CD3D1XShader* CAmbientOcclusion::m_pAmbientOcclusionPS = nullptr;
CD3D1XShader* CAmbientOcclusion::m_pDenoisePS = nullptr;
CD3D1XConstantBuffer<CBAmbientOcclusion>* CAmbientOcclusion::m_pAmbientOcclusionCB = nullptr;
bool	CAmbientOcclusion::m_bRequiresReloading = false;
void CAmbientOcclusion::Init()
{
    m_pAORaser = RwRasterCreate( (RwInt32)( RsGlobal.maximumWidth*gAmbientOcclusionSettings.GetFloat( "Scale" ) ),
        (RwInt32)( RsGlobal.maximumHeight*gAmbientOcclusionSettings.GetFloat( "Scale" ) ), 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16 );
    m_pAOTempRaster = RwRasterCreate( m_pAORaser->width, m_pAORaser->height, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT16 );
    gDebugSettings.DebugRenderTargetList.push_back( m_pAORaser );
    m_pAmbientOcclusionPS = new CD3D1XPixelShader( "shaders/AmbientOcclusion.hlsl", "AmbientOcclusionPS", &gAmbientOcclusionSettings.m_ShaderDefineList );
    gAmbientOcclusionSettings.m_aShaderPointers.push_back( m_pAmbientOcclusionPS );
    // Presets may ship their own AmbientOcclusion.hlsl without the denoiser; a missing entry point can't be compiled.
    std::ifstream shaderFile( "shaders/AmbientOcclusion.hlsl" );
    const std::string shaderText( ( std::istreambuf_iterator<char>( shaderFile ) ), std::istreambuf_iterator<char>() );
    if ( shaderText.find( "AODenoisePS" ) != std::string::npos )
    {
        m_pDenoisePS = new CD3D1XPixelShader( "shaders/AmbientOcclusion.hlsl", "AODenoisePS", &gAmbientOcclusionSettings.m_ShaderDefineList );
        gAmbientOcclusionSettings.m_aShaderPointers.push_back( m_pDenoisePS );
    }
    m_pAmbientOcclusionCB = new CD3D1XConstantBuffer<CBAmbientOcclusion>();
}

void CAmbientOcclusion::Shutdown()
{
    if ( m_pAORaser )
        RwRasterDestroy( m_pAORaser );
    if ( m_pAOTempRaster )
        RwRasterDestroy( m_pAOTempRaster );
    delete m_pAmbientOcclusionPS;
    delete m_pDenoisePS;
    delete m_pAmbientOcclusionCB;
}

void CAmbientOcclusion::RenderAO( RwRaster * normalsDepth )
{
    static unsigned int frame = 0;
    auto &cb = m_pAmbientOcclusionCB->data;
    cb.AORadius = gAmbientOcclusionSettings.GetFloat( "Radius" );
    cb.AOCurve = gAmbientOcclusionSettings.GetFloat( "Curve" );
    cb.AOIntensity = gAmbientOcclusionSettings.GetFloat( "Intesity" ); // the setting's name, as presets spell it
    cb.AOFrame = gAmbientOcclusionSettings.GetToggleField( "TemporalNoise" ) ? static_cast<float>( frame++ % 64 ) : 0.0f;
    cb.AOTexelSize[0] = 1.0f / m_pAORaser->width;
    cb.AOTexelSize[1] = 1.0f / m_pAORaser->height;
    cb.AODenoiseDir[0] = cb.AODenoiseDir[1] = 0.0f;
    m_pAmbientOcclusionCB->Update();

    g_pRwCustomEngine->SetRenderTargets( &m_pAORaser, nullptr, 1 );
    g_pStateMgr->SetConstantBufferPS( m_pAmbientOcclusionCB, 8 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetRaster( normalsDepth );
    m_pAmbientOcclusionPS->Set();
    CFullscreenQuad::Draw();
    if ( !m_pDenoisePS )
        return;

    // Edge-aware blur, horizontal into the temporary target, then vertical back
    for ( int pass = 0; pass < 2; pass++ )
    {
        RwRaster *src = pass == 0 ? m_pAORaser : m_pAOTempRaster, *dst = pass == 0 ? m_pAOTempRaster : m_pAORaser;
        cb.AODenoiseDir[0] = pass == 0 ? 1.0f : 0.0f;
        cb.AODenoiseDir[1] = pass == 0 ? 0.0f : 1.0f;
        m_pAmbientOcclusionCB->Update();
        g_pStateMgr->SetRaster( nullptr, 1 );
        g_pRwCustomEngine->SetRenderTargets( &dst, nullptr, 1 );
        g_pStateMgr->FlushRenderTargets();
        g_pStateMgr->SetRaster( src, 1 );
        m_pDenoisePS->Set();
        CFullscreenQuad::Draw();
    }
    g_pStateMgr->SetRaster( nullptr, 1 );
}

RwRaster* CAmbientOcclusion::GetAORaster()
{
    return m_pAORaser;
}

void CAmbientOcclusion::QueueTextureReload()
{
    CRwD3D1XEngine* dxEngine = (CRwD3D1XEngine*)g_pRwCustomEngine;

    if ( m_bRequiresReloading || dxEngine->m_bScreenSizeChanged )
    {
        m_pAORaser->width = (int)( RsGlobal.maximumWidth*gAmbientOcclusionSettings.GetFloat( "Scale" ) );
        m_pAORaser->height = (int)( RsGlobal.maximumHeight*gAmbientOcclusionSettings.GetFloat( "Scale" ) );
        dxEngine->m_pRastersToReload.push_back( m_pAORaser );
        m_pAOTempRaster->width = m_pAORaser->width;
        m_pAOTempRaster->height = m_pAORaser->height;
        dxEngine->m_pRastersToReload.push_back( m_pAOTempRaster );
    }
}

void AmbientOcclusionSettingsBlock::Load( const tinyxml2::XMLDocument & doc )
{
    SettingsBlock::Load( doc );

    m_ShaderDefineList.AddDefine( "AO_SAMPLE_COUNT", std::to_string( gAmbientOcclusionSettings.GetUInt( "SampleCount" ) ) );
    m_ShaderDefineList.AddDefine( "AO_SLICE_COUNT", std::to_string( gAmbientOcclusionSettings.GetUInt( "Slices" ) ) );
}

void TW_CALL ReloadAOShadersCallBack( void *value )
{
    gAmbientOcclusionSettings.m_bShaderReloadRequired = true;
    gAmbientOcclusionSettings.m_ShaderDefineList.Reset();
    gAmbientOcclusionSettings.m_ShaderDefineList.AddDefine( "AO_SAMPLE_COUNT", std::to_string( gAmbientOcclusionSettings.GetUInt( "SampleCount" ) ) );
    gAmbientOcclusionSettings.m_ShaderDefineList.AddDefine( "AO_SLICE_COUNT", std::to_string( gAmbientOcclusionSettings.GetUInt( "Slices" ) ) );
}
void TW_CALL ReloadAOTexturesCallBack( void *value )
{
    CAmbientOcclusion::m_bRequiresReloading = true;
}
void AmbientOcclusionSettingsBlock::InitGUI( TwBar * mainBar )
{
    TwAddButton( mainBar, "Reload AO shaders", ReloadAOShadersCallBack, nullptr, "group=AmbientOcclusion" );
    TwAddButton( mainBar, "Reload AO textures", ReloadAOTexturesCallBack, nullptr, "group=AmbientOcclusion" );
}