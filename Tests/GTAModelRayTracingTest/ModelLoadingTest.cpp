#include "ModelLoadingTest.h"
#include "bvh_builder.h"
#include "forward_pbr_pipeline.h"
#include "gta_map_loader.h"
#include "ray_tracer.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/types/constant_buffer_info.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/image_buffer_format.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/D3D11Impl/D3D11Renderer.h>
#include <common_headers.h>
#include <filesystem>
#include <gbuffer/gbuffer_pass.h>
#include <gbuffer/gbuffer_pipeline.h>
#include <ray_traced_gi/lf_gi_filter_pass.h>
#include <ray_traced_gi/per_pixel_gi_pass.h>
#include <ray_traced_shadows/rt_shadows_pass.h>
#include <ray_tracer.h>
#include <ray_tracing_scene_cache.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rp_clump/rp_clump.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_camera/rw_camera.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_tex_dict/rw_tex_dict.h>
#include <rw_engine/rw_texture/rw_texture.h>

static uint32_t max_rt_iterations = 320;
bool            ModelLoadingTest::CustomInitialize()
{
    if ( !rh::rw::engine::RwTestSample::CustomInitialize() )
        return false;
    // m_pPipeline = new ForwardPBRPipeline();
    // m_pGBPipe = new GBufferPipeline();

    GBufferDesc desc{};
    desc.width  = 1280;
    desc.height = 720;
    desc.bufferFormats.emplace_back(
        static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA32 ) );
    desc.bufferFormats.emplace_back(
        static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA16 ) );
    desc.bufferFormats.emplace_back(
        static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA8 ) );
    desc.bufferFormats.emplace_back(
        static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA8 ) );
    /*m_pGBPass = new GBufferPass( desc );
    bvh_builder = new rw_raytracing_lib::BVHBuilder();
    ray_tracer = new rw_raytracing_lib::RayTracer();
    rt_scene = new rw_raytracing_lib::RayTracingScene( bvh_builder );*/
    m_pMapLoader = new GTAMapLoader();
    /*m_pShadowsPass = new rw_raytracing_lib::RTShadowsPass( 1280, 720 );
    m_pGIPass = new rw_raytracing_lib::RTPerPixelGIPass( 1280, 720, 0.33f );
    m_pLFGIFilterPass = new rw_raytracing_lib::LowFreqGIFilterPass( 1280, 720,
    0.33f ); rh::engine::IGPUAllocator *allocator =
    rh::engine::g_pRHRenderer->GetGPUAllocator();

    rh::engine::ImageBufferInfo createInfo_{};
    createInfo_.type = rh::engine::ImageBufferType::RenderTargetBuffer;
    createInfo_.width = 1280;
    createInfo_.height = 720;
    createInfo_.mipLevels = 1;
    createInfo_.format = rh::engine::ImageBufferFormat::RGBA32;
    allocator->AllocateImageBuffer( createInfo_, IrradianceGIRT );

*/
    namespace fs = std::filesystem;
    for ( auto &p : fs::directory_iterator( "ides_3" ) )
    {
        fs::path file_path = p.path();
        if ( file_path.extension() == ".ide" ||
             file_path.extension() == ".IDE" )
        {
            m_pMapLoader->LoadIDE( file_path.string() );
        }
    }

    for ( auto &p : fs::directory_iterator( "ipls_3_fast" ) )
    {
        fs::path file_path = p.path();
        if ( file_path.extension() == ".ipl" ||
             file_path.extension() == ".IPL" )
        {
            auto clump_list = m_pMapLoader->LoadModels(
                "models_3", m_pMapLoader->LoadIPL( file_path.string(), 0 ) );
            m_vClumpList.insert( m_vClumpList.end(), clump_list.begin(),
                                 clump_list.end() );
        }
    }
    // load ipl required ids
    /*
        allocator->AllocateConstantBuffer( {sizeof(
       rh::rw::engine::CameraContext )}, mBaseConstantBuffer );
        allocator->AllocateConstantBuffer( {sizeof( DirectX::XMMATRIX ) * 2},
       mPerModelConstantBuffer );
    */
    PrepareDFFs();
    // rh::rw::engine::g_cameraContext->deltas.x = 0;

    // rh::rw::engine::g_cameraContext->deltas.z = 1;
    return true;
}

void ModelLoadingTest::RenderUI()
{
    // ImGui::ShowDemoWindow();
    /*ImGui::Begin( "Info" );
    ImGui::Text( "FrameRate:%f", m_fFrameRate );
    if ( m_fFrameRate > 0.0 )
        ImGui::Text( "FrameTime:%f", 1.0 / m_fFrameRate );
    ImGui::End();*/
}

void ModelLoadingTest::CustomShutdown() { TestSample::CustomShutdown(); }
void ModelLoadingTest::GenerateQuad( float w, float h )
{
    /* m_vQuad.clear();

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
     m_vQuad.push_back( vtx );*/
}

void ModelLoadingTest::CustomRender()
{
    RwRGBA clearColor = { 128, 128, 255, 255 };

    // rh::engine::IRenderingContext *context =
    //    static_cast<rh::engine::IRenderingContext *>(
    //        rh::engine::g_pRHRenderer->GetCurrentContext() );
    // context->UpdateBuffer( mBaseConstantBuffer,
    // rh::rw::engine::g_cameraContext,
    //                       sizeof( *rh::rw::engine::g_cameraContext ) );

    rh::rw::engine::RwCameraClear( m_pMainCamera, &clearColor,
                                   rwCAMERACLEARIMAGE | rwCAMERACLEARZ );

    /*context->BindConstantBuffers(
        rh::engine::ShaderStage::Vertex | rh::engine::ShaderStage::Pixel |
            rh::engine::ShaderStage::Compute,
        { { 0, mBaseConstantBuffer }, { 1, mPerModelConstantBuffer } } );

    m_pGBPass->PrepareFrame( context );
    m_pMapLoader->RenderScene( m_pGBPipe, mPerModelConstantBuffer );
    m_pGBPass->EndFrame( context );

    context->BindImageBuffers( rh::engine::ImageBindType::RenderTarget,
                               { { 0, rh::rw::engine::GetInternalRaster(
                                          m_pMainCamera->frameBuffer ) } } );
    m_pShadowsPass->Execute( ray_tracer, m_pGBPass->GetGBuffer( 0 ) );
    auto sh_y = m_pGIPass->Execute( ray_tracer, m_pGBPass->GetGBuffer( 0 ) );
    auto filtered_gi = m_pLFGIFilterPass->Execute( m_pGBPass->GetGBuffer( 0 ),
                                                   m_pGBPass->GetGBuffer( 1 ) );
    GenerateQuad( 1280.0f, 720.0f );
    rh::rw::engine::g_pRwRenderEngine->m_p2DRenderer->BindTexture(
        filtered_gi );
    rh::rw::engine::RwIm2DRenderPrimitive(
        RwPrimitiveType::rwPRIMTYPETRILIST, m_vQuad.data(),
        static_cast<int32_t>( m_vQuad.size() ) );
    rh::rw::engine::g_pRwRenderEngine->m_p2DRenderer->BindTexture( nullptr );
    rh::rw::engine::g_pRwRenderEngine->RenderStateSet(
        rwRENDERSTATETEXTURERASTER, 0 );
    rh::rw::engine::g_cameraContext->deltas.z =
        min( rh::rw::engine::g_cameraContext->deltas.z + 1,
             max( max_rt_iterations, 2 ) );*/
    RenderUI();
    // bvh_builder->DrawDebug();
}

void ModelLoadingTest::CustomUpdate( float dt )
{
    if ( dt >= 0.0f )
        m_fFrameRate = 1.0f / dt;
    // rh::rw::engine::g_cameraContext->deltas.x = ( rand() % 1000 ) / 1000.0f;
    // rh::rw::engine::g_cameraContext->deltas.y = ( rand() % 1000 ) / 1000.0f;
    // DIMOUSESTATE mouseCurrState{};
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
    {
        if ( !m_bMouseAquired )
        {
            // m_pMouse->Acquire();
            m_bMouseAquired = true;
        }
        // m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE ), &mouseCurrState );
    }
    else
    {
        if ( m_bMouseAquired )
        {
            // m_pMouse->Unacquire();
            m_bMouseAquired = false;
        }
    }

    RwV2d mousePosRw = { 0 * 0.005f, 0 * 0.005f };
    auto  ltm        = rh::rw::engine::RwFrameGetLTM( m_pMainCameraFrame );
    {
        DirectX::XMFLOAT4X4 currentRot = { ltm->right.x,
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
                                           0,
                                           0,
                                           0,
                                           1 };

        DirectX::XMMATRIX rightRotAxis = DirectX::XMMatrixRotationAxis(
            { { { ltm->right.x, ltm->right.y, ltm->right.z } } },
            ( mousePosRw.y ) );

        DirectX::XMMATRIX upRotAxis = DirectX::XMMatrixRotationAxis(
            { { { -ltm->up.x, -ltm->up.y, -ltm->up.z } } }, ( mousePosRw.x ) );

        DirectX::XMMATRIX current_rot_xm =
            DirectX::XMLoadFloat4x4( &currentRot );

        const DirectX::XMMATRIX rotMat =
            current_rot_xm * rightRotAxis * upRotAxis;
        /* ltm->right = { rotMat.r[0].vector4_f32[0],
         rotMat.r[0].vector4_f32[1], rotMat.r[0].vector4_f32[2] }; ltm->up    =
         { rotMat.r[1].vector4_f32[0], rotMat.r[1].vector4_f32[1],
                     rotMat.r[1].vector4_f32[2] };
         ltm->at    = { rotMat.r[2].vector4_f32[0], rotMat.r[2].vector4_f32[1],
                     rotMat.r[2].vector4_f32[2] };*/
    }
    RwV3d &camPos = ltm->pos;

    const float camSpeed = dt * 150;
    if ( GetKeyState( 0x57 ) & 0x8000 )
    {
        camPos.x += ltm->at.x * camSpeed;
        camPos.y += ltm->at.y * camSpeed;
        camPos.z += ltm->at.z * camSpeed;
    }
    if ( GetKeyState( 0x53 ) & 0x8000 )
    {
        camPos.x -= ltm->at.x * camSpeed;
        camPos.y -= ltm->at.y * camSpeed;
        camPos.z -= ltm->at.z * camSpeed;
    }
    if ( GetKeyState( 0x41 ) & 0x8000 )
    {
        camPos.x += ltm->right.x * camSpeed;
        camPos.y += ltm->right.y * camSpeed;
        camPos.z += ltm->right.z * camSpeed;
    }
    if ( GetKeyState( 0x44 ) & 0x8000 )
    {
        camPos.x -= ltm->right.x * camSpeed;
        camPos.y -= ltm->right.y * camSpeed;
        camPos.z -= ltm->right.z * camSpeed;
    }
    if ( GetKeyState( 0x45 ) & 0x8000 )
    {
        camPos.x += ltm->up.x * camSpeed;
        camPos.y += ltm->up.y * camSpeed;
        camPos.z += ltm->up.z * camSpeed;
    }
    if ( GetKeyState( 0x51 ) & 0x8000 )
    {
        camPos.x -= ltm->up.x * camSpeed;
        camPos.y -= ltm->up.y * camSpeed;
        camPos.z -= ltm->up.z * camSpeed;
    }
}

void ModelLoadingTest::LoadDFF( const rh::engine::String &path )
{
    RpClump *result = nullptr;
    if ( !rh::rw::engine::LoadClump( result, path ) )
        rh::debug::DebugLogger::Error( "Failed to load clump from dff!" );
    else if ( result != nullptr )
        m_vClumpList.push_back( result );
}

DirectX::XMMATRIX GetWorldMat( float x )
{
    return DirectX::XMMatrixIdentity() *
           DirectX::XMMatrixTranslation( 0, x, 0 );
}

void ModelLoadingTest::PrepareDFFs()
{
    // m_pMapLoader->BuildScene( rt_scene, ray_tracer,
    //                          rh::rw::engine::g_cameraContext->viewPos );
}
