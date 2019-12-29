#include "D3D11DeviceState.h"
#include "D3D11CommandBuffer.h"
#include "D3D11Common.h"
#include "D3D11DeviceOutputView.h"
#include "D3D11Framebuffer.h"
#include "D3D11RenderPass.h"
#include "D3D11Window.h"
#include <common.h>

using namespace rh::engine;

D3D11DeviceState::D3D11DeviceState()
{
    IDXGIAdapter *adapterPtr = nullptr;

    // dxgi factory initialization
    if ( !CALL_D3D_API(
             CreateDXGIFactory( __uuidof( IDXGIFactory ),
                                reinterpret_cast<void **>( &m_pdxgiFactory ) ),
             TEXT( "Failed to create DXGI Factory." ) ) )
        return;

    // Init adapter list.
    for ( UINT i = 0; m_pdxgiFactory->EnumAdapters( i, &adapterPtr ) !=
                      DXGI_ERROR_NOT_FOUND;
          ++i )
        m_vAdapters.push_back( adapterPtr );

    SetCurrentAdapter( 0 );
    SetCurrentOutput( 0 );
}

D3D11DeviceState::~D3D11DeviceState()
{
    rh::debug::DebugLogger::Log( "D3D11DeviceState destructor..." );

    for ( auto output : m_vOutputs )
    {
        output->Release();
        output = nullptr;
    }

    for ( auto adapter : m_vAdapters )
    {
        adapter->Release();
        adapter = nullptr;
    }
}

bool D3D11DeviceState::Init()
{
    rh::debug::DebugLogger::Log( "D3D11DeviceState initialization..." );

    HRESULT hr = S_OK;

    // return if something is wrong, e.g. we have set some wrong parameters.
    if ( m_vDisplayModes.empty() ||
         m_uiCurrentDisplayMode >= m_vDisplayModes.size() )
        return false;

    auto currentAdapterMode = m_vDisplayModes[m_uiCurrentDisplayMode];
    auto currentAdapter     = m_vAdapters[m_uiCurrentAdapter];

    // initialize device creation flags
    // TODO: add ability to set some custom flags
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    // createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    size_t numFeatureLevels = m_vFeatureLevels.size();

    // loop over each driver type to find supported one
    for ( const auto &driverType : m_vDriverTypes )
    {
        m_driverType = driverType;
        hr           = D3D11CreateDevice(
            m_driverType == D3D_DRIVER_TYPE_HARDWARE ? nullptr : currentAdapter,
            m_driverType, nullptr, createDeviceFlags, m_vFeatureLevels.data(),
            static_cast<UINT>( numFeatureLevels ), D3D11_SDK_VERSION,
            &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext );

        // if we failed to create device with invalid arg error,
        // probably it's because current feature level is not supported by GPU,
        // so we will try to find supported feature level set
        if ( hr == E_INVALIDARG )
        {
            // try to find supported feature level by iterating over whole set
            // and removing one feature level after another until success
            for ( size_t i = 1; i < numFeatureLevels - 1; i++ )
            {
                hr = D3D11CreateDevice(
                    m_driverType == D3D_DRIVER_TYPE_HARDWARE ? nullptr
                                                             : currentAdapter,
                    m_driverType, nullptr, createDeviceFlags,
                    &m_vFeatureLevels[i],
                    static_cast<UINT>( numFeatureLevels - i ),
                    D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel,
                    &m_pImmediateContext );
                if ( SUCCEEDED( hr ) )
                    break;
            }
        }

        if ( SUCCEEDED( hr ) )
            break;
    }

    rh::debug::DebugLogger::Log( TEXT( "D3D11CreateDevice params:" ) );
    rh::debug::DebugLogger::Log( TEXT( "\tDriverType:" ) +
                                 ToRHString( std::to_string( m_driverType ) ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tFeatureLevel:" ) +
        ToRHString( std::to_string( m_featureLevel ) ) );

    if ( !CALL_D3D_API(
             hr, TEXT( "Failed to create device, invalid input arguments!" ) ) )
        return false;

    rh::debug::DebugLogger::Log( TEXT( "D3D11 Device created" ) );

    // Query debug interface
    m_pd3dDevice->QueryInterface( __uuidof( ID3D11Debug ),
                                  reinterpret_cast<void **>( &m_pDebug ) );

    D3D11CommandBufferCreateParams create_params{};
    create_params.mDevice = m_pd3dDevice;
    mMainCmdBuffer        = new D3D11CommandBuffer( create_params );
    return true;
}

bool D3D11DeviceState::Shutdown()
{
    delete mMainCmdBuffer;
    mMainCmdBuffer = nullptr;
    if ( m_pImmediateContext )
    {
        m_pImmediateContext->ClearState();
        m_pImmediateContext->Flush();
        m_pImmediateContext = nullptr;
    }

    if ( m_pd3dDevice )
    {
        m_pd3dDevice->Release();
        m_pd3dDevice = nullptr;
    }

    if ( m_pDebug )
    {
        m_pDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL |
                                           D3D11_RLDO_SUMMARY );
        m_pDebug->Release();
        m_pDebug = nullptr;
    }

    return true;
}

bool D3D11DeviceState::GetAdaptersCount( unsigned int &count )
{
    if ( !m_vAdapters.empty() )
    {
        assert( m_vAdapters.size() < UINT_MAX );

        count = static_cast<unsigned int>( m_vAdapters.size() );

        return true;
    }

    return false;
}

bool D3D11DeviceState::GetAdapterInfo( unsigned int        id,
                                       rh::engine::String &info )
{
    if ( id >= m_vAdapters.size() )
        return false;

    DXGI_ADAPTER_DESC desc;

    if ( !CALL_D3D_API( m_vAdapters[id]->GetDesc( &desc ),
                        TEXT( "Failed to get adapter description!" ) ) )
        return false;

    info = ToRHString( desc.Description );

    return true;
}

bool D3D11DeviceState::GetCurrentAdapter( unsigned int &id )
{
    if ( m_vAdapters.empty() || m_uiCurrentAdapter >= m_vAdapters.size() )
        return false;

    id = m_uiCurrentAdapter;

    return true;
}

bool D3D11DeviceState::SetCurrentAdapter( unsigned int id )
{
    IDXGIOutput *output;

    if ( id >= m_vAdapters.size() )
        return false;

    m_uiCurrentAdapter = id;
    m_uiCurrentOutput  = 0;

    // Clear last output list
    for ( auto outputPtr : m_vOutputs )
        outputPtr->Release();

    m_vOutputs.clear();

    // Init output list.
    for ( UINT i = 0; m_vAdapters[m_uiCurrentAdapter]->EnumOutputs(
                          i, &output ) != DXGI_ERROR_NOT_FOUND;
          ++i )
        m_vOutputs.push_back( output );

    m_vDisplayModes.clear();

    // If no outputs found - something is wrong
    return !m_vOutputs.empty();
}

bool D3D11DeviceState::GetOutputCount( unsigned int /*adapterId*/,
                                       unsigned int &count )
{
    // TODO: Consider adapterId
    count = static_cast<unsigned int>( m_vOutputs.size() );

    return true;
}

bool D3D11DeviceState::GetOutputInfo( unsigned int id, String &info )
{
    if ( id >= m_vOutputs.size() )
        return false;

    DXGI_OUTPUT_DESC desc;

    if ( !CALL_D3D_API( m_vOutputs[id]->GetDesc( &desc ),
                        TEXT( "Failed to get output description!" ) ) )
        return false;

    info = ToRHString( desc.DeviceName );

    return true;
}

bool D3D11DeviceState::GetCurrentOutput( unsigned int &id )
{
    if ( m_vOutputs.empty() || m_uiCurrentOutput >= m_vOutputs.size() )
        return false;

    id = m_uiCurrentOutput;

    return true;
}

bool D3D11DeviceState::SetCurrentOutput( unsigned int id )
{
    if ( id >= m_vOutputs.size() )
        return false;

    m_uiCurrentOutput = id;

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // retrieve avaliable display mode count.
    UINT modeCount = 0;

    if ( !CALL_D3D_API( m_vOutputs[m_uiCurrentOutput]->GetDisplayModeList(
                            format, 0, &modeCount, nullptr ),
                        TEXT( "Failed to get display mode count." ) ) )
    {
        return false;
    }

    std::vector<DXGI_MODE_DESC> modeDescriptions{modeCount};

    // get display mode list
    if ( !CALL_D3D_API( m_vOutputs[m_uiCurrentOutput]->GetDisplayModeList(
                            format, 0, &modeCount, modeDescriptions.data() ),
                        TEXT( "Failed to retrieve display mode list." ) ) )
    {
        return false;
    }

    m_vDisplayModes.reserve( modeCount );
    // populate adapter mode list
    for ( auto &modeDescription : modeDescriptions )
        if ( modeDescription.RefreshRate.Denominator != 0 )
            m_vDisplayModes.emplace_back( modeDescription );

    return true;
}

bool D3D11DeviceState::GetDisplayModeCount( unsigned int  outputId,
                                            unsigned int &count )
{
    if ( outputId >= m_vOutputs.size() )
        return false;

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // retrieve avaliable display mode count.
    UINT modeCount = 0;

    if ( !CALL_D3D_API( m_vOutputs[outputId]->GetDisplayModeList(
                            format, 0, &modeCount, nullptr ),
                        TEXT( "Failed to get display mode count." ) ) )
    {
        return false;
    }

    count = modeCount;

    return true;
}

bool D3D11DeviceState::GetDisplayModeInfo( unsigned int     id,
                                           DisplayModeInfo &info )
{
    if ( id >= m_vDisplayModes.size() )
        return false;

    info.width       = m_vDisplayModes[id].Width;
    info.height      = m_vDisplayModes[id].Height;
    info.refreshRate = m_vDisplayModes[id].RefreshRate.Denominator == 0
                           ? 0
                           : m_vDisplayModes[id].RefreshRate.Numerator /
                                 m_vDisplayModes[id].RefreshRate.Denominator;

    return true;
}

bool D3D11DeviceState::GetCurrentDisplayMode( unsigned int &id )
{
    if ( m_vDisplayModes.empty() ||
         m_uiCurrentDisplayMode >= m_vDisplayModes.size() )
        return false;

    id = m_uiCurrentDisplayMode;

    return true;
}

bool D3D11DeviceState::SetCurrentDisplayMode( unsigned int id )
{
    if ( m_vDisplayModes.empty() || id >= m_vDisplayModes.size() )
        return false;

    m_uiCurrentDisplayMode = id;
    return true;
}

IDeviceOutputView *
D3D11DeviceState::CreateDeviceOutputView( HWND window, const OutputInfo &info )
{
    m_hWnd           = window;
    auto displayMode = m_vDisplayModes[info.displayModeId];
    // Create swap chain for device.
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount            = 1;
    sd.BufferDesc.Width       = displayMode.Width;
    sd.BufferDesc.Height      = displayMode.Height;
    sd.BufferDesc.Format      = displayMode.Format;
    sd.BufferDesc.RefreshRate = displayMode.RefreshRate;
    sd.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow           = window;
    sd.SampleDesc.Count       = 1;
    sd.SampleDesc.Quality     = 0;
    sd.Windowed               = info.windowed;
    sd.Flags = info.windowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

    rh::debug::DebugLogger::Log( TEXT( "Swap-Chain params:" ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tBufferDesc.Width:" ) +
        ToRHString( std::to_string( sd.BufferDesc.Width ) ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tBufferDesc.Height:" ) +
        ToRHString( std::to_string( sd.BufferDesc.Height ) ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tBufferDesc.Format:" ) +
        ToRHString( std::to_string( sd.BufferDesc.Format ) ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tBufferDesc.RefreshRate.Numerator:" ) +
        ToRHString( std::to_string( sd.BufferDesc.RefreshRate.Numerator ) ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tBufferDesc.RefreshRate.Denominator:" ) +
        ToRHString( std::to_string( sd.BufferDesc.RefreshRate.Denominator ) ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tBufferDesc.OutputWindow:" ) +
        ToRHString(
            std::to_string( reinterpret_cast<uint64_t>( sd.OutputWindow ) ) ) );
    rh::debug::DebugLogger::Log(
        TEXT( "\tBufferDesc.Windowed:" ) +
        ToRHString( std::to_string( info.windowed ) ) );
    IDXGISwapChain *swapChainPtr = nullptr;

    if ( !CALL_D3D_API( m_pdxgiFactory->CreateSwapChain( m_pd3dDevice, &sd,
                                                         &swapChainPtr ),
                        TEXT( "Failed to create swap chain using DX11 API" ) ) )
        return nullptr;

    return new D3D11DeviceOutputView( window, swapChainPtr );
}

void rh::engine::D3D11DeviceState::InitAllocator(
    D3D11GPUAllocator &allocator, D3D11DeviceOutputView *output )
{
    allocator.Init( m_pd3dDevice, output );
}

bool rh::engine::D3D11DeviceState::InitRenderingContext(
    D3D11RenderingContext &context, D3D11GPUAllocator *allocator,
    bool /*deferred*/ )
{
    // TODO: Add deferred context creation
    context.Init( m_pImmediateContext, allocator );
    return true;
}

D3D_FEATURE_LEVEL rh::engine::D3D11DeviceState::GetFeatureLevel() const
{
    return m_featureLevel;
}

ICommandBuffer *rh::engine::D3D11DeviceState::GetMainCommandBuffer()
{
    return mMainCmdBuffer;
}

IWindow *D3D11DeviceState::CreateDeviceWindow( HWND              hwnd,
                                               const OutputInfo &info )
{
    auto                    display_mode = m_vDisplayModes[info.displayModeId];
    D3D11WindowCreateParams create_params{};
    create_params.mWindowParams.mWidth      = display_mode.Width;
    create_params.mWindowParams.mHeight     = display_mode.Height;
    create_params.mWindowParams.mFullscreen = !info.windowed;
    create_params.mWndHandle                = hwnd;
    create_params.mDevice                   = m_pd3dDevice;
    create_params.mDXGIFactory              = m_pdxgiFactory;
    return new D3D11Window( create_params );
}

ISyncPrimitive *D3D11DeviceState::CreateSyncPrimitive( SyncPrimitiveType )
{
    return nullptr;
}

IFrameBuffer *D3D11DeviceState::CreateFrameBuffer(
    const FrameBufferCreateParams &create_params )
{
    return new D3D11Framebuffer( create_params );
}

IRenderPass *D3D11DeviceState::CreateRenderPass(
    const RenderPassCreateParams &create_params )
{
    return new D3D11RenderPass( create_params );
}

IPipeline *
D3D11DeviceState::CreatePipeline( const PipelineCreateParams &params )
{
    return nullptr;
}

void D3D11DeviceState::ExecuteCommandBuffer( ICommandBuffer *cmdBuffer,
                                             ISyncPrimitive *,
                                             ISyncPrimitive * )
{
    auto cmd_buffer = dynamic_cast<D3D11CommandBuffer *>( cmdBuffer );
    m_pImmediateContext->ExecuteCommandList( cmd_buffer->GetCmdList(), false );
}

void D3D11DeviceState::Wait( const std::vector<ISyncPrimitive *> & ) {}

IShader *D3D11DeviceState::CreateShader( const ShaderDesc &params )
{
    return nullptr;
}

IBuffer *D3D11DeviceState::CreateBuffer( const BufferCreateInfo &params )
{
    return nullptr;
}