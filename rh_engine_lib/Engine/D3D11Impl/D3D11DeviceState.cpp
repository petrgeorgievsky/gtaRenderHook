#include "D3D11DeviceState.h"
#include "D3D11Buffer.h"
#include "D3D11CommandBuffer.h"
#include "D3D11Common.h"
#include "D3D11DescriptorSet.h"
#include "D3D11DescriptorSetAllocator.h"
#include "D3D11DescriptorSetLayout.h"
#include "D3D11Framebuffer.h"
#include "D3D11ImageBuffer.h"
#include "D3D11ImageView.h"
#include "D3D11Pipeline.h"
#include "D3D11RenderPass.h"
#include "D3D11Sampler.h"
#include "D3D11Shader.h"
#include "D3D11Window.h"
#include <cassert>
#include <d3d11.h>

using namespace rh::engine;

D3D11DeviceState::D3D11DeviceState()
{
    m_driverType     = D3D_DRIVER_TYPE_NULL;
    m_featureLevel   = D3D_FEATURE_LEVEL_11_0;
    m_vFeatureLevels = {
        D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,  D3D_FEATURE_LEVEL_9_1,
    };
    m_vDriverTypes = { D3D_DRIVER_TYPE_UNKNOWN, D3D_DRIVER_TYPE_HARDWARE };
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

    for ( auto &output : m_vOutputs )
    {
        output->Release();
        output = nullptr;
    }

    for ( auto &adapter : m_vAdapters )
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

    // auto currentAdapterMode = m_vDisplayModes[m_uiCurrentDisplayMode];
    auto currentAdapter = m_vAdapters[m_uiCurrentAdapter];

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

    std::vector<DXGI_MODE_DESC> modeDescriptions{ modeCount };

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

IPipeline *D3D11DeviceState::CreateRasterPipeline(
    const RasterPipelineCreateParams &params )
{
    D3D11GraphicsPipelineCreateInfo ci{ params, m_pd3dDevice };
    return new D3D11Pipeline( ci );
}

void D3D11DeviceState::ExecuteCommandBuffer( ICommandBuffer *cmdBuffer,
                                             ISyncPrimitive *,
                                             ISyncPrimitive * )
{
    auto cmd_buffer = dynamic_cast<D3D11CommandBuffer *>( cmdBuffer );
    m_pImmediateContext->ExecuteCommandList( cmd_buffer->GetCmdList(), false );
}

void D3D11DeviceState::Wait( const ArrayProxy<ISyncPrimitive *> & ) {}

IDescriptorSetLayout *rh::engine::D3D11DeviceState::CreateDescriptorSetLayout(
    const DescriptorSetLayoutCreateParams &params )
{
    return new D3D11DescriptorSetLayout( params );
}

IPipelineLayout *rh::engine::D3D11DeviceState::CreatePipelineLayout(
    const PipelineLayoutCreateParams & /*params*/ )
{
    return nullptr;
}

void rh::engine::D3D11DeviceState::WaitForGPU() {}

IDescriptorSetAllocator *
rh::engine::D3D11DeviceState::CreateDescriptorSetAllocator(
    const DescriptorSetAllocatorCreateParams &params )
{
    return new D3D11DescriptorSetAllocator( params );
}

void rh::engine::D3D11DeviceState::UpdateDescriptorSets(
    const DescriptorSetUpdateInfo &params )
{
    if ( params.mDescriptorType == DescriptorType::ROBuffer )
    {
        static_cast<D3D11DescriptorSet *>( params.mSet )
            ->UpdateDescriptorBinding( params.mBinding,
                                       static_cast<D3D11Buffer *>(
                                           params.mBufferUpdateInfo[0].mBuffer )
                                           ->GetImpl() );
    }
    else if ( params.mDescriptorType == DescriptorType::Sampler )
    {
        static_cast<D3D11DescriptorSet *>( params.mSet )
            ->UpdateDescriptorBinding( params.mBinding,
                                       static_cast<D3D11Sampler *>(
                                           params.mImageUpdateInfo[0].mSampler )
                                           ->GetImpl() );
    }
    else if ( params.mDescriptorType == DescriptorType::ROTexture )
    {
        static_cast<D3D11DescriptorSet *>( params.mSet )
            ->UpdateDescriptorBinding( params.mBinding,
                                       static_cast<D3D11ImageView *>(
                                           params.mImageUpdateInfo[0].mView )
                                           ->GetSRV() );
    }
}

ISampler *
rh::engine::D3D11DeviceState::CreateSampler( const SamplerDesc &params )
{
    D3D11SamplerCreateParams create_params{ params, m_pd3dDevice };
    return new D3D11Sampler( create_params );
}

IImageView *rh::engine::D3D11DeviceState::CreateImageView(
    const ImageViewCreateInfo &params )
{
    D3D11ImageViewCreateInfo create_params{ params, m_pd3dDevice };
    return new D3D11ImageView( create_params );
}

ICommandBuffer *rh::engine::D3D11DeviceState::CreateCommandBuffer()
{
    D3D11CommandBufferCreateParams cmd_buff_cparams{};
    cmd_buff_cparams.mDevice = m_pd3dDevice;
    return new D3D11CommandBuffer( cmd_buff_cparams );
}

IShader *D3D11DeviceState::CreateShader( const ShaderDesc &params )
{
    D3D11ShaderDesc shader_desc{};
    shader_desc.mDevice = m_pd3dDevice;
    // TODO: FIX
    shader_desc.mShaderModel = "5_0";
    shader_desc.mDesc        = params;

    return new D3D11Shader( shader_desc );
}

IBuffer *D3D11DeviceState::CreateBuffer( const BufferCreateInfo &params )
{
    D3D11BufferCreateInfo create_info{ params, m_pd3dDevice,
                                       m_pImmediateContext };

    return new D3D11Buffer( create_info );
}

IImageBuffer *rh::engine::D3D11DeviceState::CreateImageBuffer(
    const ImageBufferCreateParams &params )
{
    D3D11ImageBufferCreateParams create_params{ params, m_pd3dDevice };
    return new D3D11ImageBuffer( create_params );
}
void D3D11DeviceState::DispatchToGPU(
    const ArrayProxy<CommandBufferSubmitInfo> &buffers )
{
    for ( auto &dispatch : buffers )
    {
        auto cmd_buffer =
            dynamic_cast<D3D11CommandBuffer *>( dispatch.mCmdBuffer );
        m_pImmediateContext->ExecuteCommandList( cmd_buffer->GetCmdList(),
                                                 false );
    }
}
