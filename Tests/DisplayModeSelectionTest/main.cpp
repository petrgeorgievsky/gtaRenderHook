#include <Engine/Common/ISwapchain.h>
#include <Engine/D3D11Impl/D3D11DeviceState.h>
#include <Engine/IRenderer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <Engine/VulkanImpl/VulkanShader.h>
#include <TestUtils\WindowsSampleWrapper.h>
#include <memory>

std::unique_ptr<rh::engine::IRenderer> rh::engine::g_pRHRenderer;

class DisplayModeTest : public rh::tests::TestSample
{
  public:
    DisplayModeTest( rh::engine::RenderingAPI api, HINSTANCE inst )
        : rh::tests::TestSample( api, inst )
    {
    }
    bool Initialize( HWND wnd ) override;

    rh::engine::IFrameBuffer *
    GetFramebufferForFrame( const rh::engine::SwapchainFrame &frame );

  private:
    rh::engine::IDeviceState *mDeviceState  = nullptr;
    rh::engine::IWindow *     mDeviceWindow = nullptr;

    //
    rh::engine::ICommandBuffer *mCmdBuffer     = nullptr;
    rh::engine::ISyncPrimitive *mRenderExecute = nullptr;

    // Per-Frame resources
    rh::engine::ISyncPrimitive *              mImageAquire = nullptr;
    std::array<rh::engine::IFrameBuffer *, 2> mFrameBuffer{nullptr, nullptr};

    //
    rh::engine::IRenderPass *mRenderPass = nullptr;

    //
    rh::engine::IPipeline *m2DPipeline = nullptr;

    //
    rh::engine::IBuffer *mVertexBuffer = nullptr;

    // TestSample interface
  public:
    void CustomShutdown() override;

    // TestSample interface
  public:
    void CustomRender() override;
};

rh::engine::IFrameBuffer *DisplayModeTest::GetFramebufferForFrame(
    const rh::engine::SwapchainFrame &frame )
{
    if ( mFrameBuffer[frame.mImageId] == nullptr )
    {
        std::vector<rh::engine::IImageView *> img_views{frame.mImageView};
        rh::engine::FrameBufferCreateParams   create_params{};
        create_params.width      = frame.mWidth;
        create_params.height     = frame.mHeight;
        create_params.imageViews = img_views;
        create_params.renderPass = mRenderPass;

        mFrameBuffer[frame.mImageId] =
            mDeviceState->CreateFrameBuffer( create_params );
    }
    return mFrameBuffer[frame.mImageId];
}

bool DisplayModeTest::Initialize( HWND window )
{
    mDeviceState = new rh::engine::VulkanDeviceState();

    unsigned int adapter_count;
    assert( mDeviceState->GetAdaptersCount( adapter_count ) );

    unsigned int output_count;
    assert( mDeviceState->GetOutputCount( 0, output_count ) );

    unsigned int display_mode_count;
    assert( mDeviceState->GetDisplayModeCount( 0, display_mode_count ) );

    mDeviceState->Init();

    rh::engine::DisplayModeInfo dm_info{};
    auto r = mDeviceState->GetDisplayModeInfo( 0, dm_info );
    assert( r );

    rh::engine::OutputInfo info{};
    info.displayModeId = 0;
    info.windowed      = true;
    mDeviceWindow      = mDeviceState->CreateDeviceWindow( window, info );

    mCmdBuffer = mDeviceState->GetMainCommandBuffer();
    mImageAquire =
        mDeviceState->CreateSyncPrimitive( rh::engine::SyncPrimitiveType::GPU );
    mRenderExecute =
        mDeviceState->CreateSyncPrimitive( rh::engine::SyncPrimitiveType::GPU );

    // Basic render pass
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

    mRenderPass = mDeviceState->CreateRenderPass( render_pass_desc );

    rh::engine::ShaderDesc vs_desc{};
    vs_desc.mShaderPath  = "Im2D.hlsl";
    vs_desc.mEntryPoint  = "BaseVS";
    vs_desc.mShaderStage = rh::engine::ShaderStage::Vertex;
    auto vs_shader       = mDeviceState->CreateShader( vs_desc );

    rh::engine::ShaderDesc ps_desc{};
    ps_desc.mShaderPath  = "Im2D.hlsl";
    ps_desc.mEntryPoint  = "NoTexPS";
    ps_desc.mShaderStage = rh::engine::ShaderStage::Pixel;
    auto ps_shader       = mDeviceState->CreateShader( ps_desc );

    rh::engine::ShaderStageDesc vs_stage_desc{};
    vs_stage_desc.mShader     = vs_shader;
    vs_stage_desc.mStage      = vs_desc.mShaderStage;
    vs_stage_desc.mEntryPoint = vs_desc.mEntryPoint;

    rh::engine::ShaderStageDesc ps_stage_desc{};
    ps_stage_desc.mShader     = ps_shader;
    ps_stage_desc.mStage      = ps_desc.mShaderStage;
    ps_stage_desc.mEntryPoint = ps_desc.mEntryPoint;

    rh::engine::PipelineCreateParams pipe_create_params{};
    pipe_create_params.mRenderPass = mRenderPass;
    pipe_create_params.mShaderStages.push_back( vs_stage_desc );
    pipe_create_params.mShaderStages.push_back( ps_stage_desc );

    m2DPipeline = mDeviceState->CreatePipeline( pipe_create_params );
    struct VertexDesc
    {
        float    x, y, z, w;
        uint32_t color;
        float    u, v;
    };
    std::vector<VertexDesc> triangle_verticles = {
        {0, 0, 0, 1, // pos
         0xFFFFFFFF, // color
         0, 0},
        // v0
        {0, 720, 0, 1, // pos
         0xFFFF0000,   // color
         0, 0},
        // v1
        {1280, 0, 0, 1, // pos
         0xFF0000FF,    // color
         0, 0},
        // v2
        {1280, 0, 0, 1, // pos
         0xFF00FF00,    // color
         0, 0},
        // v3
        {1280, 720, 0, 1, // pos
         0xFFFF0000,      // color
         0, 0},
        // v4
        {0, 720, 0, 1, // pos
         0xFFFF00FF,   // color
         0, 0}
        // v5
    };
    rh::engine::BufferCreateInfo v_buff_create_info{};
    v_buff_create_info.mSize = sizeof( VertexDesc ) * triangle_verticles.size();
    v_buff_create_info.mUsage = rh::engine::BufferUsage::VertexBuffer;
    mVertexBuffer = mDeviceState->CreateBuffer( v_buff_create_info );
    mVertexBuffer->Update( triangle_verticles.data(),
                           v_buff_create_info.mSize );
    delete vs_shader;
    delete ps_shader;
    return true;
}

void DisplayModeTest::CustomRender()
{
    auto [swapchain, invalidate_framebuffers] = mDeviceWindow->GetSwapchain();
    if ( invalidate_framebuffers )
    {
        for ( auto fb : mFrameBuffer )
            delete fb;
        mFrameBuffer[0] = mFrameBuffer[1] = nullptr;
    }
    auto frame = swapchain->GetAvaliableFrame( mImageAquire );

    // Record frame
    mCmdBuffer->BeginRecord();

    rh::engine::RenderPassBeginInfo info{};
    info.m_pRenderPass  = mRenderPass;
    info.m_pFrameBuffer = GetFramebufferForFrame( frame );
    info.m_aClearValues = {
        {rh::engine::ClearValueType::Color, {0, 0, 128, 0xFF}, {}}};

    mCmdBuffer->BeginRenderPass( info );
    auto fb_info = info.m_pFrameBuffer->GetInfo();

    mCmdBuffer->SetViewports(
        0, {rh::engine::ViewPort{
               0,                                    // float topLeftX;
               0,                                    // float topLeftY;
               static_cast<float>( fb_info.width ),  // float width;
               static_cast<float>( fb_info.height ), // float height;
               0,                                    // float minDepth;
               1.0                                   // float maxDepth;
           }} );

    mCmdBuffer->SetScissors( 0, {rh::engine::Scissor{
                                    0,             // float topLeftX;
                                    0,             // float topLeftY;
                                    fb_info.width, // float width;
                                    fb_info.height // float height;
                                }} );
    mCmdBuffer->BindPipeline( m2DPipeline );
    mCmdBuffer->BindVertexBuffers( 0, {{mVertexBuffer, 0}} );
    mCmdBuffer->Draw( 6, 1, 0, 0 );

    mCmdBuffer->EndRenderPass();

    mCmdBuffer->EndRecord();

    // Submit command buffer
    mDeviceState->ExecuteCommandBuffer( mCmdBuffer, mImageAquire,
                                        mRenderExecute );

    swapchain->Present( frame, mRenderExecute );

    std::vector<rh::engine::ISyncPrimitive *> exec_prim_list = {
        mCmdBuffer->ExecutionFinishedPrimitive()};

    mDeviceState->Wait( exec_prim_list );

    static bool res_switch = true;
    if ( res_switch )
    {
        rh::engine::DisplayModeInfo dm_info{};
        mDeviceState->GetDisplayModeInfo( 25, dm_info );
        rh::engine::WindowParams params{};
        params.mWidth  = dm_info.width;
        params.mHeight = dm_info.height;
        mDeviceWindow->SetWindowParams( params );
        res_switch = false;
    }
}

void DisplayModeTest::CustomShutdown()
{
    delete mVertexBuffer;
    delete m2DPipeline;
    delete mRenderPass;
    for ( auto fb : mFrameBuffer )
        delete fb;
    delete mImageAquire;
    delete mRenderExecute;
    delete mDeviceWindow;
    mDeviceState->Shutdown();
    delete mDeviceState;
}

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
