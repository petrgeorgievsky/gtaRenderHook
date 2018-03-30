#ifndef D3DRenderer_h__
#define D3DRenderer_h__
class CD3D1XStateManager;
class CD3DRenderer
{
public:
	/* \brief Initializes D3D renderer.
	 *
	 *  Initializes window handle, GPUs and display mode lists.
	 */
	CD3DRenderer(HWND& window);
	/* \brief Releases D3D interfaces.
	*
	*  Releases adapters and DXGIFactory.
	*/
	~CD3DRenderer();

	/* \brief Initializes D3D device.
	*
	*  Initializes device, immediate context and swap chain.
	*/
	bool InitDevice();
	/*
	*  Releases D3D device interfaces.
	*/
	void DeInitDevice();
	/* \brief Begins camera rendering.
	*
	*  Sets camera frame and depthstencilbuffer to output merger stage.
	*/
	void BeginUpdate(RwCamera *camera);
	/* \brief Ends camera rendering.
	*
	*  Resets output merger stage state.
	*/
	void EndUpdate(RwCamera *camera);
	/* 
	*  Clears camera framebufer and/or depthstencilbufer.
	*/
	void Clear(RwCamera *camera, RwRGBA& color, RwInt32 flags);
	/* 
	*  Shows image on the screen
	*/
	void Present(bool VSync);
	/*
	*  Draws indexed primitive.
	*/
	void DrawIndexed(UINT indexCount, UINT startIndex, UINT baseVertex);

	/*
	*  Returns d3d device pointer.
	*/
	ID3D11Device*           getDevice()						{ return m_pd3dDevice; }
	/*
	*  Returns d3d context pointer.
	*/
	ID3D11DeviceContext*	getContext()					{ return m_pImmediateContext; }
	/*
	*  Returns current feature level.
	*/
	D3D_FEATURE_LEVEL		&getFeatureLevel()				{ return m_featureLevel; }
	/*
	*  Returns found adapter count.
	*/
	UINT					getAdapterCount()				{ return m_vAdapters.size(); }
	/*
	*  Returns selected adapter id.
	*/
	UINT					getCurrentAdapter()			{ return m_uiCurrentAdapter; }
	/*
	*  Returns selected adapter mode id.
	*/
	UINT					getCurrentAdapterMode()		{ return m_uiCurrentAdapterMode; }
	/*
	*  Changes selected adapter id.
	*/
	void					setCurrentAdapter(UINT n)		{ if(n<getAdapterCount()) m_uiCurrentAdapter = n; }
	/*
	*  Changes selected adapter mode id.
	*/
	void					setCurrentAdapterMode(UINT n)	{ if (n<getAdapterModeCount()) m_uiCurrentAdapterMode = n; }
	/*
	*  Returns avaliable adapter mode count.
	*/
	UINT					getAdapterModeCount()			{ return m_vAdapterModes.size(); }
	/*
	*  Returns adapter mode description.
	*/
	DXGI_MODE_DESC			getAdapterModeDesc(UINT n)		
	{
		if (n<getAdapterModeCount()) return m_vAdapterModes[n];
		else	return m_vAdapterModes[0];
	}
	/*
	*  Returns selected adapter mode description.
	*/
	DXGI_MODE_DESC			getCurrentAdapterModeDesc()	{ return m_vAdapterModes[m_uiCurrentAdapterMode]; }
	/*
	*  Returns selected adapter name.
	*/
	const char*				getAdapterInfo(UINT n);
	/*
	*  Returns selected adapter id.
	*/
	IDXGISwapChain*		getSwapChain()					{ return m_pSwapChain; }
	/*
	*  Returns window handle.
	*/
	HWND					getHWND()						{ return m_hWnd; }
	/*
	*  Starts debugging event.
	*/
	void BeginDebugEvent(LPCWSTR name) {
		m_pRenderingAnnotation->BeginEvent(name);
	}
	/*
	*  Ends debugging event.
	*/
	void EndDebugEvent() {
		m_pRenderingAnnotation->EndEvent();
	}

private:
	HWND					m_hWnd					= nullptr;
	D3D_DRIVER_TYPE         m_driverType			= D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       m_featureLevel			= D3D_FEATURE_LEVEL_11_0;
	IDXGIFactory*			m_pdxgiFactory			= nullptr;
	ID3D11Device*           m_pd3dDevice			= nullptr;
	ID3D11DeviceContext*    m_pImmediateContext		= nullptr;
	IDXGISwapChain*         m_pSwapChain			= nullptr;
	IDXGISwapChain1*        m_pSwapChain1			= nullptr;
	ID3D11Debug*			m_pDebug				= nullptr;
	ID3DUserDefinedAnnotation* m_pRenderingAnnotation = nullptr;
	UINT					m_uiWidth = 0, 
							m_uiHeight = 0,
							m_uiCurrentAdapter = 0,
							m_uiCurrentAdapterMode = 0;
	std::vector <IDXGIAdapter*>	m_vAdapters		{};
	std::vector <DXGI_MODE_DESC>	m_vAdapterModes {};
};

#endif // D3DRenderer_h__
