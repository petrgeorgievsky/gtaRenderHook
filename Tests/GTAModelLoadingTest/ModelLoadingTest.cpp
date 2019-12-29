#include "ModelLoadingTest.h"
#include "forward_pbr_pipeline.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IGPUAllocator.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/constant_buffer_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <common_headers.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rp_clump/rp_clump.h>
#include <rw_engine/rw_camera/rw_camera.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_pipeline.h>
#include <rw_engine/rw_tex_dict/rw_tex_dict.h>
#include <rw_engine/rw_texture/rw_texture.h>
using namespace rh;

static rw::engine::RpGeometryRw36 geometry_interface_36{};
bool ModelLoadingTest::CustomInitialize()
{
    if ( !rw::engine::RwTestSample::CustomInitialize() )
        return false;
    m_sDffPath = "models/test.dff";
    m_pPipeline = new ForwardPBRPipeline();

    engine::IGPUAllocator *allocator = engine::g_pRHRenderer->GetGPUAllocator();

    allocator->AllocateConstantBuffer( {sizeof( rw::engine::CameraContext )}, mBaseConstantBuffer );
    allocator->AllocateConstantBuffer( {sizeof( DirectX::XMMATRIX )}, mPerModelConstantBuffer );

    LoadDFF( m_sDffPath );

    return true;
}

void ModelLoadingTest::CustomShutdown()
{
    TestSample::CustomShutdown();
}

void ModelLoadingTest::CustomRender()
{
    RwRGBA clearColor = {128, 128, 255, 255};

    engine::IRenderingContext *context = static_cast<engine::IRenderingContext *>(
        engine::g_pRHRenderer->GetCurrentContext() );
    context->UpdateBuffer( mBaseConstantBuffer,
                           rw::engine::g_cameraContext,
                           sizeof( rw::engine::CameraContext ) );

    rw::engine::RwCameraClear( m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE | rwCAMERACLEARZ );

    context->BindConstantBuffers( engine::ShaderStage::Vertex | engine::ShaderStage::Pixel,
                                  {{0, mBaseConstantBuffer}, {1, mPerModelConstantBuffer}} );
    for ( auto clump : m_vClumpList ) {
        RwLLLink *cur, *end, *next;
        cur = rw::engine::rwLinkList::GetFirstLLLink( &clump->atomicList );
        end = rw::engine::rwLinkList::GetTerminator( &clump->atomicList );

        while ( cur != end ) {
            RpAtomic *atomic = rw::engine::rwLLLink::GetData<RpAtomic>( cur,
                                                                        offsetof( RpAtomic,
                                                                                  inClumpLink ) );
            next = rw::engine::rwLLLink::GetNext( cur );
            if ( atomic ) {
                auto ltm = rw::engine::RwFrameGetLTM(
                    static_cast<RwFrame *>( rw::engine::rwObject::GetParent( atomic ) ) );
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
                context->UpdateBuffer( mPerModelConstantBuffer,
                                       &objTransformMatrix,
                                       sizeof( objTransformMatrix ) );
                rw::engine::DrawAtomic( atomic, &geometry_interface_36, context, m_pPipeline );
            }

            cur = next;
        }
    }
}

void ModelLoadingTest::CustomUpdate( float dt )
{
    DIMOUSESTATE mouseCurrState{};
    if ( GetKeyState( VK_CONTROL ) & 0x8000 ) {
        if ( !m_bMouseAquired ) {
            m_pMouse->Acquire();
            m_bMouseAquired = true;
        }
        m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE ), &mouseCurrState );
    } else {
        if ( m_bMouseAquired ) {
            m_pMouse->Unacquire();
            m_bMouseAquired = false;
        }
    }

    RwV2d mousePosRw = {mouseCurrState.lX * 0.005f, mouseCurrState.lY * 0.005f};

    DirectX::XMMATRIX currentRot = {m_pMainCameraFrame->ltm.right.x,
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
                                    1};

    DirectX::XMMATRIX rightRotAxis
        = DirectX::XMMatrixRotationAxis( {{{m_pMainCameraFrame->ltm.right.x,
                                            m_pMainCameraFrame->ltm.right.y,
                                            m_pMainCameraFrame->ltm.right.z}}},
                                         ( mousePosRw.y ) );

    DirectX::XMMATRIX upRotAxis = DirectX::XMMatrixRotationAxis( {{{-m_pMainCameraFrame->ltm.up.x,
                                                                    -m_pMainCameraFrame->ltm.up.y,
                                                                    -m_pMainCameraFrame->ltm.up.z}}},
                                                                 ( mousePosRw.x ) );

    const DirectX::XMMATRIX rotMat = currentRot * rightRotAxis * upRotAxis;
    m_pMainCameraFrame->ltm.right = {rotMat.r[0].vector4_f32[0],
                                     rotMat.r[0].vector4_f32[1],
                                     rotMat.r[0].vector4_f32[2]};
    m_pMainCameraFrame->ltm.up = {rotMat.r[1].vector4_f32[0],
                                  rotMat.r[1].vector4_f32[1],
                                  rotMat.r[1].vector4_f32[2]};
    m_pMainCameraFrame->ltm.at = {rotMat.r[2].vector4_f32[0],
                                  rotMat.r[2].vector4_f32[1],
                                  rotMat.r[2].vector4_f32[2]};

    RwV3d &camPos = m_pMainCameraFrame->ltm.pos;

    const float camSpeed = dt * 10;
    if ( GetKeyState( 0x57 ) & 0x8000 ) {
        camPos.x += m_pMainCameraFrame->ltm.at.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.at.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.at.z * camSpeed;
    }
    if ( GetKeyState( 0x53 ) & 0x8000 ) {
        camPos.x -= m_pMainCameraFrame->ltm.at.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.at.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.at.z * camSpeed;
    }
    if ( GetKeyState( 0x41 ) & 0x8000 ) {
        camPos.x += m_pMainCameraFrame->ltm.right.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.right.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.right.z * camSpeed;
    }
    if ( GetKeyState( 0x44 ) & 0x8000 ) {
        camPos.x -= m_pMainCameraFrame->ltm.right.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.right.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.right.z * camSpeed;
    }
    if ( GetKeyState( 0x45 ) & 0x8000 ) {
        camPos.x += m_pMainCameraFrame->ltm.up.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.up.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.up.z * camSpeed;
    }
    if ( GetKeyState( 0x51 ) & 0x8000 ) {
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
    for ( auto clump : m_vClumpList ) {
        RwLLLink *cur, *end, *next;
        cur = rw::engine::rwLinkList::GetFirstLLLink( &clump->atomicList );
        end = rw::engine::rwLinkList::GetTerminator( &clump->atomicList );

        while ( cur != end ) {
            RpAtomic *atomic = rw::engine::rwLLLink::GetData<RpAtomic>( cur,
                                                                        offsetof( RpAtomic,
                                                                                  inClumpLink ) );
            next = rw::engine::rwLLLink::GetNext( cur );
            if ( atomic ) {
                rw::engine::RenderStatus status
                    = rw::engine::RwRHInstanceAtomic( atomic, &geometry_interface_36 );
                switch ( status ) {
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
