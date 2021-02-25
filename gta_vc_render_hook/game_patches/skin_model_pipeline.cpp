//
// Created by peter on 07.01.2021.
//

#include "skin_model_pipeline.h"
#include <injection_utils/InjectorHelpers.h>
#include <render_client/render_client.h>
#include <rw_engine/anim_hierarcy_rw36.h>
#include <rw_engine/i_anim_hierarcy.h>
#include <rw_engine/rp_geometry_rw36.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_skin_pipeline.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>
#include <rw_game_hooks.h>

using namespace rh::rw::engine;

static int32_t SkinD3D8AtomicAllInOneNode( void * /*self*/,
                                           const RxPipelineNodeParam *params )
{
    static RpGeometryRw36 geometry_interface_35{};
    auto *                atomic = static_cast<RpAtomic *>( params->dataParam );
    RpGeometry *          geom   = atomic->geometry;

    geometry_interface_35.Init( geom );

    if ( InstanceSkinAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;

    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic( atomic, &geometry_interface_35,
                [&ltm, atomic]( rh::rw::engine::ResEnty *res_entry ) {
                    auto &renderer =
                        gRenderClient->RenderState.SkinMeshDrawCalls;
                    static AnimHierarcyRw36 g_anim{};

                    auto mesh_list = geometry_interface_35.GetMeshList();
                    auto materials =
                        renderer.AllocateDrawCallMaterials( mesh_list.size() );
                    for ( auto i = 0; i < mesh_list.size(); i++ )
                        materials[i] =
                            ConvertMaterialData( mesh_list[i].material );

                    SkinDrawCallInfo info{};
                    info.DrawCallId     = reinterpret_cast<uint64_t>( atomic );
                    info.MeshId         = res_entry->meshData;
                    info.WorldTransform = DirectX::XMFLOAT4X3{
                        ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                        ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                        ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
                    };

                    PrepareBoneMatrices( info.BoneTransform, atomic, g_anim );

                    renderer.RecordDrawCall( info );
                } );

    return 1;
}

void SkinModelPipeline::Patch()
{
    SetPointer( 0x6DF8EC,
                reinterpret_cast<void *>( SkinD3D8AtomicAllInOneNode ) );
}
