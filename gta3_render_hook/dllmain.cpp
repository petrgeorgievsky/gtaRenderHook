// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "bvh_builder.h"
#include "forward_pbr_pipeline.h"
#include "gta3_geometry_proxy.h"
#include "ray_tracer.h"
#include "ray_tracing_scene_cache.h"
#include "stdafx.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/constant_buffer_info.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <RwRenderEngine.h>
#include <common.h>
#include <gbuffer/gbuffer_pass.h>
#include <gbuffer/gbuffer_pipeline.h>
#include <ray_traced_gi/lf_gi_filter_pass.h>
#include <ray_traced_gi/per_pixel_gi_pass.h>
#include <ray_traced_shadows/rt_shadows_pass.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_game_hooks.h>

using namespace rh::engine;
using namespace rh::rw::engine;
bool true_ret_hook() { return true; }
bool false_ret_hook() { return false; }

static rw_raytracing_lib::BVHBuilder *               bvh_builder = nullptr;
static rw_raytracing_lib::RayTracingScene *          rt_scene    = nullptr;
static std::vector<rw_raytracing_lib::BLAS_Instance> g_currFrameBLASInst;
static std::vector<RwIm2DVertex>                     m_vQuad;
static rw_raytracing_lib::RayTracer                  g_rayTracer;
static rw_raytracing_lib::RTShadowsPass *            g_pShadowPass;
static rw_raytracing_lib::RTPerPixelGIPass *         g_pGIPass;
static rw_raytracing_lib::LowFreqGIFilterPass *      g_pLFGIFilterPass;
static GBufferPipeline *                             g_gbPipeline = nullptr;
static GBufferPass *                                 g_gbPass     = nullptr;
void GenerateQuad( float w, float h )
{
    m_vQuad.clear();

    RwIm2DVertex vtx{};
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    vtx.emissiveColor             = 0xFFFFFFFF;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.u = 0;
    vtx.y         = h;
    vtx.v         = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = vtx.v = 0;
    vtx.u         = 1;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );
}

void render_rt()
{
    auto packed_rt = rt_scene->PackBLAS();
    if ( packed_rt.second )
        g_rayTracer.AllocatePackedBLAS( packed_rt.first );
    g_rayTracer.AllocateTLAS( rt_scene->GenerateTLAS( g_currFrameBLASInst ) );
    // void *shadow_rt = g_pShadowPass->Execute( &g_rayTracer,
    //                                          g_gbPass->GetGBuffer( 0 ),
    //                                          g_gbPass->GetGBuffer( 1 ) );

    void *gi_rt = g_pGIPass->Execute( &g_rayTracer, g_gbPass->GetGBuffer( 0 ),
                                      g_gbPass->GetGBuffer( 1 ) );

    void *fgi_rt = g_pLFGIFilterPass->Execute(
        g_gbPass->GetGBuffer( 0 ), g_gbPass->GetGBuffer( 1 ), gi_rt,
        g_pGIPass->GetSHCoCg(), g_gbPass->GetGBuffer( 2 ),
        g_gbPass->GetGBuffer( 3 ) );

    rh::rw::engine::g_pRwRenderEngine->m_p2DRenderer->BindTexture( fgi_rt );
    rh::rw::engine::RwIm2DRenderPrimitive(
        RwPrimitiveType::rwPRIMTYPETRILIST, m_vQuad.data(),
        static_cast<int32_t>( m_vQuad.size() ) );
    rh::rw::engine::g_pRwRenderEngine->m_p2DRenderer->BindTexture( nullptr );
    rh::rw::engine::g_pRwRenderEngine->RenderStateSet(
        rwRENDERSTATETEXTURERASTER, 0 );
    g_cameraContext->deltas.x = ( (float)( rand() % 10000 ) ) / 10000.0f;
    g_cameraContext->deltas.y = ( (float)( rand() % 10000 ) ) / 10000.0f;
    g_currFrameBLASInst.clear();
    rt_scene->TidyUp();
}

static rh::engine::IGPUResource *gBaseConstantBuffer     = nullptr;
static rh::engine::IGPUResource *gPerModelConstantBuffer = nullptr;
static ForwardPBRPipeline *      g_pForwardPBRPipeline   = nullptr;

#define RwFrameGetLTM( frame )                                                 \
    ( reinterpret_cast<RwMatrix *(__cdecl *)( RwFrame * )>( 0x5A1FA0 ) )(      \
        frame )

static RwCamera *&camera_ptr = *reinterpret_cast<RwCamera **>( 0x72676C );

void RenderScene()
{
    if ( g_pForwardPBRPipeline == nullptr )
    {
        IGPUAllocator *allocator = g_pRHRenderer->GetGPUAllocator();

        allocator->AllocateConstantBuffer( {sizeof( CameraContext )},
                                           gBaseConstantBuffer );
        allocator->AllocateConstantBuffer( {sizeof( DirectX::XMMATRIX ) * 2},
                                           gPerModelConstantBuffer );
        g_pForwardPBRPipeline = new ForwardPBRPipeline();
        g_gbPipeline          = new GBufferPipeline();
        bvh_builder           = new rw_raytracing_lib::BVHBuilder();
        rt_scene = new rw_raytracing_lib::RayTracingScene( bvh_builder );
        GBufferDesc desc{};
        desc.width  = camera_ptr->frameBuffer->width;
        desc.height = camera_ptr->frameBuffer->height;
        desc.bufferFormats.push_back(
            static_cast<uint32_t>( ImageBufferFormat::RGBA32 ) );
        desc.bufferFormats.push_back(
            static_cast<uint32_t>( ImageBufferFormat::RGBA16 ) );
        desc.bufferFormats.push_back(
            static_cast<uint32_t>( ImageBufferFormat::RGBA8 ) );
        desc.bufferFormats.push_back(
            static_cast<uint32_t>( ImageBufferFormat::RGBA8 ) );
        g_gbPass      = new GBufferPass( desc );
        g_pShadowPass = new rw_raytracing_lib::RTShadowsPass(
            camera_ptr->frameBuffer->width, camera_ptr->frameBuffer->height );
        g_pGIPass = new rw_raytracing_lib::RTPerPixelGIPass(
            camera_ptr->frameBuffer->width, camera_ptr->frameBuffer->height,
            0.25f );
        g_pLFGIFilterPass = new rw_raytracing_lib::LowFreqGIFilterPass(
            camera_ptr->frameBuffer->width, camera_ptr->frameBuffer->height,
            0.25f );
        GenerateQuad( camera_ptr->frameBuffer->width,
                      camera_ptr->frameBuffer->height );
    }
    auto context =
        static_cast<IRenderingContext *>( g_pRHRenderer->GetCurrentContext() );
    g_gbPass->PrepareFrame( context );
    ( reinterpret_cast<void( __cdecl * )()>( 0x48E0F0 ) )();
    g_gbPass->EndFrame( context );

    auto frameBufferInternal = GetInternalRaster( camera_ptr->frameBuffer );
    g_pRHRenderer->BindImageBuffers( ImageBindType::RenderTarget,
                                     {{0, frameBufferInternal}} );
}

static RpGeometryRw35 geometry_interface_35{};
static RwBool
SkinD3D8AtomicAllInOneNode( RxPipelineNodeInstance * /*self*/,
                            const RxPipelineNodeParam * /*params*/ )
{
    return true;
}
static RwBool D3D8AtomicAllInOneNode( RxPipelineNodeInstance * /*self*/,
                                      const RxPipelineNodeParam *params )
{
    RpAtomic *atomic;

    atomic = static_cast<RpAtomic *>( params->dataParam );
    RpGeometryGTA3 *geom =
        reinterpret_cast<RpGeometryGTA3 *>( atomic->geometry );
    RpMeshHeader *meshHeader = geom->mesh;
    // geometry_interface_35.Init( geom );
    // rt_scene->PushModel( meshHeader->serialNum, &geometry_interface_35 );
    // if ( RwRHInstanceAtomic( atomic, &geometry_interface_35 ) !=
    // RenderStatus::Instanced )
    return false;

    IRenderingContext *context =
        static_cast<IRenderingContext *>( g_pRHRenderer->GetCurrentContext() );
    const RwMatrix *ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DirectX::XMMATRIX objTransformMatrix = {
        ltm->right.x, ltm->right.y, ltm->right.z, 0,
        ltm->up.x,    ltm->up.y,    ltm->up.z,    0,
        ltm->at.x,    ltm->at.y,    ltm->at.z,    0,
        ltm->pos.x,   ltm->pos.y,   ltm->pos.z,   1};
    struct PerModelCB
    {
        DirectX::XMMATRIX worldMatrix;
        DirectX::XMMATRIX worldITMatrix;
    } perModelCB;
    perModelCB.worldMatrix = objTransformMatrix;
    perModelCB.worldITMatrix =
        DirectX::XMMatrixInverse( nullptr, objTransformMatrix );
    g_pRwRenderEngine->RenderStateSet( rwRENDERSTATEVERTEXALPHAENABLE, 1 );
    context->UpdateBuffer( gBaseConstantBuffer, g_cameraContext,
                           sizeof( *g_cameraContext ) );
    context->UpdateBuffer( gPerModelConstantBuffer, &perModelCB,
                           sizeof( perModelCB ) );
    context->BindConstantBuffers(
        ShaderStage::Vertex | ShaderStage::Pixel | ShaderStage::Compute,
        {{0, gBaseConstantBuffer}, {1, gPerModelConstantBuffer}} );

    DrawAtomic( atomic, &geometry_interface_35, context, g_gbPipeline );

    rw_raytracing_lib::BLAS_Instance inst;
    inst.blas_id = meshHeader->serialNum;
    DirectX::XMStoreFloat4x4( &inst.world_transform, objTransformMatrix );
    g_currFrameBLASInst.push_back( inst );

    return ( TRUE );
}
static RwBool rxD3D8SubmitNode( RxPipelineNodeInstance *,
                                const RxPipelineNodeParam * )
{
    return true;
}

void debug_log( char *format... )
{
    std::string str( 250, '\0' );
    va_list     args;
    va_start( args, format );

    vsprintf_s( str.data(), 250, format, args );

    va_end( args );

    rh::debug::DebugLogger::Log( "GTA3LOGS:" + str );
}

RwInt32 rwD3D8FindCorrectRasterFormat( RwRasterType type, RwInt32 flags )
{
    RwUInt32 format = flags & rwRASTERFORMATMASK;
    switch ( type )
    {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            /* Check if we are requesting a default pixel format palette texture
             */
            if ( format & rwRASTERFORMATPAL8 )
            {
                format |= rwRASTERFORMAT8888;
            }
            if ( ( format & rwRASTERFORMATPAL8 ) == 0 )
            {
                format |= rwRASTERFORMAT8888;
            }
        }
        else
        {
            /* No support for 4 bits palettes */
            if ( format & rwRASTERFORMATPAL4 )
            {
                /* Change it to a 8 bits palette */
                format &= static_cast<RwUInt32>( ~rwRASTERFORMATPAL4 );

                format |= rwRASTERFORMATPAL8;
            }
        }
        break;
    case rwRASTERTYPECAMERATEXTURE:
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format |= rwRASTERFORMAT888;
        }
        break;

    case rwRASTERTYPECAMERA:
        /* Always force default */
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format = rwRASTERFORMAT8888;
        }
        break;

    case rwRASTERTYPEZBUFFER:
        /* Always force default */
        if ( ( format & rwRASTERFORMATPIXELFORMATMASK ) ==
             rwRASTERFORMATDEFAULT )
        {
            format = rwRASTERFORMAT32;
        }
        break;
    default: break;
    }
    return static_cast<RwInt32>( format );
}

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call,
                       LPVOID /*lpReserved*/ )
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        g_pIO_API     = {reinterpret_cast<RwStreamFindChunk_FN>( 0x5AA800 ),
                     reinterpret_cast<RwStreamRead_FN>( 0x5A3D90 )};
        g_pGlobal_API = {
            reinterpret_cast<RwSystemFunc>( 0x5B74A0 ),
            reinterpret_cast<uint32_t *>( 0x9415F8 ),
            reinterpret_cast<RwResourcesAllocateResEntry_FN>( 0x5C3430 )};
        g_pRaster_API     = {reinterpret_cast<RwRasterCreate_FN>( 0x5ADBF0 ),
                         reinterpret_cast<RwRasterDestroy_FN>( 0x5ADA40 )};
        g_pTexture_API    = {reinterpret_cast<RwTextureCreate_FN>( 0x5A7590 ),
                          reinterpret_cast<RwTextureSetName_FN>( 0x5A7670 ),
                          reinterpret_cast<RwTextureSetName_FN>( 0x5A76E0 )};
        g_pRwRenderEngine = std::unique_ptr<RwRenderEngine>(
            new RwRenderEngine( RenderingAPI::DX11 ) );
        {
            RwPointerTable gta3_ptr_table{};
            gta3_ptr_table.m_fpRenderSystem              = 0x618B54;
            gta3_ptr_table.m_fpIm3DOpen                  = 0x5D1D40;
            gta3_ptr_table.m_fpCheckNativeTextureSupport = 0x581180;
            gta3_ptr_table.m_fpSetRenderState            = 0x618B60;
            gta3_ptr_table.m_fpGetRenderState            = 0x618B64;
            gta3_ptr_table.m_fpIm2DRenderLine            = 0x618B68;
            gta3_ptr_table.m_fpIm2DRenderPrim            = 0x618B70;
            gta3_ptr_table.m_fpIm2DRenderIndexedPrim     = 0x618B74;
            //  pipeline ptr
            RwGameHooks::Patch( gta3_ptr_table );
        }
        {
            // RedirectJump( 0x405DB0, reinterpret_cast<void *>( debug_log ) );
            // RedirectJump( 0x59E720, reinterpret_cast<void *>( debug_log ) );
            RedirectCall( 0x5929FE, reinterpret_cast<void *>(
                                        rwD3D8FindCorrectRasterFormat ) );
            RedirectCall( 0x592A0E, reinterpret_cast<void *>(
                                        rwD3D8FindCorrectRasterFormat ) );
            RedirectCall( 0x592A1E, reinterpret_cast<void *>(
                                        rwD3D8FindCorrectRasterFormat ) );
            RedirectCall( 0x592A2E, reinterpret_cast<void *>(
                                        rwD3D8FindCorrectRasterFormat ) );

            // RedirectJump( 0x5BB9D0, reinterpret_cast<void *>( true_ret_hook )
            // );

            // RedirectJump( 0x429C00, reinterpret_cast<void *>( true_ret_hook )
            // ); RedirectCall( 0x48C20C, reinterpret_cast<void *>(
            // false_ret_hook ) );
            RedirectCall( 0x48E6C3, reinterpret_cast<void *>( render_rt ) );
            RedirectCall( 0x48E6B9, reinterpret_cast<void *>( RenderScene ) );

            DWORD value = 4;
            Patch( 0x582DBB, &value, sizeof( value ) );
            SetPointer( 0x61AD6C,
                        reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
            SetPointer( 0x61B274,
                        reinterpret_cast<void *>(
                            SkinD3D8AtomicAllInOneNode ) ); // skin
            SetPointer( 0x61AADC,
                        reinterpret_cast<void *>( rxD3D8SubmitNode ) );
        }
        /* {
byte value = 0x3;
Patch( 0x48E84D, &value, 1 );
}*/
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
