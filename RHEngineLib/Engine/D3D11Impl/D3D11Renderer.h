#pragma once
#include "../IRenderer.h"
#include "RenderStateCache/D3D11RenderStateCache.h"
#include "../../stdafx.h"
#include "../Definitions.h"
#include "../../Common/SmartCOMPtr.h"

namespace RHEngine
{
	class D3D11Renderer : public IRenderer
	{
	public:
		D3D11Renderer(HWND window, HINSTANCE inst);
		~D3D11Renderer();
		virtual bool InitDevice() override;
		virtual bool ShutdownDevice() override;
		virtual bool GetAdaptersCount(int &) override;
		virtual bool GetAdapterInfo(unsigned int n, std::wstring &) override;
		virtual bool GetOutputCount(unsigned int adapterId, int &) override;
		virtual bool GetOutputInfo(unsigned int n, std::wstring &) override;
		virtual bool SetCurrentOutput(unsigned int id) override;
		virtual bool SetCurrentAdapter(unsigned int n) override;
		virtual bool GetDisplayModeCount(unsigned int outputId, int &) override;
		virtual bool SetCurrentDisplayMode(unsigned int id) override;
		virtual bool GetDisplayModeInfo(unsigned int id, DisplayModeInfo &) override;
		virtual bool GetCurrentAdapter(int &) override;
		virtual bool GetCurrentOutput(int &) override;
		virtual bool GetCurrentDisplayMode(int &) override;
		virtual bool Present(void* image) override;
		virtual void * AllocateImageBuffer(unsigned int width, unsigned int height, RHImageBufferType type) override;
		virtual bool FreeImageBuffer(void * buffer, RHImageBufferType type) override;
		virtual bool BindImageBuffers(RHImageBindType bindType, const std::unordered_map<int, void*>&  buffers) override;
		virtual bool ClearImageBuffer(RHImageClearType clearType, void* buffer, const float clearColor[4]) override;
	private:
		/// Device driver type - represents where to do the graphics work on GPU or on CPU(with different implementations)
		D3D_DRIVER_TYPE         m_driverType = D3D_DRIVER_TYPE_NULL;
		/// Device feature level - describes what feature set are avaliable to this GPU
		D3D_FEATURE_LEVEL       m_featureLevel = D3D_FEATURE_LEVEL_11_0;
		/// DXGI Factory - object used to select appropriate physical devices for rendering
		IDXGIFactory*			m_pdxgiFactory = nullptr;
		/// Graphical device. Used to handle all sorts of GPU memory management work.
		ID3D11Device*           m_pd3dDevice = nullptr;
		/// Device context. Used to send various requests to GPU.
		ID3D11DeviceContext*    m_pImmediateContext = nullptr;
		/// Back-buffer swap-chain. Stores 2 or more image buffers, 
		/// one being processed in this frame, and other from previous frames.
		IDXGISwapChain*         m_pSwapChain = nullptr;
		/// Debug interface. Used to validate API usage 
		ID3D11Debug*			m_pDebug = nullptr;
		/// Current physical device ID
		UINT					m_uiCurrentAdapter = 0, 
		/// Current monitor ID
								m_uiCurrentOutput = 0, 
		/// Current display Mode ID
								m_uiCurrentDisplayMode = 0;
		/// Physical device list
		std::vector <IDXGIAdapter*>	m_vAdapters{};
		/// Monitor list
		std::vector <IDXGIOutput*>	m_vOutputs{};
		/// Display mode descriptions
		std::vector <DXGI_MODE_DESC>	m_vDisplayModes{};
		D3D11RenderStateCache *m_pRenderStateCache = nullptr;
		
		virtual bool BeginCommandList(void * cmdList) override;
		virtual bool EndCommandList(void * cmdList) override;
		virtual bool RequestSwapChainImage(void * frameBuffer) override;
		virtual bool PresentSwapChainImage(void * frameBuffer) override;
	};
};