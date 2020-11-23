
#include <TestUtils/WindowsSampleWrapper.h>
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <cassert>
#include <sstream>
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "dinput8.lib" )

TCHAR szName[]     = TEXT( "Local\\RenderHookSharedMemory" );
TCHAR event_name[]          = TEXT( "Local\\RenderHookInitDeviceEvent" );
TCHAR render_start_event[]  = TEXT( "Local\\RenderHookRenderStartEvent" );
TCHAR render_finish_event[] = TEXT( "Local\\RenderHookRenderFinishEvent" );
TCHAR init_finish_event[] = TEXT( "Local\\RenderHookInitFinishEvent" );
#define SHARED_MEMORY_SIZE 1024 * 1024 * 1024

class DisplayModeTest : public rh::tests::TestSample
{
  public:
    DisplayModeTest( rh::engine::RenderingAPI api, HINSTANCE inst )
        : rh::tests::TestSample( api, inst )
    {
    }
    bool Initialize( void *wnd ) override
    {
        STARTUPINFOA start_info{};
        start_info.cb = sizeof( start_info );
        PROCESS_INFORMATION proc_info{};
        std::stringstream   cmd_args;
        cmd_args << "C:\\Users\\peter\\Documents\\Visual Studio 2015\\Projects\\gtaRenderHook\\build\\rw_x64_render_driver\\rw_x64_render_driver.exe";

        auto cmd_args_str = cmd_args.str();
        char cmd_args_cstr[512];
        strcpy_s( cmd_args_cstr, cmd_args_str.c_str() );

        _SECURITY_ATTRIBUTES eventsecattr{};
        eventsecattr.nLength = sizeof( _SECURITY_ATTRIBUTES );
        eventsecattr.bInheritHandle = true;
        hEventHandle = CreateEvent( &eventsecattr, TRUE, FALSE, event_name );
        mRenderStart =
            CreateEvent( &eventsecattr, TRUE, FALSE, render_start_event );
        mRenderFinish =
            CreateEvent( &eventsecattr, TRUE, FALSE, render_finish_event );
        mInitEvent =
            CreateEvent( &eventsecattr, TRUE, FALSE, init_finish_event );
        BOOL result =
            CreateProcessA( nullptr, cmd_args_cstr, nullptr, nullptr, false, 0,
                            nullptr, nullptr, &start_info, &proc_info );
        hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE, // use paging file
            NULL,                 // default security
            PAGE_READWRITE,       // read/write access
            0,                    // maximum object size (high-order DWORD)
            SHARED_MEMORY_SIZE,   // maximum object size (low-order DWORD)
            szName );             // name of mapping object

        if ( hMapFile == NULL )
        {
            _tprintf( TEXT( "Could not open file mapping object (%d).\n" ),
                      static_cast<int>( GetLastError() ) );
            return 1;
        }

        mSharedMemory =
            MapViewOfFile( hMapFile,            // handle to map object
                                   FILE_MAP_ALL_ACCESS, // read/write permission
                                   0, 0, SHARED_MEMORY_SIZE );

        if ( mSharedMemory == NULL )
        {
            _tprintf( TEXT( "Could not map view of file (%d).\n" ),
                      static_cast<int>( GetLastError() ) );

            CloseHandle( hMapFile );

            return 1;
        }
        CopyMemory( mSharedMemory, &wnd, sizeof( HWND ) );
        SetEvent( hEventHandle );
        WaitForSingleObject( mInitEvent, INFINITE );
        return true;
    }

  private:
    // TestSample interface
    HANDLE hMapFile;
    HANDLE hEventHandle;
    LPVOID mSharedMemory;
    HANDLE mRenderStart;
    HANDLE mRenderFinish;
    HANDLE mInitEvent;
  public:
    void CustomShutdown() override
    {
        UnmapViewOfFile( mSharedMemory );
        CloseHandle( hMapFile );

    }

    // TestSample interface
  public:
    void CustomRender() override { 
        struct render_color
        {
            float r, g, b, a;
        };
        render_color color{0.1f, 0.0f, float( rand() % 100 ) / 100.0f, 1.0f};
        CopyMemory( mSharedMemory, &color, sizeof( render_color ) );
        SetEvent( mRenderStart );

        WaitForSingleObject( mRenderFinish, INFINITE );
        ResetEvent( mRenderFinish );
      }
};


int APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
    UNREFERENCED_PARAMETER( nCmdShow );

    // Window params initialization
    rh::tests::WindowsSampleParams initParams;
    initParams.instance    = hInstance;
    initParams.sampleTitle = TEXT( "DisplayModeTest" );
    initParams.windowClass = TEXT( "DISPLAYMODETEST" );

    rh::tests::WindowsSampleWrapper sample(
        initParams, std::make_unique<DisplayModeTest>(
                        rh::engine::RenderingAPI::Vulkan, hInstance ) );

    // Initialize test sample.
    if ( !sample.Init() )
        return FALSE;

    // Run test sample!
    sample.Run();

    return TRUE;
}
