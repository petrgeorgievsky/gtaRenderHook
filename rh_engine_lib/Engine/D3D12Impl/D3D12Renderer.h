#pragma once
#include "Engine/Common/IStateCacheObject.h"
#include "Engine/Definitions.h"
#include "Engine/IRenderer.h"

namespace rh::engine {
class D3D12Renderer : public IRenderer
{
public:
    D3D12Renderer( HWND window, HINSTANCE inst );
    ~D3D12Renderer() override;
    virtual bool InitDevice() override;
    virtual bool ShutdownDevice() override;
    virtual bool GetAdaptersCount( unsigned int & ) override;
    virtual bool GetAdapterInfo( unsigned int n, rh::engine::String & ) override;
    virtual bool GetOutputCount( unsigned int adapterId, int & ) override;
    virtual bool SetCurrentOutput( unsigned int id ) override;
    virtual bool GetOutputInfo( unsigned int n, std::wstring & ) override;
    virtual bool SetCurrentAdapter( unsigned int n ) override;
    virtual bool GetDisplayModeCount( unsigned int outputId, int & ) override;
    virtual bool SetCurrentDisplayMode( unsigned int id ) override;
    virtual bool GetDisplayModeInfo( unsigned int id, DisplayModeInfo & ) override;
    virtual bool Present( void * ) override;
    virtual void *GetCurrentDevice() override;
    virtual void *GetCurrentContext() override;
    virtual void BindViewPorts( const std::vector<ViewPort> &viewports ) override;

private:
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
    IDXGIFactory *m_pdxgiFactory = nullptr;
    ID3D12Device *m_pd3dDevice = nullptr;
    UINT m_uiCurrentAdapter = 0, m_uiCurrentOutput = 0, m_uiCurrentAdapterMode = 0;
    std::vector<IDXGIAdapter *> m_vAdapters{};
    std::vector<IDXGIOutput *> m_vOutputs{};
    std::vector<DXGI_MODE_DESC> m_vAdapterModes{};

    virtual bool GetCurrentAdapter( int & ) override;
    virtual bool GetCurrentOutput( int & ) override;
    virtual bool GetCurrentDisplayMode( int & ) override;
    virtual void *AllocateImageBuffer( const ImageBufferInfo &info ) override;
    virtual bool FreeImageBuffer( void *buffer, ImageBufferType type ) override;
    virtual bool BindImageBuffers( ImageBindType bindType,
                                   const std::vector<IndexPtrPair> &buffers ) override;
    virtual bool ClearImageBuffer( ImageClearType clearType,
                                   void *buffer,
                                   const std::array<float, 4> &clearColor ) override;
    virtual bool BeginCommandList( void *cmdList ) override;
    virtual bool EndCommandList( void *cmdList ) override;
    virtual bool RequestSwapChainImage( void *frameBuffer ) override;
    virtual bool PresentSwapChainImage( void *frameBuffer ) override;
    virtual void FlushCache() override;

    virtual void InitImGUI() override;
    virtual void ImGUIStartFrame() override;
    virtual IGPUAllocator *GetGPUAllocator() override;

    virtual void ImGUIRender() override;
    void ShutdownImGUI() override;
};
} // namespace rh::engine
