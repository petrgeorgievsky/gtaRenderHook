#include "gta3_default_rendering_pipeline.h"
#include "gta3_geometry_proxy.h"
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <RwRenderEngine.h>
#include <common_headers.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
using namespace rh::engine;
using namespace rh::rw::engine;

int32_t RwDefaultRenderingPipeline::AtomicAllInOneNode(
    RxPipelineNodeInstance * /*self*/, const RxPipelineNodeParam *params )
{
    RpAtomic *atomic;
    atomic = static_cast<RpAtomic *>( params->dataParam );

    // auto privateData = (_rxD3D9InstanceNodeData *)self->privateData;
    // if ( RwRHInstanceAtomic( atomic, &g_rw35GeometryProxy ) !=
    // RenderStatus::Instanced )
    return false;

    // IRenderingContext *context = static_cast<IRenderingContext *>(
    //    g_pRHRenderer->GetCurrentContext() );

    const RwMatrix *ltm = rh::rw::engine::RwFrameGetLTM(
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

    // context->UpdateBuffer( gBaseConstantBuffer, g_cameraContext, sizeof(
    // *g_cameraContext ) ); context->UpdateBuffer( gPerModelConstantBuffer,
    // &perModelCB, sizeof( perModelCB ) ); context->BindConstantBuffers(
    // RHEngine::RHShaderStage::Vertex | RHEngine::RHShaderStage::Pixel,
    //{{0, gBaseConstantBuffer}, {1, gPerModelConstantBuffer}} );
    // DrawAtomic( atomic, &g_rw35GeometryProxy,
    // context, g_pForwardPBRPipeline );

    return ( TRUE );
}
