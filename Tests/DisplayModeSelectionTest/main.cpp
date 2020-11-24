#include "Im2DRenderer.h"
#include <Engine/Common/ISwapchain.h>
#include <Engine/D3D11Impl/D3D11DeviceState.h>
#include <Engine/IRenderer.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <Engine/VulkanImpl/VulkanShader.h>
#include <TestUtils/WindowsSampleWrapper.h>
#include <filesystem>
#include <memory>

std::unique_ptr<rh::engine::IRenderer> rh::engine::g_pRHRenderer;

struct PerFrameResources
{
    rh::engine::ISyncPrimitive *mImageAquire;
    rh::engine::ISyncPrimitive *mRenderExecute;
    rh::engine::ICommandBuffer *mCmdBuffer;
    bool                        mBufferIsRecorded = false;
};

class DisplayModeTest : public rh::tests::TestSample
{
  public:
    DisplayModeTest( rh::engine::RenderingAPI api, HINSTANCE inst )
        : rh::tests::TestSample( api, inst )
    {
    }
    bool Initialize( void *wnd ) override;

    rh::engine::IFrameBuffer *
    GetFramebufferForFrame( const rh::engine::SwapchainFrame &frame );

  private:
    rh::engine::IDeviceState *mDeviceState  = nullptr;
    rh::engine::IWindow *     mDeviceWindow = nullptr;

    // Per-Frame resources
    std::array<rh::engine::IFrameBuffer *, 8> mFrameBuffer{
        nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr };

    std::array<PerFrameResources, 16> mPerFrameResources{};

    //
    rh::engine::IRenderPass *mRenderPass = nullptr;

    // 2d renderer
    std::unique_ptr<Im2DRenderer> m2DRenderer;

    //
    rh::engine::IBuffer *   mGlobalsBuffer = nullptr;
    rh::engine::IImageView *mImageView     = nullptr;

    rh::engine::IDescriptorSetLayout *   mGlobalDescSetLayout = nullptr;
    rh::engine::IDescriptorSet *         mGlobalDescSet       = nullptr;
    rh::engine::IDescriptorSetAllocator *mGlobalDescSetAlloc  = nullptr;

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
        std::vector<rh::engine::IImageView *> img_views{ frame.mImageView };
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

struct RenderStateBuffer
{
    float fScreenWidth;
    float fScreenHeight;
    float fRedness;
    float fPadding;
};

RenderStateBuffer gGlobalStateBuffer;

bool DisplayModeTest::Initialize( void *window )
{
    bool d3d11_test = true;
    /*if ( d3d11_test )
        mDeviceState = new rh::engine::D3D11DeviceState();
    else*/
    mDeviceState = new rh::engine::VulkanDeviceState();

    mDeviceState->Init();

    rh::engine::OutputInfo info{};
    info.displayModeId = 0;
    info.windowed      = true;
    mDeviceWindow =
        mDeviceState->CreateDeviceWindow( static_cast<HWND>( window ), info );

    rh::engine::DescriptorSetAllocatorCreateParams dsc_all_cp{};
    std::array<rh::engine::DescriptorPoolSize, 3>  dsc_pool_sizes = {
        rh::engine::DescriptorPoolSize{
            .mType = rh::engine::DescriptorType::ROBuffer, .mCount = 32 },
        rh::engine::DescriptorPoolSize{ rh::engine::DescriptorType::ROTexture,
                                        32 },
        rh::engine::DescriptorPoolSize{ rh::engine::DescriptorType::Sampler,
                                        32 } };

    dsc_all_cp.mMaxSets         = 40;
    dsc_all_cp.mDescriptorPools = dsc_pool_sizes;
    mGlobalDescSetAlloc =
        mDeviceState->CreateDescriptorSetAllocator( dsc_all_cp );

    for ( size_t i = 0; i < mPerFrameResources.size(); i++ )
    {
        mPerFrameResources[i].mCmdBuffer = mDeviceState->CreateCommandBuffer();
        mPerFrameResources[i].mImageAquire = mDeviceState->CreateSyncPrimitive(
            rh::engine::SyncPrimitiveType::GPU );
        mPerFrameResources[i].mRenderExecute =
            mDeviceState->CreateSyncPrimitive(
                rh::engine::SyncPrimitiveType::GPU );
    }

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

    // DescriptorSetLayouts

    std::array<rh::engine::DescriptorBinding, 1> desc_set_bindings = { {
        0,                                    //  mBindingId;
        rh::engine::DescriptorType::ROBuffer, //  mDescriptorType;
        1,                                    //  mCount;
        rh::engine::ShaderStage::Pixel |
            rh::engine::ShaderStage::Vertex //  mShaderStages;
    } };
    mGlobalDescSetLayout =
        mDeviceState->CreateDescriptorSetLayout( { desc_set_bindings } );

    Im2DRendererInitParams im2d_init_params{};
    im2d_init_params.mDeviceState                = mDeviceState;
    im2d_init_params.mGlobalsDescriptorSetLayout = mGlobalDescSetLayout;
    im2d_init_params.mRenderPass                 = mRenderPass;
    im2d_init_params.mCmdBufferCount             = 16;

    m2DRenderer = std::make_unique<Im2DRenderer>( im2d_init_params );

    std::array<rh::engine::IDescriptorSetLayout *, 1> layout_array = {
        mGlobalDescSetLayout };
    rh::engine::DescriptorSetsAllocateParams all_params{};
    all_params.mLayouts = layout_array;
    mGlobalDescSet =
        mGlobalDescSetAlloc->AllocateDescriptorSets( all_params )[0];

    RenderStateBuffer buff{};
    buff.fScreenHeight = 1000;
    buff.fScreenWidth  = 2000;
    rh::engine::BufferCreateInfo rs_buff_create_info{};
    rs_buff_create_info.mSize        = sizeof( RenderStateBuffer );
    rs_buff_create_info.mUsage       = rh::engine::BufferUsage::ConstantBuffer;
    rs_buff_create_info.mFlags       = rh::engine::BufferFlags::Immutable;
    rs_buff_create_info.mInitDataPtr = &buff;
    mGlobalsBuffer = mDeviceState->CreateBuffer( rs_buff_create_info );

    rh::engine::DescriptorSetUpdateInfo desc_set_upd_info{};
    desc_set_upd_info.mSet              = mGlobalDescSet;
    desc_set_upd_info.mDescriptorType   = rh::engine::DescriptorType::ROBuffer;
    std::array update_info              = { rh::engine::BufferUpdateInfo{
        0, sizeof( RenderStateBuffer ), mGlobalsBuffer } };
    desc_set_upd_info.mBufferUpdateInfo = update_info;
    mDeviceState->UpdateDescriptorSets( desc_set_upd_info );

    std::vector<uint32_t> buffer_data( 128 * 128, 0xFFFF00FF );
    for ( size_t x = 0; x < 128; x++ )
    {
        for ( size_t y = 0; y < 128; y++ )
        {
            buffer_data[x + y * 128] =
                ( x + y ) % 2 == 0 ? 0xFFFF00FF : 0xFF0000FF;
        }
    }

    std::array image_init_buffer = { rh::engine::ImageBufferInitData{
        buffer_data.data(),
        static_cast<uint32_t>( buffer_data.size() * sizeof( uint32_t ) ), 8 } };

    rh::engine::ImageBufferCreateParams image_buffer_ci{};
    image_buffer_ci.mDimension   = rh::engine::ImageDimensions::d2D;
    image_buffer_ci.mWidth       = 128;
    image_buffer_ci.mHeight      = 128;
    image_buffer_ci.mFormat      = rh::engine::ImageBufferFormat::RGBA8;
    image_buffer_ci.mPreinitData = image_init_buffer;
    auto img_buffer = mDeviceState->CreateImageBuffer( image_buffer_ci );

    // CopyDataToImage( img_buffer, 128, 128 );

    rh::engine::ImageViewCreateInfo shader_view_ci{};
    shader_view_ci.mBuffer = img_buffer;
    shader_view_ci.mFormat = rh::engine::ImageBufferFormat::RGBA8;
    shader_view_ci.mUsage  = rh::engine::ImageViewUsage::ShaderResource;
    mImageView             = mDeviceState->CreateImageView( shader_view_ci );
    // delete img_buffer;
    return true;
}

void DisplayModeTest::CustomRender()
{
    static std::vector triangle_verticles = {
        Im2DVertex{ 0, 0, 0, 1, // pos
                    0, 0,       // uv
                    0xFFFFFFFF, // color
                    0 },
        // v0
        Im2DVertex{ 0, 720, 0, 1, // pos
                    0, 1,         // uv
                    0xFFFF0000,   // color
                    0 },
        // v1
        Im2DVertex{ 1280, 0, 0, 1, // pos
                    1, 0,
                    0xFF0000FF, // color
                    0 },
        // v2
        Im2DVertex{ 1280, 0, 0, 1, // pos
                    1, 0,
                    0xFF00FF00, // color
                    0 },
        // v3
        Im2DVertex{ 0, 720, 0, 1, // pos
                    0, 1,
                    0xFFFF00FF, // color
                    0 },
        // v4
        Im2DVertex{ 1280, 720, 0, 1, // pos
                    1, 1,
                    0xFFFF0000, // color
                    0 }
        // v5
    };
    static int current_frame = 0;
    const auto &[swapchain, invalidate_framebuffers] =
        mDeviceWindow->GetSwapchain();
    if ( invalidate_framebuffers )
    {
        for ( auto &fb : mFrameBuffer )
        {
            delete fb;
            fb = nullptr;
        }
    }
    auto cmdbuffer   = mPerFrameResources[current_frame].mCmdBuffer;
    auto render_exec = mPerFrameResources[current_frame].mRenderExecute;
    auto img_aq      = mPerFrameResources[current_frame].mImageAquire;
    if ( mPerFrameResources[current_frame].mBufferIsRecorded )
    {
        std::array exec_prim_list = { cmdbuffer->ExecutionFinishedPrimitive() };

        mDeviceState->Wait( exec_prim_list );
        mPerFrameResources[current_frame].mBufferIsRecorded = false;
    }
    auto frame = swapchain->GetAvaliableFrame( img_aq );

    gGlobalStateBuffer.fRedness = frame.mImageId % 2 ? 1.0f : 0.0f;
    mGlobalsBuffer->Update( &gGlobalStateBuffer, sizeof( RenderStateBuffer ) );

    auto record_cmd_buffer = [&]( auto record_call ) {
        cmdbuffer->BeginRecord();
        record_call();
        cmdbuffer->EndRecord();
    };

    auto record_render_pass = [&]( auto record_call ) {
        std::array                      clear_values = { rh::engine::ClearValue{
            rh::engine::ClearValueType::Color,
            rh::engine::ClearValue::ClearColor{ 0, 0, 128, 0xFF },
            {} } };
        rh::engine::RenderPassBeginInfo info{};
        info.m_pRenderPass  = mRenderPass;
        info.m_pFrameBuffer = GetFramebufferForFrame( frame );
        info.m_aClearValues = clear_values;
        cmdbuffer->BeginRenderPass( info );
        record_call();
        cmdbuffer->EndRenderPass();
    };

    record_cmd_buffer( [&]() {
        record_render_pass( [&]() {
            auto       fb_info   = GetFramebufferForFrame( frame )->GetInfo();
            std::array viewports = { rh::engine::ViewPort{
                0,                                    // float topLeftX;
                0,                                    // float topLeftY;
                static_cast<float>( fb_info.width ),  // float width;
                static_cast<float>( fb_info.height ), // float height;
                0,                                    // float minDepth;
                1.0                                   // float maxDepth;
            } };
            std::array scissors  = { rh::engine::Scissor{
                0,             // float topLeftX;
                0,             // float topLeftY;
                fb_info.width, // float width;
                fb_info.height // float height;
            } };
            cmdbuffer->SetViewports( 0, viewports );
            cmdbuffer->SetScissors( 0, scissors );
            m2DRenderer->SetImageView( mImageView );
            m2DRenderer->RecordDrawCall( triangle_verticles );

            std::array desc_sets = { mGlobalDescSet };

            rh::engine::DescriptorSetBindInfo desc_set_bind{};
            desc_set_bind.mPipelineLayout = m2DRenderer->GetLayout();
            desc_set_bind.mDescriptorSets = desc_sets;

            cmdbuffer->BindDescriptorSets( desc_set_bind );

            m2DRenderer->DrawBatch( cmdbuffer );
            m2DRenderer->FrameEnd();
        } );
    } );

    // Submit command buffer
    mDeviceState->ExecuteCommandBuffer( cmdbuffer, img_aq, render_exec );
    mPerFrameResources[current_frame].mBufferIsRecorded = true;
    swapchain->Present( frame.mImageId, render_exec );

    static bool res_switch = true;
    if ( res_switch )
    {
        mDeviceState->WaitForGPU();
        rh::engine::DisplayModeInfo dm_info{};
        mDeviceState->GetDisplayModeInfo( 25, dm_info );
        rh::engine::WindowParams params{};
        params.mWidth  = dm_info.width;
        params.mHeight = dm_info.height;
        mDeviceWindow->SetWindowParams( params );

        gGlobalStateBuffer.fScreenWidth  = dm_info.width;
        gGlobalStateBuffer.fScreenHeight = dm_info.height;
        mGlobalsBuffer->Update( &gGlobalStateBuffer,
                                sizeof( RenderStateBuffer ) );

        res_switch = false;
    }

    current_frame = ( current_frame + 1 ) % mPerFrameResources.size();
}

void DisplayModeTest::CustomShutdown()
{
    mDeviceState->WaitForGPU();
    m2DRenderer.reset();
    delete mGlobalsBuffer;
    delete mGlobalDescSet;
    delete mGlobalDescSetAlloc;
    delete mGlobalDescSetLayout;
    delete mRenderPass;
    for ( auto fb : mFrameBuffer )
        delete fb;
    for ( auto res : mPerFrameResources )
    {
        delete res.mCmdBuffer;
        delete res.mImageAquire;
        delete res.mRenderExecute;
    }
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
