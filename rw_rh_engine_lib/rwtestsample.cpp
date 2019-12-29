#include "rwtestsample.h"
#include "rw_engine/global_definitions.h"
#include "rw_engine/rw_camera/rw_camera.h"
#include "rw_engine/rw_frame/rw_frame.h"
#include "rw_engine/rw_raster/rw_raster.h"
#include "rw_engine/rw_standard_render_commands/camerabeginupdatecmd.h"
#include "rw_engine/rw_standard_render_commands/cameraendupdatecmd.h"
#include "rw_engine/rw_standard_render_commands/rastershowrastercmd.h"
#include <DebugUtils/DebugLogger.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_win32.h>
using namespace rh::rw::engine;

RwGlobals *rh::rw::engine::g_pRwEngineInstance;
RwTestSample::RwTestSample( rh::engine::RenderingAPI api, HINSTANCE inst ) : TestSample( api, inst )
{
    // TODO: REMOVE
    if ( api == rh::engine::RenderingAPI::Vulkan )
        m_bRenderGUI = false;
    g_pRwEngineInstance = static_cast<RwGlobals *>( malloc( sizeof( RwGlobals ) ) );
    g_pRwRenderEngine = std::make_unique<RwRenderEngine>( api );
}

RwTestSample::~RwTestSample()
{
    free( g_pRwEngineInstance );
}

bool RwTestSample::Initialize( HWND wnd )
{
    unsigned int gpuCount;

    // Preparation of rendering engine, initializes info about hardware that'll
    // use this window
    if ( !g_pRwRenderEngine->Open( wnd ) ) {
        rh::debug::DebugLogger::Error( TEXT( "Failed to open RHEngine!" ) );
        return false;
    }

    // Preparation of standard rendering callbacks
    if ( !g_pRwRenderEngine->Standards( reinterpret_cast<int *>( g_pRwEngineInstance->stdFunc ),
                                        rwSTANDARDNUMOFSTANDARD ) ) {
        rh::debug::DebugLogger::Error( TEXT( "Failed to initialize RH engine standards!" ) );
        return false;
    }

    // GPU count retrieval
    if ( !g_pRwRenderEngine->GetNumSubSystems( gpuCount ) ) {
        g_pRwRenderEngine->Close();
        rh::debug::DebugLogger::Error( TEXT( "Failed to get gpu count!" ) );
        return false;
    }

    rh::debug::DebugLogger::Log( TEXT( "GPU List:" ) );

    // Enumerate over GPUs and log all the info
    for ( unsigned int i = 0; i < gpuCount; i++ ) {
        RwSubSystemInfo info;

        if ( !g_pRwRenderEngine->GetSubSystemInfo( info, i ) ) {
            g_pRwRenderEngine->Close();
            rh::debug::DebugLogger::Error( TEXT( "Failed to get gpu info!" ) );
            return false;
        }

        // m_aSubSysInfo.push_back(info);
        rh::debug::DebugLogger::Log( ToRHString( info.name ) );
    }

    // Show device settings dialog
    /*DeviceSettingsDialog::SetSubSystemInfo(m_aSubSysInfo);

if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), wnd,
DeviceSettingsDialog::DialogProc) <= 0) { TCHAR errormsg[4096];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  nullptr,
                  GetLastError(),
                  0,
                  errormsg,
                  4096,
                  nullptr);
    RHDebug::DebugLogger::Error(ToRHString(errormsg));
}*/
    RwVideoMode videoMode{};
    int modeId = 0;
    int maxModeId = 0;
    g_pRwRenderEngine->GetNumModes( maxModeId );
    while ( videoMode.width != 1280 && videoMode.height != 720 && modeId < maxModeId ) {
        if ( !g_pRwRenderEngine->GetModeInfo( videoMode, modeId ) )
            break;
        modeId++;
    }

    g_pRwRenderEngine->UseMode( static_cast<unsigned int>( modeId - 1 ) );
    if ( !g_pRwRenderEngine->Start() ) {
        rh::debug::DebugLogger::Error( TEXT( "Failed to start RWEngine!" ) );
        g_pRwRenderEngine->Close();
        return false;
    }
    rh::engine::g_pRHRenderer->InitImGUI();

    return CustomInitialize();
}

bool RwTestSample::CustomInitialize()
{
    m_pMainCameraFrame = RwFrameCreate();
    m_pMainCamera = RwCameraCreate();
    m_pMainCamera->frameBuffer = rh::rw::engine::RwRasterCreate( 1280, 720, 32, rwRASTERTYPECAMERA );
    m_pMainCamera->zBuffer = rh::rw::engine::RwRasterCreate( 1280, 720, 32, rwRASTERTYPEZBUFFER );
    m_pMainCamera->object.object.parent = static_cast<void *>( m_pMainCameraFrame );
    return true;
}

void RwTestSample::CustomShutdown()
{
    if ( m_pMainCamera ) {
        if ( m_pMainCamera->frameBuffer ) {
            rh::rw::engine::RwRasterDestroy( m_pMainCamera->frameBuffer );
            m_pMainCamera->frameBuffer = nullptr;
        }

        if ( m_pMainCamera->zBuffer ) {
            rh::rw::engine::RwRasterDestroy( m_pMainCamera->zBuffer );
            m_pMainCamera->zBuffer = nullptr;
        }

        rh::rw::engine::RwCameraDestroy( m_pMainCamera );
        m_pMainCamera = nullptr;
    }

    if ( g_pRwRenderEngine ) {
        rh::engine::g_pRHRenderer->ShutdownImGUI();
        g_pRwRenderEngine->Stop();
        g_pRwRenderEngine->Close();
    }
}

void RwTestSample::Render()
{
    if ( m_bRenderGUI ) {
        rh::engine::g_pRHRenderer->ImGUIStartFrame();

        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    RwCameraBeginUpdateCmd begin_upd( m_pMainCamera );
    begin_upd.Execute();

    CustomRender();

    if ( m_bRenderGUI ) {
        ImGui::Render();
        rh::engine::g_pRHRenderer->ImGUIRender();
    }
    RwCameraEndUpdateCmd end_upd( m_pMainCamera );
    end_upd.Execute();

    RwRasterShowRasterCmd rast_show_cmd( m_pMainCamera->frameBuffer, 0 );
    rast_show_cmd.Execute();
}
