#pragma once
#include "D3D11RenderTargetCache.h"
namespace RHEngine 
{
	/**
	 * @brief D3D11 render state cache class, 
	 * used to interact with graphics pipeline states 
	 * caching them to reduce CPU overhead
	 * 
	 */
	class D3D11RenderStateCache
	{
	public:

		/**
		 * @brief Returns render target cache object
		 * 
		 * @return D3D11RenderTargetCache 
		 */
		D3D11RenderTargetCache GetRTCache() { return m_rtCache; }

		/**
		 * @brief Immediatly sends cached render targets to GPU
		 * 
		 * @param context - device context
		 * 
		 * *Use this method if you want to use resources of binded render targets in next calls
		 */
		void FlushRenderTargets(ID3D11DeviceContext* context);

		/**
		 * @brief Sends all cached render states to GPU
		 * 
		 * @param context - device context
		 */
		void Flush(ID3D11DeviceContext* context);
	private:
		D3D11RenderTargetCache m_rtCache;
	};
};