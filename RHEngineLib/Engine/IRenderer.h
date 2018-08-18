#pragma once
#include "../stdafx.h"
#include "Definitions.h"
namespace RHEngine
{
	struct DisplayModeInfo 
	{
		unsigned int width;
		unsigned int height;
		unsigned int refreshRate;
	};
	/**
		\brief Renderer interface for API-dependant renderers

		This abstraction clusterfuck is created to be able to implement other wrappers for APIs
	*/
	class IRenderer
	{
	public:
		/**
		 * @brief Construct a new IRenderer object
		 * 
		 * @param window - window handle
		 * @param inst - instance handle
		 */
		IRenderer(HWND window, HINSTANCE inst) {
			m_hWnd = window; m_hInst = inst;
		};

		/**
		 * @brief Destroy the IRenderer object
		 * 
		 */
		~IRenderer() {};

		/**
		 * @brief Initializes rendering device and main swap-chain.
		 * 
		 * @return true if initialization was succesful
		 */
		virtual bool InitDevice() = 0;
		
		/**
		 * @brief Releases resources held by rendering device
		 * 
		 * @return true if everything went well
		 */
		virtual bool ShutdownDevice() = 0;

		/**
		 * @brief Get Adapters count
		 * 
		 * @param n - adapters (GPU) count
		 * @return true if no error occurs
		 */
		virtual bool GetAdaptersCount(int& n) = 0;

		/**
		 * @brief Get the Adapter Info
		 * 
		 * @param n - adapter id
		 * @param info - adapter name
		 * @return true if no error occurs
		 */
		virtual bool GetAdapterInfo(unsigned int n, std::wstring& info)=0;

		/**
		 * @brief Set Current Adapter
		 * 
		 * @param n - adapter id
		 * @return true if no error occurs
		 */
		virtual bool SetCurrentAdapter(unsigned int n) = 0;

		/**
		 * @brief Get Current Adapter
		 * 
		 * @param n - current adapter
		 * @return true if no error occurs
		 */
		virtual bool GetCurrentAdapter(int& n) = 0;

		/**
		 * @brief Get adapter output count
		 * 
		 * @param adapterId - adapter id
		 * @param n - output(monitor) count
		 * @return true if no error occurs
		 */
		virtual bool GetOutputCount(unsigned int adapterId, int& n) = 0;

		/**
		 * @brief Get output device info
		 * 
		 * @param n - output device id
		 * @param info - output device name
		 * @return true if no error occurs
		 */
		virtual bool GetOutputInfo(unsigned int n, std::wstring& info) = 0;

		/**
		 * @brief Sets output(display) device for current adapter
		 * 
		 * @param id - output device id
		 * @return true if no error occurs
		 */
		virtual bool SetCurrentOutput(unsigned int id) = 0;

		/**
		 * @brief Get current output device
		 * 
		 * @param id - current output device id
		 * @return true if no error occurs
		 */
		virtual bool GetCurrentOutput(int& id) = 0;

		/**
		 * @brief Get Display Mode count
		 * 
		 * @param outputId - current output(display) device id
		 * @param count - display mode count
		 * @return true if no error occurs
		 */
		virtual bool GetDisplayModeCount(unsigned int outputId, int& count) = 0;
		
		/*
			Returns display mode info.
		*/
		virtual bool GetDisplayModeInfo(unsigned int id, DisplayModeInfo& info)=0;
		/*
			Sets display mode for current adapter and selected output.
		*/
		virtual bool SetCurrentDisplayMode(unsigned int id) = 0;
		/*
			Returns current display mode.
		*/
		virtual bool GetCurrentDisplayMode(int&) = 0;
		/*
			Allocates image buffer of required size and required type, and returns pointer to allocated buffer.
		*/
		virtual void* AllocateImageBuffer(unsigned int width, unsigned int height, RHImageBufferType type) = 0;
		/*
			Frees image buffer allocated by this renderer.
		*/
		virtual bool FreeImageBuffer(void * buffer, RHImageBufferType type) = 0;
		/*
			Binds image buffers to specific pipeline stage.
		*/
		virtual bool BindImageBuffers(RHImageBindType bindType, const std::unordered_map<int, void*>& buffers) = 0;
		virtual bool ClearImageBuffer(RHImageClearType clearType, void* buffer,const float clearColor[4]) = 0;
		virtual bool Present(void* image) = 0;
		virtual bool BeginCommandList(void* cmdList) = 0;
		virtual bool EndCommandList(void* cmdList) = 0;
		virtual bool RequestSwapChainImage(void* frameBuffer) = 0;
		virtual bool PresentSwapChainImage(void* frameBuffer) = 0;
	protected:
		HWND m_hWnd = nullptr;
		HINSTANCE m_hInst = nullptr;
	};
};