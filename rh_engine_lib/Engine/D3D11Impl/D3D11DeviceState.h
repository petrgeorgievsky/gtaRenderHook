#pragma once
#include "../Common/IDeviceState.h"
#include <d3dcommon.h>

// d3d11 struct forwards:
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGIFactory;
struct IDXGIAdapter;
struct IDXGIOutput;
struct DXGI_MODE_DESC;
struct ID3D11Debug;

namespace rh::engine
{
class D3D11DeviceState : public IDeviceState
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
    ICommandBuffer *GetMainCommandBuffer() override;

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
    CreateRasterPipeline( const RasterPipelineCreateParams &params ) override;
    IShader *CreateShader( const ShaderDesc &params ) override;
    IBuffer *CreateBuffer( const BufferCreateInfo &params ) override;
    IImageBuffer *
         CreateImageBuffer( const ImageBufferCreateParams &params ) override;
    void Wait( const ArrayProxy<ISyncPrimitive *> &primitiveList ) override;
    void DispatchToGPU(
        const ArrayProxy<CommandBufferSubmitInfo> &buffers ) override;

  private:
    /// Device driver type - represents where to do the graphics work on GPU or
    /// on CPU(with different implementations)
    D3D_DRIVER_TYPE m_driverType;

    /// Device feature level - describes what feature set are avaliable to this
    /// GPU
    D3D_FEATURE_LEVEL m_featureLevel;

    /// DXGI Factory - object used to select appropriate physical devices for
    /// rendering
    IDXGIFactory *m_pdxgiFactory = nullptr;

    /// Current physical device ID
    uint32_t m_uiCurrentAdapter = 0;

    /// Current monitor ID
    uint32_t m_uiCurrentOutput = 0;

    /// Current display Mode ID
    uint32_t m_uiCurrentDisplayMode = 0;

    /// Physical device list
    std::vector<IDXGIAdapter *> m_vAdapters{};

    /// Monitor list
    std::vector<IDXGIOutput *> m_vOutputs{};

    /// Display mode descriptions
    std::vector<DXGI_MODE_DESC> m_vDisplayModes{};

    std::vector<D3D_DRIVER_TYPE> m_vDriverTypes;

    std::vector<D3D_FEATURE_LEVEL> m_vFeatureLevels;

    /// Graphical device. Used to handle all sorts of GPU memory management
    /// work.
    ID3D11Device *m_pd3dDevice = nullptr;

    /// Device context. Used to send various requests to GPU.
    ID3D11DeviceContext *m_pImmediateContext = nullptr;

    /// Debug interface. Used to validate API usage
    ID3D11Debug *m_pDebug = nullptr;

    // HWND m_hWnd = nullptr;

    ICommandBuffer *mMainCmdBuffer = nullptr;

    virtual IDescriptorSetLayout *CreateDescriptorSetLayout(
        const DescriptorSetLayoutCreateParams &params ) override;

    virtual IPipelineLayout *
    CreatePipelineLayout( const PipelineLayoutCreateParams &params ) override;

    virtual void WaitForGPU() override;

    virtual IDescriptorSetAllocator *CreateDescriptorSetAllocator(
        const DescriptorSetAllocatorCreateParams &params ) override;

    virtual void
    UpdateDescriptorSets( const DescriptorSetUpdateInfo &params ) override;

    virtual ISampler *CreateSampler( const SamplerDesc &params ) override;

    virtual IImageView *
    CreateImageView( const ImageViewCreateInfo &params ) override;

    virtual ICommandBuffer *CreateCommandBuffer() override;
};

} // namespace rh::engine
