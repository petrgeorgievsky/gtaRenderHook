#include "renderloop.h"
#include "gta_sa_internal_classes/renderer.h"
#include <render_client/render_client.h>
#include <render_loop.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>

static rh::rw::engine::RpGeometryRw36 geometry_interface_35{};

void RenderLoop::Render()
{
    RenderOpaque();
    RenderTransparent();
    RenderPostProcessing();
}

void RenderLoop::Init()
{
    /*
    rh::rw::engine::RwRenderEngine::RegisterPostInitCallback( []() {
        unsigned int current_display_mode;
        rh::engine::DisplayModeInfo mode_info;
        auto &device_state = rh::engine::g_pRHRenderer->GetDeviceState();
        if ( !device_state.GetCurrentDisplayMode( current_display_mode ) ) {
            rh::debug::DebugLogger::Error( "Failed to retrieve current display
    mode!" ); return;
        }
        if ( !device_state.GetDisplayModeInfo( current_display_mode, mode_info )
    ) { rh::debug::DebugLogger::Error( "Failed to retrieve current display mode
    info!" ); return;
        }
        g_pGBufferPipeline = new GBufferPipeline();

        GBufferDesc desc;
        desc.width = mode_info.width;
        desc.height = mode_info.height;
        desc.bufferFormats.emplace_back(
            static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA32 ) );
        desc.bufferFormats.emplace_back(
            static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA16 ) );
        desc.bufferFormats.emplace_back(
            static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA8 ) );
        desc.bufferFormats.emplace_back(
            static_cast<uint32_t>( rh::engine::ImageBufferFormat::RGBA8 ) );

        g_pGBufferPass = new GBufferPass( desc );

        auto allocator = rh::engine::g_pRHRenderer->GetGPUAllocator();
        rh::engine::ConstantBufferInfo info{};
        info.size = sizeof( *rh::rw::engine::g_cameraContext );

        allocator->AllocateConstantBuffer( info, gBaseConstantBuffer );

        info.size = 128;

        allocator->AllocateConstantBuffer( info, gPerModelConstantBuffer );
    } );

    rh::rw::engine::RwRenderEngine::RegisterOutputResizeCallback( []( uint32_t
    w, uint32_t h ) { if ( g_pGBufferPass ) g_pGBufferPass->UpdateSize( w, h );
    } );
    rh::rw::engine::RwRenderEngine::RegisterPreShutdownCallback( []() {
        delete gBaseConstantBuffer;
        delete gPerModelConstantBuffer;
        delete g_pGBufferPipeline;
        delete g_pGBufferPass;
    } );
    */
}

#define RwFrameGetLTM( frame )                                                 \
    ( reinterpret_cast<RwMatrix *(__cdecl *)( RwFrame * )>( 0x7F0990 ) )(      \
        frame )

void RenderLoop::RenderOpaque()
{
    for ( uint32_t i = 0; i < CRenderer::ms_nNoOfVisibleEntities; i++ )
    {
        void *    entity_ptr = CRenderer::ms_aVisibleEntityPtrs[i];
        RwObject *rw_obj     = *reinterpret_cast<RwObject **>(
            reinterpret_cast<INT_PTR>( entity_ptr ) + 0x18 );
        if ( rw_obj->type == 1 )
        {
            auto atomic = reinterpret_cast<RpAtomic *>( rw_obj );
            geometry_interface_35.Init( atomic->geometry );
            if ( rh::rw::engine::RwRHInstanceAtomic( atomic,
                                                     &geometry_interface_35 ) !=
                 rh::rw::engine::RenderStatus::Instanced )
                continue;

            const RwMatrix *ltm = RwFrameGetLTM( static_cast<RwFrame *>(
                rh::rw::engine::rwObject::GetParent( atomic ) ) );

            rh::rw::engine::DrawAtomic(
                atomic, &geometry_interface_35,
                [&ltm, &entity_ptr]( rh::rw::engine::ResEnty *res_entry ) {
                    auto &renderer = rh::rw::engine::gRenderClient->RenderState
                                         .MeshDrawCalls;
                    rh::rw::engine::DrawCallInfo info{};
                    info.mDrawCallId = reinterpret_cast<uint64_t>( entity_ptr );
                    info.mMeshId     = res_entry->meshData;
                    info.mWorldTransform = DirectX::XMFLOAT4X3{
                        ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                        ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                        ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
                    };
                    renderer.RecordDrawCall( info );
                } );
        }
    }
    /*
    rh::engine::IRenderingContext *context =
        static_cast<rh::engine::IRenderingContext *>(
            rh::engine::g_pRHRenderer->GetCurrentContext() );

    g_pGBufferPass->PrepareFrame( context );

    context->UpdateBuffer( gBaseConstantBuffer, rh::rw::engine::g_cameraContext,
                           sizeof( *rh::rw::engine::g_cameraContext ) );

    context->BindConstantBuffers(
        rh::engine::ShaderStage::Vertex | rh::engine::ShaderStage::Pixel |
            rh::engine::ShaderStage::Compute,
        { { 0, gBaseConstantBuffer }, { 1, gPerModelConstantBuffer } } );

    for ( uint32_t i = 0; i < CRenderer::ms_nNoOfVisibleEntities; i++ )
    {
        void *    entity_ptr = CRenderer::ms_aVisibleEntityPtrs[i];
        RwObject *rw_obj     = *reinterpret_cast<RwObject **>(
            reinterpret_cast<INT_PTR>( entity_ptr ) + 0x18 );
        if ( rw_obj->type == 1 )
        {
            auto atomic = reinterpret_cast<RpAtomic *>( rw_obj );
            geometry_interface_35.Init( atomic->geometry );
            if ( rh::rw::engine::RwRHInstanceAtomic( atomic,
                                                     &geometry_interface_35 ) !=
                 rh::rw::engine::RenderStatus::Instanced )
                continue;

            const RwMatrix *  ltm = RwFrameGetLTM( static_cast<RwFrame *>(
                rh::rw::engine::rwObject::GetParent( atomic ) ) );
            DirectX::XMMATRIX objTransformMatrix = {
                ltm->right.x, ltm->right.y, ltm->right.z, 0,
                ltm->up.x,    ltm->up.y,    ltm->up.z,    0,
                ltm->at.x,    ltm->at.y,    ltm->at.z,    0,
                ltm->pos.x,   ltm->pos.y,   ltm->pos.z,   1 };
            struct PerModelCB
            {
                DirectX::XMMATRIX worldMatrix;
                DirectX::XMMATRIX worldITMatrix;
            } perModelCB;
            perModelCB.worldMatrix = objTransformMatrix;
            perModelCB.worldITMatrix =
                DirectX::XMMatrixInverse( nullptr, objTransformMatrix );
            context->UpdateBuffer( gPerModelConstantBuffer, &perModelCB,
                                   sizeof( perModelCB ) );
            rh::rw::engine::DrawAtomic( atomic, &geometry_interface_35, context,
                                        g_pGBufferPipeline );
        }
    }

    g_pGBufferPass->EndFrame( context );
    void *frameBufferInternal =
        rh::rw::engine::GetInternalRaster( Scene.m_pRwCamera->frameBuffer );
    rh::engine::g_pRHRenderer->BindImageBuffers(
        rh::engine::ImageBindType::RenderTarget,
        { { 0, frameBufferInternal } } );*/
}

void RenderLoop::RenderTransparent() {}

void RenderLoop::RenderPostProcessing() {}
