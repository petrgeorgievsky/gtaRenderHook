// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3DRenderer.h"
#include "D3D1XTexture.h"
#include <directxcolors.h>
#include "D3D1XStateManager.h"
#include "D3DSpecificHelpers.h"
#include "CDebug.h"

CD3DRenderer::CD3DRenderer(HWND& window)
{
	m_hWnd = window;
	// dxgi factory initialization
	if (!CALL_D3D_API(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pdxgiFactory)),
		"Failed to create DXGI Factory.")) return;

	IDXGIAdapter * pAdapter;
	// Init adapter list.
	if (!gDebugSettings.UseDefaultAdapter) {
		for (UINT i = 0; m_pdxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
			m_vAdapters.push_back(pAdapter);
	}
	else {
		if(m_pdxgiFactory->EnumAdapters(0, &pAdapter) != DXGI_ERROR_NOT_FOUND)
			m_vAdapters.push_back(pAdapter);
	}
	
	IDXGIOutput *pOutput;
	if (!CALL_D3D_API(m_vAdapters[m_uiCurrentAdapter]->EnumOutputs(0, &pOutput),
		"Failed to enumerate adapter output #0.")) return;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// retrieve avaliable display mode count.
	UINT modeCount	= 0;
	if (!CALL_D3D_API(pOutput->GetDisplayModeList(format, 0, &modeCount, nullptr),
		"Failed to get display mode count.")) {
		pOutput->Release();
		return;
	}

	std::vector<DXGI_MODE_DESC> modeDescriptions{ modeCount };
	// get display mode list
	if (!CALL_D3D_API(pOutput->GetDisplayModeList(format, 0, &modeCount, modeDescriptions.data()), 
		"Failed to retrieve display mode list.")) {
		pOutput->Release();
		return;
	}

	// populate adapter mode list, if mode description has refresh rate less than 60 Hz
	// TODO: perhaps max refresh rate should be choosen seperatly, maybe from settings file
	for (UINT i = 0; i < modeDescriptions.size(); i++)
	{
		if (modeDescriptions[i].RefreshRate.Numerator / modeDescriptions[i].RefreshRate.Denominator <= 60)
			m_vAdapterModes.push_back(modeDescriptions[i]);
	}

	pOutput->Release();
}

CD3DRenderer::~CD3DRenderer()
{
	for (auto pAdapter : m_vAdapters)
		pAdapter->Release();
}

bool CD3DRenderer::InitDevice()
{
	HRESULT hr = S_OK;
	// return if something is wrong, e.g. we have set some wrong parameters.
	// TODO: try to avoid these errors, for example reset current adapter mode to 0 or something like that, but perhaps this is just a garbage thought
	if (m_vAdapterModes.size() <= 1 ||
		m_uiCurrentAdapterMode > m_vAdapterModes.size())
		return false;

	auto currentAdapterMode = m_vAdapterModes[m_uiCurrentAdapterMode];
	auto currentAdapter = m_vAdapters[m_uiCurrentAdapter];

	g_pDebug->printMsg("Device initialization: start", 1);
	// resize and reposition rendering window if required
	// TODO: add ability to choose wether to center it if we are in windowed mode
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	m_uiWidth  = rc.right - rc.left;
	m_uiHeight = rc.bottom - rc.top;
	if (m_uiWidth != currentAdapterMode.Width || m_uiHeight != currentAdapterMode.Height) {
		m_uiWidth = currentAdapterMode.Width;
		m_uiHeight = currentAdapterMode.Height;
		SetWindowPos(m_hWnd, nullptr, 0, 0, currentAdapterMode.Width, currentAdapterMode.Height, 0);
	}
	// initialize device creation flags
	// TODO: add ability to set some custom flags
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	//createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	// initialize driver types and feature levels
	// TODO: move them to header, or store them in memory, perhaps
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_UNKNOWN,
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);


	// loop over each driver type to find supported one
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(m_driverType== D3D_DRIVER_TYPE_HARDWARE? nullptr : currentAdapter, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		// if we failed to create device with invalid arg error, 
		// probably it's because current feature level is not supported by GPU,
		// so we will try to find supported feature level set
		if (hr == E_INVALIDARG)
		{
			
			// try to find supported feature level by iterating over whole set and removing one feature level after another until success
			for (UINT i = 1; i < numFeatureLevels-1; i++) {
				hr = D3D11CreateDevice(m_driverType == D3D_DRIVER_TYPE_HARDWARE ? nullptr : currentAdapter, m_driverType, nullptr, createDeviceFlags, &featureLevels[i], numFeatureLevels - i,
					D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
				if (SUCCEEDED(hr))
					break;
			}
		}

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr)) {
		g_pDebug->printError("Failed to create device, invalid input arguments");
		return false;
	}

	g_pDebug->printMsg("D3D11 Device created",1);
	// Query debug interface
	m_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pDebug);

	// Create swap chain for device.  
	IDXGIFactory2* dxgiFactory2 = nullptr;
	m_pdxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		m_pImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&m_pRenderingAnnotation));
		// Initialize swap chain parameters
		// TODO: add ability to set flags and some params in settings file
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width  = currentAdapterMode.Width;
		sd.Height = currentAdapterMode.Height;
		sd.Format = currentAdapterMode.Format;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.Flags = gDebugSettings.Windowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsd;
		ZeroMemory(&fsd, sizeof(fsd));
		fsd.Scaling = currentAdapterMode.Scaling;
		fsd.ScanlineOrdering = currentAdapterMode.ScanlineOrdering;
		fsd.Windowed = gDebugSettings.Windowed;
		fsd.RefreshRate = currentAdapterMode.RefreshRate;

		if(CALL_D3D_API(dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, m_hWnd, &sd, &fsd, nullptr, &m_pSwapChain1),
			"Failed to create swap chain using DX11.1 API."))
			m_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain1), reinterpret_cast<void**>(&m_pSwapChain));
		

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width  = currentAdapterMode.Width;
		sd.BufferDesc.Height = currentAdapterMode.Height;
		sd.BufferDesc.Format = currentAdapterMode.Format;
		sd.BufferDesc.RefreshRate = currentAdapterMode.RefreshRate;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = gDebugSettings.Windowed;
		sd.Flags = gDebugSettings.Windowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
		CALL_D3D_API(m_pdxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain), "Failed to create swap chain using DX11 API");
	}
	m_pdxgiFactory->Release();

	g_pDebug->printMsg("Device initialization: end",1);

	return true;
}

void CD3DRenderer::DeInitDevice()
{
	if (m_pImmediateContext) { 
		m_pImmediateContext->ClearState();
		m_pImmediateContext->Flush();
	}

	if (m_pSwapChain1) { 
		m_pSwapChain1->Release();
		m_pSwapChain1 = nullptr;
	}else if (m_pSwapChain) { 
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
	if (m_pImmediateContext) { 
		m_pImmediateContext->Release();
		m_pImmediateContext = nullptr;
	}
	if (m_pd3dDevice) {
		m_pd3dDevice->Release();
		m_pd3dDevice = nullptr;
	}
	if (m_pRenderingAnnotation) {
		m_pRenderingAnnotation->Release();
		m_pRenderingAnnotation = nullptr;
	}
	if (m_pDebug) {
		m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL); 
		m_pDebug->Release(); 
		m_pDebug = nullptr;
	}
}

void CD3DRenderer::BeginUpdate(RwCamera *camera)
{
	RwD3D1XRaster* pd3dDepthRaster = GetD3D1XRaster(camera->zBuffer);
	RwD3D1XRaster* pd3dRTRaster = GetD3D1XRaster(camera->frameBuffer);

	// set render targets and depth buffers for camera
	// texture flags check is used if you want to use UAV for rendering
	// TODO: remove UAV set for camera, and make some separate functions for that
	if (camera->frameBuffer && pd3dRTRaster->textureFlags!=64) {
		// Set render targets
		auto rtrv = pd3dRTRaster->resourse->GetRTRV();

		if (camera->zBuffer != nullptr) 
			g_pStateMgr->SetRenderTargets(1, &rtrv, pd3dDepthRaster->resourse->GetDSRV());		
		else
			g_pStateMgr->SetRenderTargets(1, &rtrv, nullptr);
	}
	else if(camera->frameBuffer&&pd3dRTRaster->textureFlags == 64) {
		// TODO: remove com pointers, or left as is, don't know if that will affect anything
		CComPtr<ID3D11RenderTargetView> renderTargets;
		CComPtr<ID3D11DepthStencilView> depthStencil;
		m_pImmediateContext->OMGetRenderTargets(1, &renderTargets, &depthStencil);
		
		auto uav = pd3dRTRaster->resourse->GetUAV();
		m_pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &renderTargets.p, nullptr, 2, 1, &uav, nullptr);
	}
	else {
		if (camera->zBuffer != nullptr) {
			g_pStateMgr->SetRenderTargets(0, nullptr, pd3dDepthRaster->resourse->GetDSRV());
		}
	}
	// Setup the viewport
	if (camera->frameBuffer) {
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)camera->frameBuffer->width;
		vp.Height = (FLOAT)camera->frameBuffer->height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		g_pStateMgr->SetViewport(vp);
	}
	g_pStateMgr->FlushRenderTargets();
}

void CD3DRenderer::EndUpdate(RwCamera *camera)
{
	// remove UAV from camera, will remove this later
	RwD3D1XRaster* pd3dRTRaster = GetD3D1XRaster(camera->frameBuffer);
	if (camera->frameBuffer && pd3dRTRaster->textureFlags == 64)
	{
		ID3D11UnorderedAccessView* uavs =  nullptr;
		m_pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(0, { nullptr }, nullptr, 2, 1, &uavs, nullptr);
	}
}

void CD3DRenderer::Clear(RwCamera *camera,RwRGBA& color, RwInt32 flags)
{
	RwD3D1XRaster* pd3dDepthRaster = GetD3D1XRaster(camera->zBuffer);
	RwD3D1XRaster* pd3dRTRaster = GetD3D1XRaster(camera->frameBuffer);

	if (flags&rwCAMERACLEARIMAGE) {
		float clearColor[] = { color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, color.alpha / 255.0f };
		m_pImmediateContext->ClearRenderTargetView(pd3dRTRaster->resourse->GetRTRV(), clearColor);
	}
	if ((flags&rwCAMERACLEARZ || flags&rwCAMERACLEARSTENCIL)&& camera->zBuffer) {
		if(!(flags&rwCAMERACLEARZ))
			m_pImmediateContext->ClearDepthStencilView(pd3dDepthRaster->resourse->GetDSRV(), D3D11_CLEAR_STENCIL, 1.0f, 0);
		else if (!(flags&rwCAMERACLEARSTENCIL))
			m_pImmediateContext->ClearDepthStencilView(pd3dDepthRaster->resourse->GetDSRV(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		else
			m_pImmediateContext->ClearDepthStencilView(pd3dDepthRaster->resourse->GetDSRV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
}

void CD3DRenderer::Present(bool VSync)
{
	m_pSwapChain->Present(VSync?1:0, 0);
}

void CD3DRenderer::DrawIndexed(UINT indexCount, UINT startIndex, UINT baseVertex)
{
	m_pImmediateContext->DrawIndexed(indexCount, startIndex, baseVertex);
}

unsigned int CD3DRenderer::getAvaliableTextureMemory()
{
	DXGI_ADAPTER_DESC desc;
	if (!CALL_D3D_API(m_vAdapters[m_uiCurrentAdapter]->GetDesc(&desc), "Failed to retrieve adapter description."))
		return 0;
	return desc.DedicatedVideoMemory;
}

const char* CD3DRenderer::getAdapterInfo(UINT n)
{
	if (n>getAdapterCount())
		return "null";
	char* info=new char[80];
	DXGI_ADAPTER_DESC desc;

	if(!CALL_D3D_API(m_vAdapters[n]->GetDesc(&desc),"Failed to retrieve adapter description."))
		wcstombs(info, L"no info", 80);
	else
		wcstombs(info, desc.Description, 80);

	return info;
}
