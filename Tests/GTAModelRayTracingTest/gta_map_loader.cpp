#include "gta_map_loader.h"
#include "ray_tracer.h"
#include "ray_tracing_scene_cache.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/IRenderer.h>
#include <fstream>
#include <rw_engine/rp_clump/rp_clump.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>

static rh::rw::engine::RpGeometryRw36 geometry_interface_36{};
void GTAMapLoader::LoadIDE( const std::string &path )
{
    std::ifstream ide_fstream( path );
    char          line[256];
    do
    {
        memset( line, 0, 256 );
        ide_fstream.getline( line, 256, '\n' );
    } while ( memcmp( line, "objs", 4 ) != 0 );

    memset( line, 0, 256 );
    ide_fstream.getline( line, 256, '\n' );
    while ( memcmp( line, "end", 3 ) != 0 )
    {
        uint32_t model_id, obj_count;
        char     dff_name[64];
        char     txd_name[64];
        memset( dff_name, 0, 64 );
        memset( txd_name, 0, 64 );
        float drawDist;

        for ( int i = 0; i < 256; i++ )
        {
            if ( line[i] < ' ' || line[i] == ',' )
                line[i] = ' ';
        }
        sscanf_s( line, "%u %s %s %u %f", &model_id, dff_name, txd_name,
                  &obj_count, &drawDist );

        if ( mIDEntries.find( model_id ) == mIDEntries.end() &&
             memcmp( dff_name, "LOD", 3 ) != 0 &&
             memcmp( dff_name, "IslandLOD", 9 ) != 0 )
        {
            // mIDEntries[model_id].object = nullptr;
            mIDEntries[model_id].draw_dist = drawDist;
            mIDEntries[model_id].dff_name  = std::string( dff_name );
            // mIDEntries[model_id].txdname = std::string( txd_name );
        }
        memset( line, 0, 256 );
        ide_fstream.getline( line, 256, '\n' );
    }
    ide_fstream.close();
}

std::vector<uint32_t> GTAMapLoader::LoadIPL( const std::string &path,
                                             uint8_t            version )
{
    std::vector<uint32_t> ipl_list;
    std::ifstream         ipl_fstream( path );
    char                  line[256];
    while ( memcmp( line, "inst", 4 ) != 0 )
    {
        memset( line, 0, 256 );
        ipl_fstream.getline( line, 256, '\n' );
    }
    memset( line, 0, 256 );
    ipl_fstream.getline( line, 256, '\n' );
    while ( memcmp( line, "end", 3 ) != 0 )
    {
        uint32_t model_id = 0, interior, lod_id;
        char     dff_name[64];
        RwV3d    pos, scale{ 1, 1, 1 };
        RwV4d    quatRot;
        for ( int i = 0; i < 256; i++ )
        {
            if ( line[i] < ' ' || line[i] == ',' )
                line[i] = ' ';
        }
        if ( version == 0 )
        {
            sscanf_s( line, "%u %s %f %f %f %f %f %f %f %f %f %f", &model_id,
                      dff_name, &pos.x, &pos.y, &pos.z, &scale.x, &scale.y,
                      &scale.z, &quatRot.x, &quatRot.y, &quatRot.z,
                      &quatRot.w );
        }
        else if ( version == 1 )
        {
            sscanf_s( line, "%u %s %u %f %f %f %f %f %f %f %f %f %f", &model_id,
                      dff_name, &interior, &pos.x, &pos.y, &pos.z, &scale.x,
                      &scale.y, &scale.z, &quatRot.x, &quatRot.y, &quatRot.z,
                      &quatRot.w );
        }
        else if ( version >= 2 )
        {
            sscanf_s( line, "%u %s %u %f %f %f %f %f %f %f %u", &model_id,
                      dff_name, &interior, &pos.x, &pos.y, &pos.z, &quatRot.x,
                      &quatRot.y, &quatRot.z, &quatRot.w, &lod_id );
        }
        DirectX::XMMATRIX transMat =
            DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
        DirectX::XMMATRIX scaleMat =
            DirectX::XMMatrixScaling( scale.x, scale.y, scale.z );
        DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationQuaternion(
            { quatRot.x, quatRot.y, -quatRot.z, quatRot.w } );
        DirectX::XMFLOAT4X4 transform;
        DirectX::XMStoreFloat4x4( &transform, rotMat * scaleMat * transMat );
        ItemPlacementEntry info = { transform, model_id, 1000.0f, { 0, 0 } };

        if ( mIDEntries.find( model_id ) != mIDEntries.end() )
        {
            mIPLEntries.push_back( info );
            if ( std::find( ipl_list.begin(), ipl_list.end(), model_id ) ==
                 ipl_list.end() )
                ipl_list.push_back( model_id );
        }
        memset( line, 0, 256 );
        ipl_fstream.getline( line, 256, '\n' );
    }
    ipl_fstream.close();
    return ipl_list;
}

std::vector<uint32_t> GTAMapLoader::LoadBinaryIPL( const std::string &path )
{
    std::vector<uint32_t> ipl_list;
    std::ifstream         ipl_fstream( path, std::ios_base::binary );
    char                  identifier[4];
    ipl_fstream.read( identifier, 4 );

    if ( memcmp( identifier, "bnry", 4 ) != 0 )
    {
        rh::debug::DebugLogger::Log( "incorrect binary ipl" );
        return {};
    }

    struct bnry_ipl_header
    {
        int32_t item_inst_count, unk1_count, unk2_count, unk3_count, car_count,
            unk4_count;
        int32_t item_inst_offset;
    } header;

    ipl_fstream.read( reinterpret_cast<char *>( &header ), sizeof( header ) );
    ipl_fstream.seekg( header.item_inst_offset );
    for ( int i = 0; i < header.item_inst_count; i++ )
    {
        uint32_t model_id;
        int32_t  interior, lod_id;
        RwV3d    pos;
        RwV4d    quatRot;
        ipl_fstream.read( reinterpret_cast<char *>( &pos ), sizeof( pos ) );
        ipl_fstream.read( reinterpret_cast<char *>( &quatRot ),
                          sizeof( quatRot ) );
        ipl_fstream.read( reinterpret_cast<char *>( &model_id ),
                          sizeof( model_id ) );
        ipl_fstream.read( reinterpret_cast<char *>( &interior ),
                          sizeof( interior ) );
        ipl_fstream.read( reinterpret_cast<char *>( &lod_id ),
                          sizeof( lod_id ) );

        DirectX::XMMATRIX transMat =
            DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
        DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationQuaternion(
            { quatRot.x, quatRot.y, -quatRot.z, quatRot.w } );
        DirectX::XMFLOAT4X4 transform;
        DirectX::XMStoreFloat4x4( &transform, rotMat * transMat );
        ItemPlacementEntry info = { transform, model_id, 1000.0f, { 0, 0 } };

        if ( mIDEntries.find( model_id ) != mIDEntries.end() )
        {
            mIPLEntries.push_back( info );
            if ( std::find( ipl_list.begin(), ipl_list.end(), model_id ) ==
                 ipl_list.end() )
                ipl_list.push_back( model_id );
        }
    }
    ipl_fstream.close();
    return ipl_list;
}

std::vector<RpClump *> GTAMapLoader::LoadModels( const std::string &   path,
                                                 std::vector<uint32_t> ids )
{
    std::vector<RpClump *> clumpList;
    for ( size_t i = 0; i < ids.size(); i++ )
    {
        std::filesystem::path fp = path;
        fp /= mIDEntries[ids[i]].dff_name + ".dff";
        if ( !std::filesystem::exists( fp ) )
            continue;
        if ( !rh::rw::engine::LoadClump( mIDEntries[ids[i]].dff_model,
                                         fp.generic_string() ) )
            rh::debug::DebugLogger::Error( "Failed to load clump from dff!" );
        mIDEntries[ids[i]].blas_inst_id = i;
        clumpList.push_back( mIDEntries[ids[i]].dff_model );
    }
    return clumpList;
}

void GTAMapLoader::BuildScene( rw_raytracing_lib::RayTracingScene *rt_scene,
                               rw_raytracing_lib::RayTracer *      ray_tracer,
                               const DirectX::XMFLOAT4 &           camPos )
{
    std::vector<rw_raytracing_lib::BLAS_Instance> blas_inst_list;
    //    for ( auto clump : models ) {
    //        RwLLLink *cur, *end;
    //        cur = rw_rh_engine::rwLinkList::GetFirstLLLink( &clump->atomicList
    //        ); end = rw_rh_engine::rwLinkList::GetTerminator(
    //        &clump->atomicList
    //        );

    //        if ( cur == end )
    //            continue;

    //        RpAtomic *atomic = rw_rh_engine::rwLLLink::GetData<RpAtomic>( cur,
    //                                                                      offsetof(
    //                                                                      RpAtomic,
    //                                                                                inClumpLink ) );
    //        if ( atomic ) {
    //            rw_rh_engine::RenderStatus status
    //                = rw_rh_engine::RwRHInstanceAtomic( atomic,
    //                &geometry_interface_36 );
    //            switch ( status ) {
    //            case rw_rh_engine::RenderStatus::Failure:
    //                RHDebug::DebugLogger::Log( "Failed instance" );
    //                break;
    //            case rw_rh_engine::RenderStatus::Instanced:
    //                RHDebug::DebugLogger::Log( "Successful instance" );
    //                break;
    //            case rw_rh_engine::RenderStatus::NotInstanced:
    //                RHDebug::DebugLogger::Log( "Unsuccessful instance" );
    //                break;
    //            }
    //        }
    //    }

    uint32_t id = 0;

    for ( auto &ipl_entry : mIPLEntries )
    {
        DirectX::XMVECTOR cam_pos     = DirectX::XMLoadFloat4( &camPos );
        DirectX::XMVECTOR obj_pos     = { ipl_entry.world_transform.m[3][0],
                                      ipl_entry.world_transform.m[3][1],
                                      ipl_entry.world_transform.m[3][2],
                                      ipl_entry.world_transform.m[3][3] };
        float             dist_to_cam = 1000;
        DirectX::XMStoreFloat(
            &dist_to_cam, DirectX::XMVector3Length(
                              DirectX::XMVectorSubtract( cam_pos, obj_pos ) ) );
        ipl_entry.current_dist_to_cam = dist_to_cam;
    }

    for ( auto ipl_entry : mIPLEntries )
    {
        if ( mIDEntries[ipl_entry.item_defenition_id].dff_model != nullptr )
        {
            id++;
            rw_raytracing_lib::BLAS_Instance blas;
            auto clump   = mIDEntries[ipl_entry.item_defenition_id].dff_model;
            blas.blas_id = ipl_entry.item_defenition_id;

            RwLLLink *cur, *end;
            cur = rh::rw::engine::rwLinkList::GetFirstLLLink(
                &clump->atomicList );
            end =
                rh::rw::engine::rwLinkList::GetTerminator( &clump->atomicList );
            if ( cur == end )
                continue;

            RpAtomic *atomic = rh::rw::engine::rwLLLink::GetData<RpAtomic>(
                cur, offsetof( RpAtomic, inClumpLink ) );
            rh::rw::engine::RwRHInstanceAtomic( atomic,
                                                &geometry_interface_36 );

            const RwMatrix *ltm =
                rh::rw::engine::RwFrameGetLTM( static_cast<RwFrame *>(
                    rh::rw::engine::rwObject::GetParent( atomic ) ) );
            geometry_interface_36.Init( atomic->geometry );
            // rt_scene->PushModel( ipl_entry.item_defenition_id,
            //                     &geometry_interface_36 );

            DirectX::XMMATRIX objTransformMatrix = {
                ltm->right.x, ltm->right.y, ltm->right.z, 0,
                ltm->up.x,    ltm->up.y,    ltm->up.z,    0,
                ltm->at.x,    ltm->at.y,    ltm->at.z,    0,
                ltm->pos.x,   ltm->pos.y,   ltm->pos.z,   1 };
            DirectX::XMMATRIX ws =
                DirectX::XMLoadFloat4x4( &ipl_entry.world_transform );

            objTransformMatrix = objTransformMatrix * ws;
            DirectX::XMStoreFloat4x4( &blas.world_transform,
                                      objTransformMatrix );
            blas_inst_list.push_back( blas );
        }
    }

    /* if ( !blas_inst_list.empty() )
     {
         auto pack_res = rt_scene->PackBLAS();
         if ( pack_res.second )
             ray_tracer->AllocatePackedBLAS( pack_res.first );
         rw_raytracing_lib::TLAS_BVH tlas =
             rt_scene->GenerateTLAS( blas_inst_list );
         if ( !tlas.tlas.empty() )
             ray_tracer->AllocateTLAS( tlas );
     }*/
}

void GTAMapLoader::RenderScene( void *pipeline, void *per_obj_cb )
{
    /*    rh::engine::IRenderingContext *context =
            static_cast<rh::engine::IRenderingContext *>(
                rh::engine::g_pRHRenderer->GetCurrentContext() );*/
    for ( auto ipl_entry : mIPLEntries )
    {
        if ( mIDEntries[ipl_entry.item_defenition_id].dff_model != nullptr )
        {
            auto clump = mIDEntries[ipl_entry.item_defenition_id].dff_model;

            RwLLLink *cur, *end;
            cur = rh::rw::engine::rwLinkList::GetFirstLLLink(
                &clump->atomicList );
            end =
                rh::rw::engine::rwLinkList::GetTerminator( &clump->atomicList );
            if ( cur == end )
                continue;

            RpAtomic *atomic = rh::rw::engine::rwLLLink::GetData<RpAtomic>(
                cur, offsetof( RpAtomic, inClumpLink ) );

            struct
            {
                DirectX::XMFLOAT4X4 world;
                DirectX::XMFLOAT4X4 world_inv;
            } transf_buf{};

            const RwMatrix *ltm =
                rh::rw::engine::RwFrameGetLTM( static_cast<RwFrame *>(
                    rh::rw::engine::rwObject::GetParent( atomic ) ) );
            DirectX::XMMATRIX objTransformMatrix = {
                ltm->right.x, ltm->right.y, ltm->right.z, 0,
                ltm->up.x,    ltm->up.y,    ltm->up.z,    0,
                ltm->at.x,    ltm->at.y,    ltm->at.z,    0,
                ltm->pos.x,   ltm->pos.y,   ltm->pos.z,   1 };
            DirectX::XMMATRIX ws =
                DirectX::XMLoadFloat4x4( &ipl_entry.world_transform );

            objTransformMatrix = objTransformMatrix * ws;

            DirectX::XMStoreFloat4x4( &transf_buf.world, objTransformMatrix );
            DirectX::XMStoreFloat4x4(
                &transf_buf.world_inv,
                DirectX::XMMatrixInverse( nullptr, objTransformMatrix ) );

            //  context->UpdateBuffer( per_obj_cb, &transf_buf,
            //                        sizeof( transf_buf ) );

            rh::rw::engine::DrawAtomic( atomic, &geometry_interface_36,
                                        []( rh::rw::engine::ResEnty * ) {} );
        }
    }
}
