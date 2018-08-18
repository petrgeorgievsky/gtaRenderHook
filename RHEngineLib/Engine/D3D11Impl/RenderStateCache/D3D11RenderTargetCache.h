#pragma once
#include "../../../stdafx.h"
#include "../../../Common/IStateCacheObject.h"
#include "../ImageBuffers/ImageBuffer.h"
namespace RHEngine 
{
	/**
	 * @brief Render targets state cache object.
	 * 
	 * This object is used to hold pointers to cached render targets and depth stencil buffer.
	 */
	class D3D11RenderTargetCache: public IStateCacheObject
	{
	public:
		/**
		 * @brief Construct a new D3D11RenderTargetCache object
		 * 
		 */
		D3D11RenderTargetCache() noexcept;

		/**
		 * @brief Set the Render Targets and DepthStencil buffer to Output-Merger stage of graphics pipeline
		 * 
		 * @param renderTargets - list of render target viewables mapped to slot number in OM stage
		 * @param depthStencilView - depth stencil viewable object
		 */
		void SetRenderTargets(const std::unordered_map<int, void *>& renderTargets, void * depthStencilView);

		/**
		 * @brief Set the Render Targets to Output-Merger stage of graphics pipeline
		 * 
		 * @param renderTargets - list of render target viewables mapped to slot number in OM stage
		 */
		void SetRenderTargets(const std::unordered_map<int, void *>& renderTargets);

		/**
		 * @brief Set the Depth Stencil buffer to Output-Merger stage of graphics pipeline
		 * 
		 * @param depthStencilView - depth stencil viewable object
		 */
		void SetDepthStencilTarget(void * depthStencilView);

		/**
		 * @brief Binds render targets to OM stage of graphics pipeline
		 * 
		 */
		virtual void OnFlush(void*) override;
	private:
		ID3D11RenderTargetViewable * m_aRenderTargetViews[8];
		ID3D11DepthStencilViewable * m_pDepthStencilView=nullptr;
	};
};