#pragma once
#include "../IRenderer.h"
#include "../../stdafx.h"
#include "../Definitions.h"
namespace RHEngine
{
	class D3D12Renderer : public IRenderer
	{
	public:
		D3D12Renderer(HWND window, HINSTANCE inst);
		~D3D12Renderer();
		virtual bool InitDevice() override;
		virtual bool ShutdownDevice() override;
		virtual bool GetAdaptersCount(int &) override;
		virtual bool GetAdapterInfo(unsigned int n, std::wstring &) override;
		virtual bool GetOutputCount(unsigned int adapterId, int &) override;
		virtual bool SetCurrentOutput(unsigned int id) override;
		virtual bool GetOutputInfo(unsigned int n, std::wstring &) override;
		virtual bool SetCurrentAdapter(unsigned int n) override;
		virtual bool GetDisplayModeCount(unsigned int outputId, int &) override;
		virtual bool SetCurrentDisplayMode(unsigned int id) override;
		virtual bool GetDisplayModeInfo(unsigned int id, DisplayModeInfo &) override;
		virtual bool Present(void*) override;
	private:
		D3D_DRIVER_TYPE         m_driverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL       m_featureLevel = D3D_FEATURE_LEVEL_11_0;
		IDXGIFactory*			m_pdxgiFactory = nullptr;
		ID3D12Device*           m_pd3dDevice = nullptr;
		UINT					m_uiCurrentAdapter = 0,
			m_uiCurrentOutput = 0,
			m_uiCurrentAdapterMode = 0;
		std::vector <IDXGIAdapter*>	m_vAdapters{};
		std::vector <IDXGIOutput*>	m_vOutputs{};
		std::vector <DXGI_MODE_DESC>	m_vAdapterModes{};

		// Унаследовано через IRenderer
		virtual bool GetCurrentAdapter(int &) override;
		virtual bool GetCurrentOutput(int &) override;
		virtual bool GetCurrentDisplayMode(int &) override;

		// Унаследовано через IRenderer
		virtual void * AllocateImageBuffer(unsigned int width, unsigned int height, RHImageBufferType type) override;
		virtual bool FreeImageBuffer(void * buffer, RHImageBufferType type) override;

		// Унаследовано через IRenderer
		virtual bool BindImageBuffers(RHImageBindType bindType, const std::unordered_map<int, void*>& buffers) override;

		// Унаследовано через IRenderer
		virtual bool ClearImageBuffer(RHImageClearType clearType, void* buffer, const float clearColor[4]) override;

		// Унаследовано через IRenderer
		virtual bool BeginCommandList(void * cmdList) override;
		virtual bool EndCommandList(void * cmdList) override;

		// Унаследовано через IRenderer
		virtual bool RequestSwapChainImage(void * frameBuffer) override;
		virtual bool PresentSwapChainImage(void * frameBuffer) override;
	};
};