#include <DebugUtils\DebugLogger.h>
#include <Engine\Common\ISwapchain.h>
#include <Engine\Common\IWindow.h>
#include <Engine\VulkanImpl\VulkanDeviceState.h>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <tchar.h>

#define BUF_SIZE 256

#define SHARED_MEMORY_SIZE 1024 * 1024 * 1024

TCHAR szName[]              = TEXT( "Local\\RenderHookSharedMemory" );
TCHAR event_name[]          = TEXT( "Local\\RenderHookInitDeviceEvent" );
TCHAR render_start_event[]  = TEXT( "Local\\RenderHookRenderStartEvent" );
TCHAR render_finish_event[] = TEXT( "Local\\RenderHookRenderFinishEvent" );
TCHAR init_finish_event[]   = TEXT( "Local\\RenderHookInitFinishEvent" );
TCHAR szMsg[]               = TEXT( "Message from first process." );

rh::engine::IDeviceState *gDeviceState = nullptr;
struct PerFrameResources
{
    rh::engine::ISyncPrimitive *mImageAquire;
    rh::engine::ISyncPrimitive *mRenderExecute;
    rh::engine::ICommandBuffer *mCmdBuffer;
    bool                        mBufferIsRecorded = false;
};
std::array<PerFrameResources, 16>         gPerFrameResources{};
std::array<rh::engine::IFrameBuffer *, 8> gFrameBuffer{
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
rh::engine::IRenderPass *gRenderPass = nullptr;

HANDLE gRenderStart;
HANDLE gRenderFinish;
HANDLE gInitEvent;

rh::engine::IFrameBuffer *
GetFramebufferForFrame( const rh::engine::SwapchainFrame &frame )
{
    if ( gFrameBuffer[frame.mImageId] == nullptr )
    {
        std::vector<rh::engine::IImageView *> img_views{ frame.mImageView };
        rh::engine::FrameBufferCreateParams   create_params{};
        create_params.width      = frame.mWidth;
        create_params.height     = frame.mHeight;
        create_params.imageViews = img_views;
        create_params.renderPass = gRenderPass;

        gFrameBuffer[frame.mImageId] =
            gDeviceState->CreateFrameBuffer( create_params );
    }
    return gFrameBuffer[frame.mImageId];
}

/// REQUIRED TO BE CROSSPROCESS COMPATIBLE
struct FrameInfo
{
    int32_t mClearColor;
};

struct SceneGraph
{
    FrameInfo mFrameInfo;
};

SceneGraph GetSceneGraph()
{
    LPCTSTR    pBuf;
    SceneGraph scene_graph;
    WaitForSingleObject( gRenderStart, INFINITE );
    CopyMemory( &scene_graph, pBuf, ( sizeof( SceneGraph ) ) );
    ResetEvent( gRenderStart );
}

void BeginFrame( const FrameInfo &frame_info )
{
    auto swapchain = window_view->GetSwapchain().mSwapchain;

    auto cmdbuffer   = gPerFrameResources[current_frame].mCmdBuffer;
    auto render_exec = gPerFrameResources[current_frame].mRenderExecute;
    auto img_aq      = gPerFrameResources[current_frame].mImageAquire;

    if ( gPerFrameResources[current_frame].mBufferIsRecorded )
    {
        std::array exec_prim_list = { cmdbuffer->ExecutionFinishedPrimitive() };

        gDeviceState->Wait( exec_prim_list );
        gPerFrameResources[current_frame].mBufferIsRecorded = false;
    }

    auto frame = swapchain->GetAvaliableFrame( img_aq );
    cmdbuffer->BeginRecord();
    std::array clear_values = {
        rh::engine::ClearValue{ rh::engine::ClearValueType::Color,
                                rh::engine::ClearValue::ClearColor{
                                    static_cast<uint8_t>( color.r * 255 ),
                                    static_cast<uint8_t>( color.g * 255 ),
                                    static_cast<uint8_t>( color.b * 255 ),
                                    static_cast<uint8_t>( color.a * 255 ) },
                                {} } };

    rh::engine::RenderPassBeginInfo info{};
    info.m_pRenderPass  = gRenderPass;
    info.m_pFrameBuffer = GetFramebufferForFrame( frame );
    info.m_aClearValues = clear_values;
    cmdbuffer->BeginRenderPass( info );
}
void EndFrame( const FrameInfo &frame_info )
{
    cmdbuffer->EndRenderPass();

    cmdbuffer->EndRecord();

    gDeviceState->ExecuteCommandBuffer( cmdbuffer, img_aq, render_exec );
    gPerFrameResources[current_frame].mBufferIsRecorded = true;
    swapchain->Present( frame.mImageId, render_exec );
    current_frame = ( current_frame + 1 ) % gPerFrameResources.size();
    SetEvent( gRenderFinish );
}
void RenderLoop()
{
    auto scene_graph = GetSceneGraph();
    BeginFrame( scene_graph.mFrameInfo );
    // RenderIm2D( scene_graph );
    EndFrame( scene_graph.mFrameInfo );
}

int main()
{
    rh::debug::DebugLogger::Init( "execution_result.txt",
                                  rh::debug::LogLevel::ConstrDestrInfo );
    // Init engine
    gDeviceState = new rh::engine::VulkanDeviceState();
    gDeviceState->Init();

    for ( size_t i = 0; i < gPerFrameResources.size(); i++ )
    {
        gPerFrameResources[i].mCmdBuffer = gDeviceState->CreateCommandBuffer();
        gPerFrameResources[i].mImageAquire = gDeviceState->CreateSyncPrimitive(
            rh::engine::SyncPrimitiveType::GPU );
        gPerFrameResources[i].mRenderExecute =
            gDeviceState->CreateSyncPrimitive(
                rh::engine::SyncPrimitiveType::GPU );
    }

    rh::engine::RenderPassCreateParams render_pass_desc{};

    rh::engine::AttachmentDescription render_pass_color_desc{};
    render_pass_color_desc.mFormat     = rh::engine::ImageBufferFormat::BGRA8;
    render_pass_color_desc.mSrcLayout  = rh::engine::ImageLayout::Undefined;
    render_pass_color_desc.mDestLayout = rh::engine::ImageLayout::PresentSrc;
    render_pass_color_desc.mLoadOp     = rh::engine::LoadOp::Clear;
    render_pass_color_desc.mStoreOp    = rh::engine::StoreOp::Store;
    render_pass_color_desc.mStencilLoadOp  = rh::engine::LoadOp::DontCare;
    render_pass_color_desc.mStencilStoreOp = rh::engine::StoreOp::DontCare;
    render_pass_desc.mAttachments.push_back( render_pass_color_desc );

    rh::engine::SubpassDescription main_subpass{};
    rh::engine::AttachmentRef      color_attach_ref{};
    color_attach_ref.mReqLayout = rh::engine::ImageLayout::ColorAttachment;
    main_subpass.mBindPoint     = rh::engine::PipelineBindPoint::Graphics;
    main_subpass.mColorAttachments.push_back( color_attach_ref );
    render_pass_desc.mSubpasses.push_back( main_subpass );

    gRenderPass = gDeviceState->CreateRenderPass( render_pass_desc );
    // Retrieve window handle via multiprocess
    // 0. Create shared memory
    HANDLE  hMapFile;
    LPCTSTR pBuf;
    {
        hMapFile =
            OpenFileMapping( FILE_MAP_ALL_ACCESS, // read/write access
                             FALSE,    // maximum object size (high-order DWORD)
                             szName ); // name of mapping object

        if ( hMapFile == nullptr )
        {
            _tprintf( TEXT( "Could not create file mapping object (%d).\n" ),
                      static_cast<int>( GetLastError() ) );
            return 1;
        }

        pBuf =
            (LPTSTR)MapViewOfFile( hMapFile,            // handle to map object
                                   FILE_MAP_ALL_ACCESS, // read/write permission
                                   0, 0, SHARED_MEMORY_SIZE );

        if ( pBuf == nullptr )
        {
            _tprintf( TEXT( "Could not map view of file (%d).\n" ),
                      static_cast<int>( GetLastError() ) );

            CloseHandle( hMapFile );

            return 1;
        }
    }
    // 1. Create and wait for event
    auto window_init_handle = OpenEvent( EVENT_ALL_ACCESS, false, event_name );
    gRenderStart  = OpenEvent( EVENT_ALL_ACCESS, false, render_start_event );
    gRenderFinish = OpenEvent( EVENT_ALL_ACCESS, false, render_finish_event );
    gInitEvent    = OpenEvent( EVENT_ALL_ACCESS, false, init_finish_event );
    if ( window_init_handle == NULL )
    {
        _tprintf( TEXT( "Could not open event (%d).\n" ),
                  static_cast<int>( GetLastError() ) );

        UnmapViewOfFile( pBuf );
        CloseHandle( hMapFile );

        return 1;
    }
    WaitForSingleObject( window_init_handle, INFINITE );
    ResetEvent( window_init_handle );
    rh::debug::DebugLogger::Log( "Wait is over now!" );
    HWND hwnd_ptr;
    CopyMemory( &hwnd_ptr, pBuf, ( sizeof( HWND ) ) );

    rh::engine::OutputInfo window_output{};
    window_output.displayModeId = 0;
    window_output.windowed      = true;

    auto window_view =
        gDeviceState->CreateDeviceWindow( hwnd_ptr, window_output );
    SetEvent( gInitEvent );
    {
        while ( true )
        {
            struct render_color
            {
                float r, g, b, a;
            };
            render_color color{};
            WaitForSingleObject( gRenderStart, INFINITE );
            CopyMemory( &color, pBuf, ( sizeof( render_color ) ) );
            ResetEvent( gRenderStart );

            static int current_frame = 0;

            auto swapchain = window_view->GetSwapchain().mSwapchain;

            auto cmdbuffer   = gPerFrameResources[current_frame].mCmdBuffer;
            auto render_exec = gPerFrameResources[current_frame].mRenderExecute;
            auto img_aq      = gPerFrameResources[current_frame].mImageAquire;

            if ( gPerFrameResources[current_frame].mBufferIsRecorded )
            {
                std::array exec_prim_list = {
                    cmdbuffer->ExecutionFinishedPrimitive() };

                gDeviceState->Wait( exec_prim_list );
                gPerFrameResources[current_frame].mBufferIsRecorded = false;
            }

            auto frame = swapchain->GetAvaliableFrame( img_aq );
            cmdbuffer->BeginRecord();
            std::array clear_values = { rh::engine::ClearValue{
                rh::engine::ClearValueType::Color,
                rh::engine::ClearValue::ClearColor{
                    static_cast<uint8_t>( color.r * 255 ),
                    static_cast<uint8_t>( color.g * 255 ),
                    static_cast<uint8_t>( color.b * 255 ),
                    static_cast<uint8_t>( color.a * 255 ) },
                {} } };

            rh::engine::RenderPassBeginInfo info{};
            info.m_pRenderPass  = gRenderPass;
            info.m_pFrameBuffer = GetFramebufferForFrame( frame );
            info.m_aClearValues = clear_values;
            cmdbuffer->BeginRenderPass( info );
            cmdbuffer->EndRenderPass();

            cmdbuffer->EndRecord();

            gDeviceState->ExecuteCommandBuffer( cmdbuffer, img_aq,
                                                render_exec );
            gPerFrameResources[current_frame].mBufferIsRecorded = true;
            swapchain->Present( frame.mImageId, render_exec );
            current_frame = ( current_frame + 1 ) % gPerFrameResources.size();
            SetEvent( gRenderFinish );
        }
    }
    delete window_view;

    UnmapViewOfFile( pBuf );
    CloseHandle( hMapFile );

    CloseHandle( window_init_handle );
    return 0;
}