// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "VoxelOctreeRenderer.h"
#include "D3DRenderer.h"
#include "RwD3D1XEngine.h"
#include "D3D1XTexture.h"
#include "LightManager.h"
#include "D3D1XStateManager.h"
#include <game_sa\CScene.h>

//CBVoxel CVoxelOctreeRenderer::cb_voxel;
CD3D1XConstantBuffer<CBVoxel>* CVoxelOctreeRenderer::m_pVoxelCB;
RwCamera* CVoxelOctreeRenderer::m_pVoxelCamera = nullptr;
RwRaster* CVoxelOctreeRenderer::voxelRadiance[3];
RwRaster* CVoxelOctreeRenderer::voxelClipMap[3];
CD3D1XComputeShader* CVoxelOctreeRenderer::m_voxelCS = nullptr;
CD3D1XComputeShader* CVoxelOctreeRenderer::m_radianceInjectCS = nullptr;
void CVoxelOctreeRenderer::Init()
{

    m_voxelCS = new CD3D1XComputeShader( "shaders/VoxelStuffCS.hlsl", "FillVolume" );
    m_radianceInjectCS = new CD3D1XComputeShader( "shaders/VoxelStuffCS.hlsl", "InjectRadiance" );
    m_pVoxelCamera = RwCameraCreate();
    RwCameraSetProjection( m_pVoxelCamera, rwPARALLEL );

    RwCameraSetNearClipPlane( m_pVoxelCamera, -200 );
    RwCameraSetFarClipPlane( m_pVoxelCamera, 400 );

    //m_pShadowCamera->projectionType = rwPARALLEL;

    // Init all voxel textures
    for ( size_t i = 0; i < 3; i++ )
    {
        voxelRadiance[i] = RwRasterCreate( voxelTreeSize, voxelTreeSize, voxelTreeSize, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT888 );
        voxelClipMap[i] = RwRasterCreate( voxelTreeSize, voxelTreeSize, voxelTreeSize, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT888 );
    }

    RwCameraSetRaster( m_pVoxelCamera, voxelClipMap[0] );
    RwCameraSetZRaster( m_pVoxelCamera, RwRasterCreate( voxelTreeSize, voxelTreeSize, 32, rwRASTERTYPEZBUFFER ) );

    SetVoxelLODSize( 1 );
    //CameraSize(m_pShadowCamera, 0, tan(3.14f / 2), 1.0f);
    RwObjectHasFrameSetFrame( m_pVoxelCamera, RwFrameCreate() );
    RpWorldAddCamera( Scene.m_pRpWorld, m_pVoxelCamera );
    m_pVoxelCB = new CD3D1XConstantBuffer<CBVoxel>();
}



void CVoxelOctreeRenderer::Shutdown()
{
    delete m_pVoxelCB;
    for ( size_t i = 0; i < 3; i++ )
    {
        RwRasterDestroy( voxelRadiance[i] );
        RwRasterDestroy( voxelClipMap[i] );
    }
    RwRasterDestroy( m_pVoxelCamera->zBuffer );
    RwCameraDestroy( m_pVoxelCamera );
    delete m_voxelCS;
    delete m_radianceInjectCS;
}

void CVoxelOctreeRenderer::CleanVoxelOctree()
{
    m_voxelCS->Set();

    auto context = GET_D3D_CONTEXT;
    for ( size_t i = 0; i < 3; i++ )
    {
        //auto uav = GetD3D1XRaster(voxelClipMap[i])->resourse->GetUAV();
        //context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
        context->Dispatch( voxelTreeSize / 4, voxelTreeSize / 4, voxelTreeSize / 4 );
    }

    m_voxelCS->ReSet();

    ID3D11UnorderedAccessView* uavs[] = { nullptr };
    context->CSSetUnorderedAccessViews( 0, 1, uavs, nullptr );
}

void CVoxelOctreeRenderer::InjectRadiance( RwRaster * shadow, void( *emmissiveObjRender )( ), int voxelTextureID )
{
    auto context = GET_D3D_CONTEXT;
    m_radianceInjectCS->Set();

    g_pStateMgr->SetRasterCS( voxelClipMap[voxelTextureID], 0 );
    g_pStateMgr->SetRasterCS( shadow, 1 );
    g_pStateMgr->SetConstantBufferCS( CLightManager::GetBuffer(), 10 );

    //auto uav = GetD3D1XRaster(voxelRadiance[voxelTextureID])->resourse->GetUAV();
    //context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
    context->Dispatch( voxelTreeSize / 4, voxelTreeSize / 4, voxelTreeSize / 4 );

    m_radianceInjectCS->ReSet();

    ID3D11UnorderedAccessView* uavs[] = { nullptr };
    context->CSSetUnorderedAccessViews( 0, 1, uavs, nullptr );
    g_pStateMgr->SetRasterCS( nullptr, 0 );
    g_pStateMgr->SetRasterCS( nullptr, 1 );
}
void CVoxelOctreeRenderer::FilterVoxelOctree()
{
    //auto context = ((CRwD3D1XEngine*)g_pRwCustomEngine)->getRenderer()->getContext();
    //context->GenerateMips(GetD3D1XRaster(voxelRadiance)->resourse->getSRV());
    //context->GenerateMips(GetD3D1XRaster(voxelNormals)->resourse->getSRV());
}
//C886D4     AmbientLightColourForFrame RwRGBAReal
#define AmbientLightColourForFrame (*(RwRGBAReal*)0xC886D4)
#define DirectionalLightColourForFrame (*(RwRGBAReal*)0xC886B4)

// Voxelizes scene using render method to choosen level of detail volume.
void CVoxelOctreeRenderer::RenderToVoxelOctree( void( *render )( ), const int &lod )
{
    CD3DRenderer* renderer = ( (CRwD3D1XEngine*)g_pRwCustomEngine )->getRenderer();
    ID3D11DeviceContext* context = renderer->getContext();

    // Sets global voxel properties.
    m_pVoxelCB->Update();
    auto buff = m_pVoxelCB->getBuffer();
    context->GSSetConstantBuffers( 4, 1, &buff );
    //context->VSSetConstantBuffers(4, 1, &m_pVoxelCB);
    g_pStateMgr->SetConstantBufferPS( m_pVoxelCB, 4 );
    g_pStateMgr->SetConstantBufferCS( m_pVoxelCB, 4 );

    renderer->BeginDebugEvent( L"Voxel Buffer" );

    RwCameraSetRaster( m_pVoxelCamera, voxelClipMap[lod - 1] );

    RwCameraClear( m_pVoxelCamera, gColourTop, rwCAMERACLEARZ );

    RwCameraBeginUpdate( m_pVoxelCamera );
    g_pRwCustomEngine->RenderStateSet( rwRENDERSTATEZTESTENABLE, 0 );
    render();
    g_pRwCustomEngine->RenderStateSet( rwRENDERSTATEZTESTENABLE, 1 );
    RwCameraEndUpdate( m_pVoxelCamera );

    ID3D11UnorderedAccessView* uavs = nullptr;
    context->OMSetRenderTargetsAndUnorderedAccessViews( 0, { nullptr }, nullptr, 3, 1, &uavs, nullptr );

    renderer->EndDebugEvent();
}

void CVoxelOctreeRenderer::SetVoxelTex( RwRGBA &color )
{
    RwRGBA rgba;
    RwRGBAFromRwRGBAReal( &rgba, &DirectionalLightColourForFrame );

    g_pRwCustomEngine->RenderStateSet( RwRenderState::rwRENDERSTATEBORDERCOLOR, RWRGBALONG( 0, 0, 0, 0 ) );
    g_pRwCustomEngine->RenderStateSet( RwRenderState::rwRENDERSTATETEXTUREADDRESS, rwTEXTUREADDRESSBORDER );
    g_pRwCustomEngine->RenderStateSet( RwRenderState::rwRENDERSTATETEXTUREFILTER, rwFILTERLINEAR );

    /*ID3D11ShaderResourceView* srv[] = { GetD3D1XRaster(voxelRadiance[0])->resourse->GetSRV(),
                                        GetD3D1XRaster(voxelRadiance[1])->resourse->GetSRV(),
                                        GetD3D1XRaster(voxelRadiance[2])->resourse->GetSRV(), };
    GET_D3D_CONTEXT->PSSetShaderResources(6, 3, srv);*/
}

void CVoxelOctreeRenderer::SetVoxelLODSize( const int & lod )
{
    RwV2d vw;
    vw.x = vw.y = (float)( ( voxelTreeSize / 2 ) / ( 1 << ( 3 - lod ) ) );
    m_pVoxelCB->data.voxelGridScale = (float)( 1 << ( 3 - lod ) );
    m_pVoxelCB->Update();
    RwCameraSetViewWindow( m_pVoxelCamera, &vw );
}
