//
// Created by peter on 26.10.2020.
//

#include "base_model_pipeline.h"
#include "../call_redirection_util.h"
#include "../gta3_geometry_proxy.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <render_loop.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>

using namespace rh::rw::engine;
static RpGeometryRw35 geometry_interface_35{};

static int32_t D3D8AtomicAllInOneNode( void * /*self*/,
                                       const RxPipelineNodeParam *params )
{
    auto *atomic = static_cast<RpAtomic *>( params->dataParam );
    auto *geom   = reinterpret_cast<RpGeometryGTA3 *>( atomic->geometry );
    geometry_interface_35.Init( geom );

    if ( RwRHInstanceAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;

    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic(
        atomic, &geometry_interface_35, [&ltm, atomic]( ResEnty *res_entry ) {
            auto &renderer = EngineClient::gRendererGlobals;

            auto mesh_list = geometry_interface_35.GetMeshList();
            auto materials =
                renderer.AllocateDrawCallMaterials( mesh_list.size() );
            for ( auto i = 0; i < mesh_list.size(); i++ )
                materials[i] = ConvertMaterialData( mesh_list[i].material );

            DrawCallInfo info{};
            info.mDrawCallId     = reinterpret_cast<uint64_t>( atomic );
            info.mMeshId         = res_entry->meshData;
            info.mWorldTransform = DirectX::XMFLOAT4X3{
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
    SetPointer( GetAddressByGame( 0x61B6A4, 0x61AD6C, 0x628280 ),
                reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
}
