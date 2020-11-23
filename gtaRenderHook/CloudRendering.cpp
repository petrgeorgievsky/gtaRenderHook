#include "stdafx.h"
#include "CloudRendering.h"
#include "D3D1X3DTexture.h"
#include "D3D1XShader.h"
#include "D3D1XShaderDefines.h"
#include "FastNoiseSIMD.h"
#include "D3D1XStateManager.h"
#include "D3D1XTextureMemoryManager.h"
#include <game_sa\CWeather.h>
#include "FullscreenQuad.h"
#include "RwRenderEngine.h"
#include <game_sa\CScene.h>
#include <game_sa\CTimeCycle.h>
#include <game_sa\CTimer.h>
#include "D3DRenderer.h"

CD3D1XShader *CCloudRendering::m_pRenderCloudsPS         = nullptr;
CD3D1XShader *CCloudRendering::m_pVolumetricCombinePS          = nullptr;
RwRaster *    CCloudRendering::m_pCloudsRaster         = nullptr;
RwRaster *    CCloudRendering::m_pNoise3DRaster     = nullptr;
CD3D1XConstantBuffer<CBCloudRendering> *CCloudRendering::m_pCloudRenderingCB =
    nullptr;
bool                            CCloudRendering::m_bRequiresReloading = false;
CloudRenderingSettingsBlock gCloudRenderingSettings;
void CCloudRendering::Init()
{
    m_pRenderCloudsPS = new CD3D1XPixelShader(
        "shaders/CloudRendering.hlsl", "RenderCloudsPS",
        &gCloudRenderingSettings.m_ShaderDefineList );
    m_pVolumetricCombinePS = new CD3D1XPixelShader(
        "shaders/CloudRendering.hlsl", "CloudsCombinePS",
        &gCloudRenderingSettings.m_ShaderDefineList );
    float renderingScale =
        gCloudRenderingSettings.GetFloat( "CloudsRenderingScale" );
    m_pCloudsRaster =
        RwRasterCreate( ( RwInt32 )( RsGlobal.maximumWidth * renderingScale ),
                        ( RwInt32 )( RsGlobal.maximumHeight * renderingScale ),
                        32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555 );
    // Volume cloud generation
    GenerateCloudsTexture();

    m_pCloudRenderingCB = new CD3D1XConstantBuffer<CBCloudRendering>();
    gCloudRenderingSettings.m_aShaderPointers.push_back( m_pRenderCloudsPS );
    gCloudRenderingSettings.m_aShaderPointers.push_back(
        m_pVolumetricCombinePS );
    gDebugSettings.DebugRenderTargetList.push_back( m_pCloudsRaster );
}

void CCloudRendering::Shutdown()
{
    delete m_pCloudRenderingCB;
    delete m_pRenderCloudsPS;
    delete m_pVolumetricCombinePS;
    if ( m_pCloudsRaster )
        RwRasterDestroy( m_pCloudsRaster );
    if ( m_pNoise3DRaster )
        RwRasterDestroy( m_pNoise3DRaster );
}

void CCloudRendering::RenderVolumetricEffects( RwRaster *normalsDepth,
                                               RwRaster *from, RwRaster *to )
{
    m_pCloudRenderingCB->data.CloudCoverage = CWeather::CloudCoverage;
    m_pCloudRenderingCB->data.CloudsColor[0] =
        CTimeCycle::m_CurrentColours.m_nFluffyCloudsBottomRed / 255.0f;
    m_pCloudRenderingCB->data.CloudsColor[1] =
        CTimeCycle::m_CurrentColours.m_nFluffyCloudsBottomGreen / 255.0f;
    m_pCloudRenderingCB->data.CloudsColor[2] =
        CTimeCycle::m_CurrentColours.m_nFluffyCloudsBottomBlue / 255.0f;
    m_pCloudRenderingCB->data.CloudsColor[3] =
        CTimeCycle::m_CurrentColours.m_fCloudAlpha/255.0f;
    m_pCloudRenderingCB->data.CloudStartHeight = 
        gCloudRenderingSettings.GetFloat( "CloudStartHeight" );
    m_pCloudRenderingCB->data.CloudEndHeight =
        gCloudRenderingSettings.GetFloat( "CloudEndHeight" );
    m_pCloudRenderingCB->data.CloudSpeed =
        gCloudRenderingSettings.GetFloat( "CloudSpeed" );
    
    m_pCloudRenderingCB->data.WindDirX = CWeather::WindDir.x;
    m_pCloudRenderingCB->data.WindDirY = CWeather::WindDir.y;
    m_pCloudRenderingCB->data.WindDirZ = CWeather::WindDir.z;
    constexpr auto time_interval       = 1.0f / ( 0.0005f ) * 3000.0f;
    if ( m_pCloudRenderingCB->data.Time < time_interval )
        m_pCloudRenderingCB->data.Time += CTimer::ms_fTimeStep;
    else
        m_pCloudRenderingCB->data.Time = m_pCloudRenderingCB->data.Time -
                                         time_interval + CTimer::ms_fTimeStep;
    
    m_pCloudRenderingCB->Update();

    g_pStateMgr->SetConstantBufferPS( m_pCloudRenderingCB, 10 );
    // First render all volumetric lighting(sun light, volume streetlights,
    // clouds etc.)
    g_pRwCustomEngine->SetRenderTargets( &m_pCloudsRaster, nullptr, 1 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetRaster( normalsDepth, 1 );
    g_pStateMgr->SetTextureAdressUV(
        RwTextureAddressMode::rwTEXTUREADDRESSWRAP );
    g_pStateMgr->SetRaster( m_pNoise3DRaster, 3 );

    m_pRenderCloudsPS->Set();
    CFullscreenQuad::Draw();
    // Than combine them with
    g_pRwCustomEngine->SetRenderTargets( &to, Scene.m_pRwCamera->zBuffer, 1 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetRaster( from );
    g_pStateMgr->SetRaster( m_pCloudsRaster, 2 );
    m_pVolumetricCombinePS->Set();

    CFullscreenQuad::Draw();

    g_pStateMgr->SetRaster( nullptr );
    g_pStateMgr->SetRaster( nullptr, 1 );
    g_pStateMgr->SetRaster( nullptr, 2 );
    g_pStateMgr->SetRaster( nullptr, 3 );
}

void CCloudRendering::QueueTextureReload() {
    CRwD3D1XEngine *dxEngine = (CRwD3D1XEngine *)g_pRwCustomEngine;

    if ( m_bRequiresReloading || dxEngine->m_bScreenSizeChanged )
    {
        float renderingScale =
            gCloudRenderingSettings.GetFloat( "CloudsRenderingScale" );
        m_pCloudsRaster->width =
            (int)( RsGlobal.maximumWidth * renderingScale );
        m_pCloudsRaster->height =
            (int)( RsGlobal.maximumHeight * renderingScale );
        dxEngine->m_pRastersToReload.push_back( m_pCloudsRaster );
    }
}

void CCloudRendering::GenerateCloudsTexture() {
    m_pNoise3DRaster = RwRasterCreate(
        128, 128, 128, ( rwRASTERTYPECAMERATEXTURE + 1 ) | rwRASTERFORMAT565 );
    auto raster_impl = GetD3D1XRaster( m_pNoise3DRaster );

    std::vector<RwV4d> pixel_array;
    pixel_array.resize( 128 * 128 * 128 );
    FastNoiseSIMD *noise_gen = FastNoiseSIMD::NewFastNoiseSIMD();
    float *noiseSet = noise_gen->GetCellularSet( 0, 0, 0, 128, 128, 128, 2.0F );
    float *noiseSet2 =
        noise_gen->GetCellularSet( 0, 0, 0, 128, 128, 128, 4.0F );
    float *noiseSet3 =
        noise_gen->GetCellularSet( 0, 0, 0, 128, 128, 128, 8.0F );
    float *perlinSet =
        noise_gen->GetPerlinFractalSet( 0, 0, 0, 128, 128, 128, 8.0F );
    size_t idx = 0;
    for ( size_t x = 0; x < 128; x++ )
    {
        for ( size_t y = 0; y < 128; y++ )
        {
            size_t y_slice_offset = 128 * 128 * x;
            for ( size_t z = 0; z < 128; z++ )
            {
                size_t z_slice_offset                            = 128 * y;
                pixel_array[y_slice_offset + z_slice_offset + z] = {
                    ( 1.0f - noiseSet[idx] ), ( 1.0f - noiseSet2[idx] ),
                    ( 1.0f - noiseSet3[idx] ), perlinSet[idx++]};
            }
        }
    }
    FastNoiseSIMD::FreeNoiseSet( noiseSet );
    FastNoiseSIMD::FreeNoiseSet( noiseSet2 );
    FastNoiseSIMD::FreeNoiseSet( noiseSet3 );
    FastNoiseSIMD::FreeNoiseSet( perlinSet );

    D3D11_SUBRESOURCE_DATA texture_data{};
    texture_data.pSysMem          = pixel_array.data();
    texture_data.SysMemPitch      = 128 * 128 * sizeof( RwV4d );
    texture_data.SysMemSlicePitch = 128 * sizeof( RwV4d );
    raster_impl->resourse =
        new CD3D1X3DTexture( m_pNoise3DRaster, &texture_data );

    CD3D1XTextureMemoryManager::AddNew( raster_impl->resourse );
}

void CloudRenderingSettingsBlock::Load( const tinyxml2::XMLDocument &doc ) 
{
    SettingsBlock::Load( doc );
    m_ShaderDefineList.AddDefine(
        "CLOUD_RM_STEPS", std::to_string( gCloudRenderingSettings.GetUInt(
                              "CloudsRaymarchingSteps" ) ) );
    m_ShaderDefineList.AddDefine(
        "CLOUD_TO_SUN_RM_STEPS", std::to_string( gCloudRenderingSettings.GetUInt(
                              "SunRaymarchingSteps" ) ) );
    
}

void TW_CALL ReloadCloudShadersCallBack( void *value )
{
    gCloudRenderingSettings.m_bShaderReloadRequired = true;
    gCloudRenderingSettings.m_ShaderDefineList.Reset();
    gCloudRenderingSettings.m_ShaderDefineList.AddDefine(
        "CLOUD_RM_STEPS",
        std::to_string( gCloudRenderingSettings.GetUInt(
                              "CloudsRaymarchingSteps" ) ) );
    gCloudRenderingSettings.m_ShaderDefineList.AddDefine(
        "CLOUD_TO_SUN_RM_STEPS",
        std::to_string(
            gCloudRenderingSettings.GetUInt( "CloudsRaymarchingSteps" ) ) );
}
void TW_CALL ReloadCloudTexturesCallBack( void *value )
{
    CCloudRendering::m_bRequiresReloading = true;
}

void CloudRenderingSettingsBlock::InitGUI( TwBar *mainBar )
{
    SettingsBlock::InitGUI( mainBar );
    TwAddButton( mainBar, "Reload cloud shaders", ReloadCloudShadersCallBack,
                 nullptr,
                 "group=CloudRendering" );
    TwAddButton( mainBar, "Reload cloud textures", ReloadCloudTexturesCallBack,
                 nullptr,
                 "group=CloudRendering" );
}
