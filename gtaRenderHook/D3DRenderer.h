#ifndef D3DRenderer_h__
#define D3DRenderer_h__
class CD3D1XStateManager;
class CD3DRenderer
{
public:
	CD3DRenderer(HWND& window);
	~CD3DRenderer();

	bool InitDevice();
	void DeInitDevice();

	void BeginUpdate(RwCamera *camera);
	void EndUpdate(RwCamera *camera);
	void Clear(RwCamera *camera,RwRGBA& color, RwInt32 flags);
	void Present(bool VSync);
	void DrawIndexed(UINT indexCount, UINT startIndex, UINT baseVertex);

	ID3D11Device*           getDevice()						{ return m_pd3dDevice; }
	ID3D11DeviceContext*	getContext()					{ return m_pImmediateContext; }
	D3D_FEATURE_LEVEL		&getFeatureLevel()				{ return m_featureLevel; }
	UINT					getAdapterCount()				{ return m_vAdapters.size(); }
	UINT					&getCurrentAdapter()			{ return m_uiCurrentAdapter; }
	UINT					&getCurrentAdapterMode()		{ return m_uiCurrentAdapterMode; }
	void					setCurrentAdapter(UINT n)		{ if(n<getAdapterCount()) m_uiCurrentAdapter = n; }
	void					setCurrentAdapterMode(UINT n)	{ if (n<getAdapterModeCount()) m_uiCurrentAdapterMode = n; }
	UINT					getAdapterModeCount()			{ return m_vAdapterModes.size(); }
	DXGI_MODE_DESC			&getAdapterModeDesc(UINT n)		
	{
		if (n<getAdapterModeCount()) return m_vAdapterModes[n];
		else	return m_vAdapterModes[0];
	}
	DXGI_MODE_DESC			&getCurrentAdapterModeDesc()	{ return m_vAdapterModes[m_uiCurrentAdapterMode]; }
	const char*				getAdapterInfo(UINT n);
	IDXGISwapChain1*		getSwapChain()					{ return m_pSwapChain1; }
	HWND					&getHWND()						{ return m_hWnd; }
	void BeginDebugEvent(LPCWSTR name) {
		userDefAnn->BeginEvent(name);
	}
	void EndDebugEvent() {
		userDefAnn->EndEvent();
	}

private:
	HWND					m_hWnd					= nullptr;
	D3D_DRIVER_TYPE         m_driverType			= D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       m_featureLevel			= D3D_FEATURE_LEVEL_11_0;
	IDXGIFactory*			m_pdxgiFactory			= nullptr;
	ID3D11Device*           m_pd3dDevice			= nullptr;
	ID3D11Device1*          m_pd3dDevice1			= nullptr;
	ID3D11Device2*          m_pd3dDevice2			= nullptr;
	ID3D11Device3*          m_pd3dDevice3			= nullptr;
	ID3D11DeviceContext*    m_pImmediateContext		= nullptr;
	ID3D11DeviceContext1*   m_pImmediateContext1	= nullptr;
	IDXGISwapChain*         m_pSwapChain			= nullptr;
	IDXGISwapChain1*        m_pSwapChain1			= nullptr;
	ID3D11Debug*			m_pDebug				= nullptr;
	ID3DUserDefinedAnnotation* userDefAnn = nullptr;
	UINT					m_uiWidth = 0, 
							m_uiHeight = 0,
							m_uiCurrentAdapter = 0,
							m_uiCurrentAdapterMode = 0;
	std::vector <IDXGIAdapter*>	m_vAdapters		{};
	std::vector <DXGI_MODE_DESC>	m_vAdapterModes {};
};

#endif // D3DRenderer_h__
