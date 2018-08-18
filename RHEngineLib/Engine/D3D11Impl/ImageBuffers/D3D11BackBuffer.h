#pragma once
#include "ImageBuffer.h"
#include "../../../Common/SmartCOMPtr.h"
namespace RHEngine {
	/**
	 * @brief D3D11 Back-buffer implementation, 
	 * holds a RenderTarget view of a swap-chain back buffer
	 * 
	 */
	class D3D11BackBuffer:
		public ID3D11RenderTargetViewable
	{
	public:

		/**
		 * @brief Construct a new D3D11BackBuffer object
		 * 
		 * @param device - logical device used to allocate memory for graphics resources
		 * @param swapChain - swap-chain of this back-buffer 
		 */
		D3D11BackBuffer(ID3D11Device* device, IDXGISwapChain* swapChain);

		/**
		 * @brief Destroy the D3D11BackBuffer object
		 * 
		 */
		~D3D11BackBuffer();

		/**
		 * @brief Get the Render Target View object
		 * 
		 * @return ID3D11RenderTargetView* - view of render target
		 */
		virtual ID3D11RenderTargetView * GetRenderTargetView() const override;

	private:

		ID3D11RenderTargetView * m_pRenderTargetView = nullptr;
	};
};