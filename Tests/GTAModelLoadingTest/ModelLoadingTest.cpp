#include "ModelLoadingTest.h"
#include "forward_pbr_pipeline.h"
#include <DebugUtils/DebugLogger.h>
#include <common_headers.h>
#include <dinput.h>
#include <render_client/render_client.h>
#include <render_loop.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rp_clump/rp_clump.h>
#include <rw_engine/rw_camera/rw_camera.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_tex_dict/rw_tex_dict.h>
#include <rw_engine/rw_texture/rw_texture.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

using namespace rh;

static rw::engine::RpGeometryRw36 geometry_interface_36{};
bool                              ModelLoadingTest::CustomInitialize()
{
    if ( !rw::engine::RwTestSample::CustomInitialize() )
        return false;
    /*rw::engine::EngineState::gRendererServerGlobals->RegisterPipeline(
        new ForwardPBRPipeline(), 0 );*/
    m_sDffPath   = "models/test.dff";
    namespace fs = std::filesystem;
    std::error_code ec;
    char            dir_path[4096];
    GetModuleFileNameA( nullptr, dir_path, 4096 );
    fs::path dir_path_ = fs::path( dir_path ).parent_path() / m_sDffPath;

    LoadDFF( dir_path_.generic_string() );

    return true;
}

void ModelLoadingTest::CustomShutdown()
{
    rh::rw::engine::RwTestSample::CustomShutdown();
}

void ModelLoadingTest::CustomRender()
{
    using namespace rw::engine;
    RwRGBA clearColor = { 128, 128, 255, 255 };

    RwCameraClear( m_pMainCamera, &clearColor,
                   rwCAMERACLEARIMAGE | rwCAMERACLEARZ );
    for ( auto clump : m_vClumpList )
    {
        for ( auto *atomic :
              rwLinkList::Iterator<RpAtomic>( clump->atomicList ) )
        {
            if ( !atomic )
                continue;
            auto ltm = RwFrameGetLTM(
                static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
            DrawAtomic(
                atomic, &geometry_interface_36,
                [&ltm, atomic]( ResEnty *res_entry ) {
                    auto &       renderer = EngineClient::gRendererGlobals;
                    DrawCallInfo info{};
                    info.mDrawCallId     = reinterpret_cast<uint64_t>( atomic );
                    info.mMeshId         = res_entry->meshData;
                    info.mWorldTransform = DirectX::XMFLOAT4X3{
                        ltm->at.x, ltm->right.x, ltm->up.x, ltm->pos.x,
                        ltm->at.y, ltm->right.y, ltm->up.y, ltm->pos.y,
                        ltm->at.z, ltm->right.z, ltm->up.z, ltm->pos.z,
                    };
                    auto mesh_list = geometry_interface_36.GetMeshList();
                    auto materials =
                        renderer.AllocateDrawCallMaterials( mesh_list.size() );
                    for ( auto i = 0; i < mesh_list.size(); i++ )
                        materials[i] =
                            ConvertMaterialData( mesh_list[i].material );
                    renderer.RecordDrawCall( info );
                } );
        }
    }
}

void ModelLoadingTest::CustomUpdate( float dt )
{
    DIMOUSESTATE mouseCurrState{};
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
    {
        if ( !m_bMouseAquired )
        {
            // static_cast<LPDIRECTINPUTDEVICE>( m_pMouse )->Acquire();
            m_bMouseAquired = true;
        }
    }
    else
    {
        if ( m_bMouseAquired )
        {
            // static_cast<LPDIRECTINPUTDEVICE>( m_pMouse )->Unacquire();
            m_bMouseAquired = false;
        }
    }

    RwV2d mousePosRw = { mouseCurrState.lX * 0.005f,
                         mouseCurrState.lY * 0.005f };

    DirectX::XMFLOAT4X4 current_rot{ m_pMainCameraFrame->ltm.right.x,
                                     m_pMainCameraFrame->ltm.right.y,
                                     m_pMainCameraFrame->ltm.right.z,
                                     0,
                                     m_pMainCameraFrame->ltm.up.x,
                                     m_pMainCameraFrame->ltm.up.y,
                                     m_pMainCameraFrame->ltm.up.z,
                                     0,
                                     m_pMainCameraFrame->ltm.at.x,
                                     m_pMainCameraFrame->ltm.at.y,
                                     m_pMainCameraFrame->ltm.at.z,
                                     0,
                                     0,
                                     0,
                                     0,
                                     1 };

    DirectX::XMMATRIX currentRot = DirectX::XMLoadFloat4x4( &current_rot );
    auto              cam_right =
        DirectX::XMLoadFloat4( reinterpret_cast<DirectX::XMFLOAT4 *>(
            &m_pMainCameraFrame->ltm.right ) );
    auto cam_up = DirectX::XMLoadFloat4(
        reinterpret_cast<DirectX::XMFLOAT4 *>( &m_pMainCameraFrame->ltm.up ) );

    DirectX::XMMATRIX rightRotAxis =
        DirectX::XMMatrixRotationAxis( cam_right, ( mousePosRw.y ) );

    DirectX::XMMATRIX upRotAxis =
        DirectX::XMMatrixRotationAxis( cam_up, -( mousePosRw.x ) );

    const DirectX::XMMATRIX rotMat = currentRot * rightRotAxis * upRotAxis;
    DirectX::XMFLOAT4X4     rot_matrix{};
    DirectX::XMStoreFloat4x4( &rot_matrix, rotMat );
    m_pMainCameraFrame->ltm.right = { rot_matrix._11, rot_matrix._12,
                                      rot_matrix._13 };
    m_pMainCameraFrame->ltm.up    = { rot_matrix._21, rot_matrix._22,
                                   rot_matrix._23 };
    m_pMainCameraFrame->ltm.at    = { rot_matrix._31, rot_matrix._32,
                                   rot_matrix._33 };
    auto first_atom =
        rw::engine::rwLinkList::GetFirstLLLink( &m_vClumpList[0]->atomicList );
    auto *atomic = rw::engine::rwLLLink::GetData<RpAtomic>(
        first_atom, offsetof( RpAtomic, inClumpLink ) );
    auto at_ltm = rw::engine::RwFrameGetLTM(
        static_cast<RwFrame *>( rw::engine::rwObject::GetParent( atomic ) ) );
    static float time_stamp = 0;
    time_stamp += dt;
    time_stamp = fmod( time_stamp, 3.14f * 2 );
    // at_ltm->pos.x = sin( time_stamp * 4 ) * 5;
    RwV3d &camPos = m_pMainCameraFrame->ltm.pos;

    const float camSpeed = dt * 10;
    if ( GetKeyState( 'W' ) & 0x8000 )
    {
        camPos.x += m_pMainCameraFrame->ltm.at.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.at.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.at.z * camSpeed;
    }
    if ( GetKeyState( 'S' ) & 0x8000 )
    {
        camPos.x -= m_pMainCameraFrame->ltm.at.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.at.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.at.z * camSpeed;
    }
    if ( GetKeyState( 'A' ) & 0x8000 )
    {
        camPos.x += m_pMainCameraFrame->ltm.right.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.right.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.right.z * camSpeed;
    }
    if ( GetKeyState( 'D' ) & 0x8000 )
    {
        camPos.x -= m_pMainCameraFrame->ltm.right.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.right.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.right.z * camSpeed;
    }
    if ( GetKeyState( 'E' ) & 0x8000 )
    {
        camPos.x += m_pMainCameraFrame->ltm.up.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.up.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.up.z * camSpeed;
    }
    if ( GetKeyState( 'Q' ) & 0x8000 )
    {
        camPos.x -= m_pMainCameraFrame->ltm.up.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.up.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.up.z * camSpeed;
    }
}

void ModelLoadingTest::LoadDFF( const rh::engine::String &path )
{
    RpClump *result = nullptr;
    if ( !rw::engine::LoadClump( result, path ) )
        debug::DebugLogger::Error( "Failed to load clump from dff!" );
    else if ( result != nullptr )
        m_vClumpList.push_back( result );
    for ( auto clump : m_vClumpList )
    {
        RwLLLink *cur, *end, *next;
        cur = rw::engine::rwLinkList::GetFirstLLLink( &clump->atomicList );
        end = rw::engine::rwLinkList::GetTerminator( &clump->atomicList );

        while ( cur != end )
        {
            auto *atomic = rw::engine::rwLLLink::GetData<RpAtomic>(
                cur, offsetof( RpAtomic, inClumpLink ) );
            next = rw::engine::rwLLLink::GetNext( cur );
            if ( atomic )
            {
                rw::engine::RenderStatus status =
                    rw::engine::RwRHInstanceAtomic( atomic,
                                                    &geometry_interface_36 );
                switch ( status )
                {
                case rw::engine::RenderStatus::Failure:
                    debug::DebugLogger::Log( "Failed instance" );
                    break;
                case rw::engine::RenderStatus::Instanced:
                    debug::DebugLogger::Log( "Successful instance" );
                    break;
                case rw::engine::RenderStatus::NotInstanced:
                    debug::DebugLogger::Log( "Unsuccessful instance" );
                    break;
                }
            }

            cur = next;
        }
    }
}
