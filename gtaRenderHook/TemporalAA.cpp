#include "stdafx.h"
#include "TemporalAA.h"
#include "RwRenderEngine.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"
#include "D3D1XTexture.h"
#include "FullscreenQuad.h"
#include "D3D1XStateManager.h"
#include "D3D1XRenderBuffersManager.h"
#include <game_sa\CScene.h>
#include <game_sa\CCamera.h>
CD3D1XShader *CTemporalAA::m_pTemporalAA = nullptr;
RwRaster *CTemporalAA::m_pAccumRasters[2] = { nullptr, nullptr };
RwV3d CTemporalAA::m_vPrevCamPos = { 0,0,0 };
RwGraphicsMatrix CTemporalAA::m_mPrevView = {};
int CTemporalAA::m_nCurrentAccRaster = 0;
int CTemporalAA::m_nCurrentJitterSample = 0;
CD3D1XConstantBuffer<CBTemporalAA>* CTemporalAA::m_pTAABuffer = nullptr;
TAASettingsBlock gTAASettingsBlock;
bool CTemporalAA::m_bFirstTimeRender = true;
void CTemporalAA::Init()
{
    m_pAccumRasters[0] = RwRasterCreate( RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555 );
    m_pAccumRasters[1] = RwRasterCreate( RsGlobal.maximumWidth, RsGlobal.maximumHeight, 32, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT1555 );
    m_pTemporalAA = new CD3D1XPixelShader( "shaders/TemporalAA.hlsl", "TAA_PS" );
    m_pTAABuffer = new CD3D1XConstantBuffer<CBTemporalAA>();
    m_pTAABuffer->SetDebugName( "TemporalAACB" );
    m_mPrevView.m[0].x = 1.0f;
    m_mPrevView.m[1].y = 1.0f;
    m_mPrevView.m[2].z = 1.0f;
    m_mPrevView.m[3].w = 1.0f;
}

void CTemporalAA::Shutdown()
{
    RwRasterDestroy( m_pAccumRasters[0] );
    RwRasterDestroy( m_pAccumRasters[1] );
    delete m_pTemporalAA;
    delete m_pTAABuffer;
}

void CTemporalAA::JitterProjMatrix()
{
    float rcx = 1.0f / RsGlobal.maximumWidth;
    float rcy = 1.0f / RsGlobal.maximumHeight;
    float Scale = gTAASettingsBlock.GetFloat( "SubPixelScale" );
    float jittermatrixX[11] = {
        // halton seq
        0.0f, 0.5f, 0.25f, 0.75f, 0.125f, 0.625f, 0.375f, 0.875f, 0.0625f, 0.5625f, 0.3125f
    };
    float jittermatrixY[11] = {
        .0f,
        0.33333333333333331f,
        0.66666666666666663f,
        0.1111111111111111f,
        0.44444444444444442f,
        0.77777777777777768f,
        0.22222222222222221f,
        0.55555555555555558f,
        0.88888888888888884f,
        0.037037037037037035f,
        0.37037037037037035f
    };
    float x8aa_X[8] = {
        -7.0f / 8.0f,
        -5.0f / 8.0f,
        -1.0f / 8.0f,
        3.0f / 8.0f,
        5.0f / 8.0f,
        7.0f / 8.0f,
        1.0f / 8.0f,
        -3.0f / 8.0f };
    float x8aa_Y[8] = {
        1.0f / 8.0f,
        -5.0f / 8.0f,
        -3.0f / 8.0f,
        -7.0f / 8.0f,
        -1.0f / 8.0f,
        7.0f / 8.0f,
        3.0f / 8.0f,
        5.0f / 8.0f };
    float uniformX[11] = { 0.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float uniformY[11] = { 0.0f, 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f };
    RwGraphicsMatrix *projTransformRef = (RwGraphicsMatrix*)&RwD3D9D3D9ProjTransform;

    m_nCurrentJitterSample = ( m_nCurrentJitterSample++ ) % gTAASettingsBlock.GetUInt( "SubPixelCount" );
    float SampleX = ( x8aa_X[m_nCurrentJitterSample] ) * rcx;
    float SampleY = ( x8aa_Y[m_nCurrentJitterSample] ) * rcy;
    auto camera = Scene.m_pRwCamera;
    projTransformRef->m[2].x += SampleX * Scale;
    projTransformRef->m[2].y += SampleY * Scale;

    g_pRenderBuffersMgr->UpdateViewProjMatricles( RwD3D9D3D9ViewTransform, *(RwMatrix*)projTransformRef );
}

void CTemporalAA::SaveViewMatrix()
{
    RwMatrix* camMatrix = RwFrameGetLTM( static_cast<RwFrame*>( Scene.m_pRwCamera->object.object.parent ) );
    // rw used inverse of camera frame matrix with negative column-vector
    RwMatrixInvert( (RwMatrix*)&m_mPrevView, camMatrix );
    m_mPrevView.m[0].x = -m_mPrevView.m[0].x;
    m_mPrevView.m[1].x = -m_mPrevView.m[1].x;
    m_mPrevView.m[2].x = -m_mPrevView.m[2].x;
    m_mPrevView.m[3].x = -m_mPrevView.m[3].x;
    m_mPrevView.m[0].w = 0.0f;
    m_mPrevView.m[1].w = 0.0f;
    m_mPrevView.m[2].w = 0.0f;
    m_mPrevView.m[3].w = 1.0f;
}

void CTemporalAA::Render( RwRaster * input, RwRaster* output, RwRaster* gBuff1, RwRaster* gBuff2 )
{
    RwV3d* curCamPos = RwMatrixGetPos( &Scene.m_pRwCamera->viewMatrix );
    m_pTAABuffer->data.PrevView = m_mPrevView;
    m_pTAABuffer->data.MovementVec = { m_vPrevCamPos.x - curCamPos->x, m_vPrevCamPos.y - curCamPos->y, m_vPrevCamPos.z - curCamPos->z };
    m_pTAABuffer->data.BlendFactor = ( gTAASettingsBlock.GetInt( "FrameCount" ) - 1.0f ) / ( 1.0f*gTAASettingsBlock.GetInt( "FrameCount" ) );
    m_pTAABuffer->data.MBMaxSpeed = gTAASettingsBlock.GetFloat( "MBMaxSpeed" );
    m_pTAABuffer->data.MBMinPixelDistance = gTAASettingsBlock.GetFloat( "MBMinPixelDistance" );
    m_pTAABuffer->data.MBEdgeScale = gTAASettingsBlock.GetFloat( "MBEdgeScale" );
    m_pTAABuffer->data.MBCenterScale = gTAASettingsBlock.GetFloat( "MBCenterScale" );
    m_pTAABuffer->Update();
    // Set last tonemap raster to render luminance.
    g_pRwCustomEngine->SetRenderTargets( &m_pAccumRasters[m_nCurrentAccRaster], Scene.m_pRwCamera->zBuffer, 1 );
    g_pStateMgr->FlushRenderTargets();
    g_pStateMgr->SetConstantBufferPS( m_pTAABuffer, 9 );
    g_pStateMgr->SetRaster( input );
    g_pStateMgr->SetRaster( m_bFirstTimeRender ? input : m_pAccumRasters[1 - m_nCurrentAccRaster], 1 );
    g_pStateMgr->SetRaster( gBuff1, 2 );
    g_pStateMgr->SetRaster( gBuff2, 3 );
    m_pTemporalAA->Set();
    CFullscreenQuad::Draw();
    g_pStateMgr->SetRaster( nullptr );
    g_pStateMgr->SetRaster( nullptr, 1 );
    g_pStateMgr->SetRaster( nullptr, 2 );
    g_pStateMgr->SetRaster( nullptr, 3 );
    CFullscreenQuad::Copy( m_pAccumRasters[m_nCurrentAccRaster], nullptr, output );


    m_nCurrentAccRaster = 1 - m_nCurrentAccRaster;
    m_bFirstTimeRender = false;
    m_vPrevCamPos = *curCamPos;
    SaveViewMatrix();
}

void CTemporalAA::QueueTextureReload()
{
    CRwD3D1XEngine *dxEngine = dynamic_cast<CRwD3D1XEngine *>(g_pRwCustomEngine);
    if ( !dxEngine->m_bScreenSizeChanged )
        return;
    for ( int i = 0; i < 2; i++ )
    {
        m_pAccumRasters[i]->width  = RsGlobal.maximumWidth;
        m_pAccumRasters[i]->height = RsGlobal.maximumHeight;
        dxEngine->m_pRastersToReload.push_back( m_pAccumRasters[i] );
    }
}

void TAASettingsBlock::Load( const tinyxml2::XMLDocument & doc )
{
    SettingsBlock::Load( doc );
}

void TAASettingsBlock::InitGUI( TwBar * bar )
{
    SettingsBlock::InitGUI( bar );
}
