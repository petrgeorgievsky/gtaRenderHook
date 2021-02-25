//
// Created by peter on 26.10.2020.
//

#include "base_model_pipeline.h"
#include <injection_utils/InjectorHelpers.h>
#include <render_client/render_client.h>
#include <rw_engine/rp_geometry_rw36.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_game_hooks.h>

using namespace rh::rw::engine;
static RpGeometryRw36 geometry_interface{};

static int32_t D3D8AtomicAllInOneNode( void * /*self*/,
                                       const RxPipelineNodeParam *params )
{
    RpAtomic *atomic;

    atomic           = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *geom = atomic->geometry;
    geometry_interface.Init( geom );
    if ( InstanceAtomic( atomic, &geometry_interface ) !=
         RenderStatus::Instanced )
        return 0;
    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic(
        atomic, &geometry_interface, [&ltm, atomic]( ResEnty *res_entry ) {
            auto &renderer  = gRenderClient->RenderState.MeshDrawCalls;
            auto  mesh_list = geometry_interface.GetMeshList();
            auto  materials =
                renderer.AllocateDrawCallMaterials( mesh_list.size() );
            for ( auto i = 0; i < mesh_list.size(); i++ )
                materials[i] = ConvertMaterialData( mesh_list[i].material );

            DrawCallInfo info{};
            info.DrawCallId     = reinterpret_cast<uint64_t>( atomic );
            info.MeshId         = res_entry->meshData;
            info.WorldTransform = DirectX::XMFLOAT4X3{
                ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
            };
            renderer.RecordDrawCall( info );
        } );
    return 1;
}

void BaseModelPipeline::Patch()
{
    SetPointer( 0x6DF9AC, reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
}
