#pragma once
#include "../Common/IDeviceState.h"
#include "D3D11DeviceOutputView.h"
#include "D3D11GPUAllocator.h"
#include "D3D11RenderingContext.h"

namespace rh::engine
{
class D3D11DeviceState final : public IDeviceState
{
  public:
    D3D11DeviceState();
    ~D3D11DeviceState() override;

    bool Init() override;
    bool Shutdown() override;
    bool GetAdaptersCount( unsigned int &count ) override;
    bool GetAdapterInfo( unsigned int id, String &info ) override;
    bool GetCurrentAdapter( unsigned int &id ) override;
    bool SetCurrentAdapter( unsigned int id ) override;
    bool GetOutputCount( unsigned int adapterId, unsigned int &count ) override;
    bool GetOutputInfo( unsigned int id, String &info ) override;
    bool GetCurrentOutput( unsigned int &id ) override;
    bool SetCurrentOutput( unsigned int id ) override;
    bool GetDisplayModeCount( unsigned int  outputId,
                              unsigned int &count ) override;
    bool GetDisplayModeInfo( unsigned int id, DisplayModeInfo &info ) override;
    bool GetCurrentDisplayMode( unsigned int &id ) override;
    bool SetCurrentDisplayMode( unsigned int id ) override;
    IDeviceOutputView *
                    CreateDeviceOutputView( HWND window, const OutputInfo &info ) override;
    ICommandBuffer *GetMainCommandBuffer() override;

    void InitAllocator( D3D11GPUAllocator &    allocator,
                        D3D11DeviceOutputView *output );
    bool InitRenderingContext( D3D11RenderingContext &context,
                               D3D11GPUAllocator *allocator, bool deferred );
    [[nodiscard]] D3D_FEATURE_LEVEL GetFeatureLevel() const;

    IWindow *CreateDeviceWindow( HWND window, const OutputInfo &info ) override;
    ISyncPrimitive *CreateSyncPrimitive( SyncPrimitiveType type ) override;
    IFrameBuffer *
    CreateFrameBuffer( const FrameBufferCreateParams &params ) override;
    IRenderPass *
         CreateRenderPass( const RenderPassCreateParams &params ) override;
    void ExecuteCommandBuffer( ICommandBuffer *buffer, ISyncPrimitive *waitFor,
                               ISyncPrimitive *signal ) override;
    virtual IPipeline *
             CreatePipeline( const PipelineCreateParams &params ) override;
    IShader *CreateShader( const ShaderDesc &params ) override;
    IBuffer *CreateBuffer( const BufferCreateInfo &params ) override;
    void Wait( const std::vector<ISyncPrimitive *> &primitiveList ) override;

  private:
    /// Device driver type - represents where to do the graphics work on GPU or
    /// on CPU(with different implementations)
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

    /// Device feature level - describes what feature set are avaliable to this
    /// GPU
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

    /// DXGI Factory - object used to select appropriate physical devices for
    /// rendering
    IDXGIFactory *m_pdxgiFactory = nullptr;

    /// Current physical device ID
    UINT m_uiCurrentAdapter = 0;

    /// Current monitor ID
    UINT m_uiCurrentOutput = 0;

    /// Current display Mode ID
    UINT m_uiCurrentDisplayMode = 0;

    /// Physical device list
    std::vector<IDXGIAdapter *> m_vAdapters{};

    /// Monitor list
    std::vector<IDXGIOutput *> m_vOutputs{};

    /// Display mode descriptions
    std::vector<DXGI_MODE_DESC> m_vDisplayModes{};

    std::vector<D3D_DRIVER_TYPE> m_vDriverTypes = {D3D_DRIVER_TYPE_UNKNOWN,
                                                   D3D_DRIVER_TYPE_HARDWARE};

    std::vector<D3D_FEATURE_LEVEL> m_vFeatureLevels = {
        D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,  D3D_FEATURE_LEVEL_9_1,
    };

    /// Graphical device. Used to handle all sorts of GPU memory management
    /// work.
    ID3D11Device *m_pd3dDevice = nullptr;

    /// Device context. Used to send various requests to GPU.
    ID3D11DeviceContext *m_pImmediateContext = nullptr;

    /// Debug interface. Used to validate API usage
    ID3D11Debug *m_pDebug = nullptr;

    HWND m_hWnd = nullptr;

    ICommandBuffer *mMainCmdBuffer = nullptr;
};

}; // namespace rh::engine
