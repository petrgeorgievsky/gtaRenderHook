// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "bilateralblurpass.h"
#include "forward_pbr_pipeline.h"
#include "gbuffer_pipeline.h"
#include "ray_tracer.h"
#include "stdafx.h"
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <RwRenderEngine.h>
#include <bvh_builder.h>
#include <common.h>
#include <ray_tracing_scene_cache.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_game_hooks.h>

using namespace RHEngine;
using namespace rw_rh_engine;
class CColourSet
{
public:
    float m_fAmbientRed;
    float m_fAmbientGreen;
    float m_fAmbientBlue;
    float m_fAmbientRed_Obj;
    float m_fAmbientGreen_Obj;
    float m_fAmbientBlue_Obj;
    float m_fDirectionalRed;
    float m_fDirectionalGreen;
    float m_fDirectionalBlue;
    unsigned short m_nSkyTopRed;
    unsigned short m_nSkyTopGreen;
    unsigned short m_nSkyTopBlue;
    unsigned short m_nSkyBottomRed;
    unsigned short m_nSkyBottomGreen;
    unsigned short m_nSkyBottomBlue;
    unsigned short m_nSunCoreRed;
    unsigned short m_nSunCoreGreen;
    unsigned short m_nSunCoreBlue;
    unsigned short m_nSunCoronaRed;
    unsigned short m_nSunCoronaGreen;
    unsigned short m_nSunCoronaBlue;
    float m_fSunSize;
    float m_fSpriteSize;
    float m_fSpriteBrightness;
    unsigned short m_nShadowStrength;
    unsigned short m_nLightShadowStrength;
    unsigned short m_nPoleShadowStrength;
    float m_fFarClip;
    float m_fFogStart;
    float m_fLightsOnGroundBrightness;
    unsigned short m_nLowCloudsRed;
    unsigned short m_nLowCloudsGreen;
    unsigned short m_nLowCloudsBlue;
    unsigned short m_nFluffyCloudsBottomRed;
    unsigned short m_nFluffyCloudsBottomGreen;
    unsigned short m_nFluffyCloudsBottomBlue;
    float m_fWaterRed;
    float m_fWaterGreen;
    float m_fWaterBlue;
    float m_fWaterAlpha;
    float m_fPostFx1Red;
    float m_fPostFx1Green;
    float m_fPostFx1Blue;
    float m_fPostFx1Alpha;
    float m_fPostFx2Red;
    float m_fPostFx2Green;
    float m_fPostFx2Blue;
    float m_fPostFx2Alpha;
    float m_fCloudAlpha;
    unsigned int m_nHighLightMinIntensity;
    unsigned short m_nWaterFogAlpha;
    float m_fIllumination;
    float m_fLodDistMult;
};
bool true_ret_hook()
{
    return true;
}
bool false_ret_hook()
{
    return false;
}

static rw_raytracing_lib::BVHBuilder *bvh_builder = nullptr;
static rw_raytracing_lib::RayTracingScene *rt_scene = nullptr;
static std::vector<rw_raytracing_lib::BLAS_Instance> g_currFrameBLASInst;
static std::vector<RwIm2DVertex> m_vQuad;
static RayTracer g_rayTracer;
static GBufferPipeline *g_gbPipeline = nullptr;
static BilateralBlurPass *g_blurPass = nullptr;
void GenerateQuad( float w, float h )
{
    m_vQuad.clear();

    RwIm2DVertex vtx{};
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    vtx.emissiveColor = 0xFFFFFFFF;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.u = 0;
    vtx.y = h;
    vtx.v = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = vtx.v = 0;
    vtx.u = 1;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );
}
static RsGlobalType &g_rsGlobal = *reinterpret_cast<RsGlobalType *>( 0xC17040 );
void render_rt()
{
    //    std::stringstream ss;
    //    ss << g_rsGlobal.maximumWidth << g_rsGlobal.maximumHeight;
    //    RHDebug::DebugLogger::Log( ss.str() );
    auto packed_rt = rt_scene->PackBLAS();
    if ( packed_rt.second )
        g_rayTracer.AllocatePackedBLASGPU( packed_rt.first );
    if ( g_currFrameBLASInst.size() <= 0 )
        return;
    g_rayTracer.AllocateBVHOnGPU( rt_scene->GenerateTLAS( g_currFrameBLASInst ) );
    g_pRwRenderEngine->RenderStateSet( rwRENDERSTATECULLMODE, rwCULLMODECULLNONE );
    g_rayTracer.TraceRays( g_gbPipeline->GetGBuffer( 0 ),
                           g_gbPipeline->GetGBuffer( 1 ),
                           g_gbPipeline->GetGBuffer( 2 ),
                           g_gbPipeline->GetGBuffer( 3 ),
                           rt_scene->GetTextureCache()->GetPoolPtr() );
    void *accum_tex = g_rayTracer.Accumulate( g_rayTracer.RaytracedTex() );
    void *blurred_tex = g_blurPass->Blur( accum_tex,
                                          g_gbPipeline->GetGBuffer( 1 ),
                                          g_gbPipeline->GetGBuffer( 0 ) );
    void *composite_tex = g_rayTracer.Composite( blurred_tex, g_gbPipeline->GetGBuffer( 2 ) );
    rw_rh_engine::g_pRwRenderEngine->m_p2DRenderer->BindTexture( composite_tex );
    rw_rh_engine::RwIm2DRenderPrimitive( RwPrimitiveType::rwPRIMTYPETRILIST,
                                         m_vQuad.data(),
                                         static_cast<int32_t>( m_vQuad.size() ) );
    rw_rh_engine::g_pRwRenderEngine->m_p2DRenderer->BindTexture( nullptr );
    rw_rh_engine::g_pRwRenderEngine->RenderStateSet( rwRENDERSTATETEXTURERASTER, 0 );
    g_cameraContext->deltas.x = ( (float) ( rand() % 10000 ) ) / 10000.0f;
    g_cameraContext->deltas.y = ( (float) ( rand() % 10000 ) ) / 10000.0f;
    g_currFrameBLASInst.clear();
    rt_scene->TidyUp();
}

static void *gBaseConstantBuffer = nullptr;
static void *gPerModelConstantBuffer = nullptr;
static ForwardPBRPipeline *g_pForwardPBRPipeline = nullptr;

#define RwFrameGetLTM( frame ) \
    ( reinterpret_cast<RwMatrix *(__cdecl *) ( RwFrame * )>( 0x7F0990 ) )( frame )

static RwCamera *&camera_ptr = *reinterpret_cast<RwCamera **>( 0xC1703C );
static uint32_t &visible_entity_num = *reinterpret_cast<uint32_t *>( 0xB76844 );
static uint32_t &visible_lod_num = *reinterpret_cast<uint32_t *>( 0xB76840 );

static void **visible_entity_ptr = reinterpret_cast<void **>( 0xB75898 );
static void **visible_lod_ptr = reinterpret_cast<void **>( 0xB748F8 );
static void *sky_top_r = reinterpret_cast<char **>( 0xB748F8 );
static uint32_t &current_tc_id = *reinterpret_cast<uint32_t *>( 0xB79FD0 );
static DirectX::XMFLOAT3 *sun_vectors = reinterpret_cast<DirectX::XMFLOAT3 *>( 0xB7CA50 );
static DirectX::XMFLOAT3 &amb_color = *reinterpret_cast<DirectX::XMFLOAT3 *>( 0xB7C4AC );
static CColourSet &gCurrentColours{*reinterpret_cast<CColourSet *>( 0xB7C4A0 )};

void DrawEntity( void *entity_ptr )
{
    ( reinterpret_cast<void( __cdecl * )( void * )>( 0x553260 ) )( entity_ptr );
}

void RenderScene()
{
    if ( g_pForwardPBRPipeline == nullptr ) {
        IGPUAllocator *allocator = g_pRHRenderer->GetGPUAllocator();

        allocator->AllocateConstantBuffer( {sizeof( CameraContext )}, gBaseConstantBuffer );
        allocator->AllocateConstantBuffer( {sizeof( DirectX::XMMATRIX ) * 2},
                                           gPerModelConstantBuffer );
        g_pForwardPBRPipeline = new ForwardPBRPipeline();
        g_gbPipeline = new GBufferPipeline();
        bvh_builder = new rw_raytracing_lib::BVHBuilder();
        rt_scene = new rw_raytracing_lib::RayTracingScene( bvh_builder );
        g_rayTracer.Initialize();
        g_blurPass = new BilateralBlurPass();
        GenerateQuad( 1600, 900 );
    }
    DirectX::XMFLOAT4 sky_color{gCurrentColours.m_nSkyTopRed / 255.0f,
                                gCurrentColours.m_nSkyTopGreen / 255.0f,
                                gCurrentColours.m_nSkyTopBlue / 255.0f,
                                1};
    DirectX::XMFLOAT4 sky_color_2{gCurrentColours.m_nSkyBottomRed / 255.0f,
                                  gCurrentColours.m_nSkyBottomGreen / 255.0f,
                                  gCurrentColours.m_nSkyBottomBlue / 255.0f,
                                  1};
    g_rayTracer.UpdateTimecyc( sky_color,
                               sky_color_2,
                               {sun_vectors[current_tc_id].x,
                                sun_vectors[current_tc_id].y,
                                sun_vectors[current_tc_id].z,
                                0} );
    auto context = static_cast<RHEngine::IRenderingContext *>( g_pRHRenderer->GetCurrentContext() );
    g_pRwRenderEngine->RenderStateSet( rwRENDERSTATECULLMODE, rwCULLMODECULLNONE );
    g_gbPipeline->PrepareFrame( context );
    for ( uint32_t i = 0; i < visible_entity_num; i++ ) {
        DrawEntity( visible_entity_ptr[i] );
    }
    for ( uint32_t i = 0; i < visible_lod_num; i++ ) {
        DrawEntity( visible_lod_ptr[i] );
    }
    ( reinterpret_cast<void( __cdecl * )()>( 0x733F10 ) )();
    g_gbPipeline->EndFrame( context );

    auto frameBufferInternal = GetInternalRaster( camera_ptr->frameBuffer );
    auto zBufferInternal = GetInternalRaster( camera_ptr->zBuffer );
    RHEngine::g_pRHRenderer->BindImageBuffers( RHEngine::ImageBindType::RenderTarget,
                                               {{0, frameBufferInternal}} );
    RHEngine::g_pRHRenderer->BindImageBuffers( RHEngine::ImageBindType::DepthStencilTarget,
                                               {{0, zBufferInternal}} );
}

static RpGeometryRw36 geometry_interface_35{};
static RwBool D3D8AtomicAllInOneNode( RxPipelineNodeInstance * /*self*/,
                                      const RxPipelineNodeParam *params )
{
    RpAtomic *atomic;

    atomic = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *geom = atomic->geometry;
    RpMeshHeader *meshHeader = geom->mesh;
    geometry_interface_35.Init( geom );
    rt_scene->PushModel( meshHeader->serialNum, &geometry_interface_35 );
    if ( RwRHInstanceAtomic( atomic, &geometry_interface_35 ) != RenderStatus::Instanced )
        return false;

    IRenderingContext *context = static_cast<IRenderingContext *>(
        g_pRHRenderer->GetCurrentContext() );
    const RwMatrix *ltm = RwFrameGetLTM( static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DirectX::XMMATRIX objTransformMatrix = {ltm->right.x,
                                            ltm->right.y,
                                            ltm->right.z,
                                            0,
                                            ltm->up.x,
                                            ltm->up.y,
                                            ltm->up.z,
                                            0,
                                            ltm->at.x,
                                            ltm->at.y,
                                            ltm->at.z,
                                            0,
                                            ltm->pos.x,
                                            ltm->pos.y,
                                            ltm->pos.z,
                                            1};
    struct PerModelCB
    {
        DirectX::XMMATRIX worldMatrix;
        DirectX::XMMATRIX worldITMatrix;
    } perModelCB;
    perModelCB.worldMatrix = objTransformMatrix;
    perModelCB.worldITMatrix = DirectX::XMMatrixInverse( nullptr, objTransformMatrix );
    g_pRwRenderEngine->RenderStateSet( rwRENDERSTATEVERTEXALPHAENABLE, 1 );
    context->UpdateBuffer( gBaseConstantBuffer, g_cameraContext, sizeof( *g_cameraContext ) );
    context->UpdateBuffer( gPerModelConstantBuffer, &perModelCB, sizeof( perModelCB ) );
    context->BindConstantBuffers( RHEngine::RHShaderStage::Vertex | RHEngine::RHShaderStage::Pixel
                                      | RHEngine::RHShaderStage::Compute,
                                  {{0, gBaseConstantBuffer}, {1, gPerModelConstantBuffer}} );
    rw_raytracing_lib::BLAS_Instance inst;
    inst.blas_id = meshHeader->serialNum;
    DirectX::XMStoreFloat4x4( &inst.world_transform, objTransformMatrix );
    g_currFrameBLASInst.push_back( inst );
    DrawAtomic( atomic, &geometry_interface_35, context, g_gbPipeline );

    return ( TRUE );
}
static RwBool rxD3D8SubmitNode( RxPipelineNodeInstance *, const RxPipelineNodeParam * )
{
    return true;
}

void empty_void() {}
void *rwD3D9RasterDtor( void *object )
{
    return object;
}

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/ )
{
    switch ( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
        g_pIO_API = {reinterpret_cast<RwStreamFindChunk_FN>( 0x7ED2D0 ),
                     reinterpret_cast<RwStreamRead_FN>( 0x7EC9D0 )};
        g_pGlobal_API = {reinterpret_cast<RwSystemFunc>( 0x7F5F70 ),
                         reinterpret_cast<uint32_t *>( 0xB4E9E0 ),
                         reinterpret_cast<RwResourcesAllocateResEntry_FN>( 0x807ED0 )};
        g_pRaster_API = {reinterpret_cast<RwRasterCreate_FN>( 0x7FB230 ),
                         reinterpret_cast<RwRasterDestroy_FN>( 0x7FB020 )};
        g_pTexture_API = {reinterpret_cast<RwTextureCreate_FN>( 0x7F37C0 ),
                          reinterpret_cast<RwTextureSetName_FN>( 0x7F38A0 ),
                          reinterpret_cast<RwTextureSetName_FN>( 0x7F3910 )};
        RHDebug::DebugLogger::Init( "gta_rt_rh_logs.log", RHDebug::LogLevel::Info );
        g_pRwRenderEngine = std::unique_ptr<RwRenderEngine>(
            new RwRenderEngine( RHRenderingAPI::DX11 ) ); 
        {
            RwPointerTable gtasa_ptr_table{};
            gtasa_ptr_table.m_fpRenderSystem = 0x8E249C;
            gtasa_ptr_table.m_fpIm3DOpen = 0x80A225; // 0x7EFE20;
            gtasa_ptr_table.m_fpCheckNativeTextureSupport = 0x745530;
            gtasa_ptr_table.m_fpCheckEnviromentMapSupport = 0x5D8980;
            gtasa_ptr_table.m_fpSetVideoMode = 0x7F8640;
            gtasa_ptr_table.m_fpSetRefreshRate = 0x7F8580;
            gtasa_ptr_table.m_fpSetRenderState = 0x8E24A8;
            gtasa_ptr_table.m_fpGetRenderState = 0x8E24AC;
            gtasa_ptr_table.m_fpIm2DRenderLine = 0x8E24B0;
            gtasa_ptr_table.m_fpIm2DRenderPrim = 0x8E24B8;
            gtasa_ptr_table.m_fpIm2DRenderIndexedPrim = 0x8E24BC;
            //  pipeline ptr
            RwGameHooks::Patch( gtasa_ptr_table );
        }
        {
            // RedirectJump( 0x429C00, reinterpret_cast<void *>( true_ret_hook ) );
            // RedirectCall( 0x48C20C, reinterpret_cast<void *>( false_ret_hook ) );
            // 0x4C9AB9 raster_destr; 4C9A80
            RedirectJump( 0x4C9A80, reinterpret_cast<void *>( rwD3D9RasterDtor ) );
            RedirectCall( 0x53EAD3, reinterpret_cast<void *>( render_rt ) );
            RedirectCall( 0x53EABF, reinterpret_cast<void *>( RenderScene ) );
            RedirectCall( 0x748A30,
                          reinterpret_cast<void *>( empty_void ) ); // CGammaInitialise
            RedirectCall( 0x5BD779,
                          reinterpret_cast<void *>( empty_void ) ); // CPostEffects10Initialise
            RedirectCall( 0x53EA0D,
                          reinterpret_cast<void *>( empty_void ) ); // 2CRealTimeShadowManager6Update
            RedirectCall( 0x53EA12,
                          reinterpret_cast<void *>( empty_void ) ); // CMirrors16BeforeMainRender
            RedirectCall( 0x53EABA,
                          reinterpret_cast<void *>( empty_void ) ); // CMirrors18RenderMirrorBuffer
            RedirectCall( 0x53EAC4,
                          reinterpret_cast<void *>(
                              empty_void ) ); // CVisibilityPlugins21RenderWeaponPedsForPC

            // DWORD value = 4;
            // Patch( 0x582DBB, &value, sizeof( value ) );
            SetPointer( 0x8D633C, reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
            SetPointer( 0x8DED0C,
                        reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) ); // skin
            SetPointer( 0x8E297C, reinterpret_cast<void *>( rxD3D8SubmitNode ) );
        }
        //        {
        //            byte value = 0x3;
        //            Patch( 0x48E84D, &value, 1 );
        //        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
