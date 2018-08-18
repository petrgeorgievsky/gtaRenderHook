#pragma once
#include "../../../stdafx.h"
namespace RHEngine {

	/**
	 * @brief Interface for video memory objects that can be viewed as render target
	 * 
	 */
	class ID3D11RenderTargetViewable {
	public:
		/**
		 * @brief Get the Render Target View object
		 * 
		 * @return ID3D11RenderTargetView* - pointer to created render target view
		 */
		virtual ID3D11RenderTargetView* GetRenderTargetView() const = 0;
	};

	/**
	 * @brief Interface for video memory objects that can be viewed as depth stencil buffer
	 * 
	 */
	class ID3D11DepthStencilViewable {
	public:
		/**
		 * @brief Get the Depth Stencil View object
		 * 
		 * @return ID3D11DepthStencilView* - pointer to created depth stencil view
		 */
		virtual ID3D11DepthStencilView* GetDepthStencilView() const = 0;
	};

};