#include "stdafx.h"
#include "D3DRenderer.h"
#include "D3D1XTexture.h"
#include <directxcolors.h>
#include "D3D1XStateManager.h"
#include "CDebug.h"
CD3DRenderer::CD3DRenderer(HWND& window)
{
	m_hWnd = window;
	
	if(FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pdxgiFactory))))
		g_pDebug->printError("Failed to create DXGI Factory.");

	IDXGIAdapter * pAdapter;
	// Init adapter list.
	for (UINT i = 0;
		m_pdxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
		m_vAdapters.push_back(pAdapter);
	
	IDXGIOutput *pOutput;
	m_vAdapters[m_uiCurrentAdapter]->EnumOutputs(0, &pOutput);

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R16G16B16A16_FLOAT
	// Init display mode list.
	UINT modeCount	= 0;
	pOutput->GetDisplayModeList(format, 0, &modeCount, nullptr);
	std::vector<DXGI_MODE_DESC> modeDescriptions{ modeCount };
	pOutput->GetDisplayModeList(format, 0, &modeCount, modeDescriptions.data());
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
	if (m_pdxgiFactory)
		m_pdxgiFactory->Release();
}

bool CD3DRenderer::InitDevice()
{
	HRESULT hr = S_OK;
	g_pDebug->printMsg("Device initialization: start", 1);
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	m_uiWidth  = rc.right - rc.left;
	m_uiHeight = rc.bottom - rc.top;
	if (m_uiWidth != m_vAdapterModes[m_uiCurrentAdapterMode].Width || m_uiHeight != m_vAdapterModes[m_uiCurrentAdapterMode].Height) {
		m_uiWidth = m_vAdapterModes[m_uiCurrentAdapterMode].Width;
		m_uiHeight = m_vAdapterModes[m_uiCurrentAdapterMode].Height;
		SetWindowPos(m_hWnd, nullptr, 0, 0, m_vAdapterModes[m_uiCurrentAdapterMode].Width, m_vAdapterModes[m_uiCurrentAdapterMode].Height, 0);
	}

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	//createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		/*D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,*/
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = D3D_DRIVER_TYPE_UNKNOWN;//driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(m_vAdapters[m_uiCurrentAdapter], m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		
		if (hr == E_INVALIDARG)
		{
			g_pDebug->printMsg("Failed to create device, invalid input arguments",0);
			// Try to find supported feature level
			hr = D3D11CreateDevice(m_vAdapters[m_uiCurrentAdapter], m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return SUCCEEDED(hr);
	g_pDebug->printMsg("D3D11 Device created",1);
	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	m_pdxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device3), reinterpret_cast<void**>(&m_pd3dDevice3));
		m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device2), reinterpret_cast<void**>(&m_pd3dDevice2));
		hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_pd3dDevice1));

		m_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pDebug);
		m_pImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&userDefAnn));
		if (SUCCEEDED(hr))
		{
			(void)m_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width  = m_vAdapterModes[m_uiCurrentAdapterMode].Width;
		sd.Height = m_vAdapterModes[m_uiCurrentAdapterMode].Height;
		sd.Format = m_vAdapterModes[m_uiCurrentAdapterMode].Format;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsd;
		ZeroMemory(&fsd, sizeof(fsd));
		fsd.Scaling = m_vAdapterModes[m_uiCurrentAdapterMode].Scaling;
		fsd.ScanlineOrdering = m_vAdapterModes[m_uiCurrentAdapterMode].ScanlineOrdering;
		fsd.Windowed = TRUE;
		fsd.RefreshRate = m_vAdapterModes[m_uiCurrentAdapterMode].RefreshRate;
		hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, m_hWnd, &sd, &fsd, nullptr, &m_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = m_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width  = m_vAdapterModes[m_uiCurrentAdapterMode].Width;
		sd.BufferDesc.Height = m_vAdapterModes[m_uiCurrentAdapterMode].Height;
		sd.BufferDesc.Format = m_vAdapterModes[m_uiCurrentAdapterMode].Format;
		sd.BufferDesc.RefreshRate = m_vAdapterModes[m_uiCurrentAdapterMode].RefreshRate;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		hr = m_pdxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain);
	}
	m_pdxgiFactory->Release();
	g_pDebug->printMsg("Device initialization: end",1);
	if (FAILED(hr))
		return SUCCEEDED(hr);

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
	}
	if (m_pSwapChain) { 
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
	if (m_pImmediateContext1)
	{
		m_pImmediateContext1->Release();
		m_pImmediateContext1 = nullptr;
	}
	if (m_pImmediateContext) { 
		m_pImmediateContext->Release();
		m_pImmediateContext = nullptr;
	}
	if (m_pd3dDevice3) {
		m_pd3dDevice3->Release();
		m_pd3dDevice3 = nullptr;
	}
	if (m_pd3dDevice2) {
		m_pd3dDevice2->Release();
		m_pd3dDevice2 = nullptr;
	}
	if (m_pd3dDevice1) { 
		m_pd3dDevice1->Release(); 
		m_pd3dDevice1 = nullptr;
	}
	if (m_pd3dDevice) {
		m_pd3dDevice->Release();
		m_pd3dDevice = nullptr;
	}
	userDefAnn->Release();
	if (m_pDebug)
		m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	if (m_pDebug) m_pDebug->Release();
}

void CD3DRenderer::BeginUpdate(RwCamera *camera)
{
	RwD3D1XRaster* pd3dDepthRaster = GetD3D1XRaster(camera->zBuffer);
	RwD3D1XRaster* pd3dRTRaster = GetD3D1XRaster(camera->frameBuffer);

	if (camera->frameBuffer && pd3dRTRaster->textureFlags!=64)
	{
		// Set render targets
		auto rtrv = pd3dRTRaster->resourse->GetRTRV();

		if (camera->zBuffer != nullptr) {
			g_pStateMgr->SetRenderTargets(1, &rtrv, pd3dDepthRaster->resourse->GetDSRV());
		}
		else
			g_pStateMgr->SetRenderTargets(1, &rtrv, nullptr);
	}
	else if(camera->frameBuffer&&pd3dRTRaster->textureFlags == 64){
		CComPtr<ID3D11RenderTargetView> renderTargets;
		CComPtr<ID3D11DepthStencilView> depthStencil;
		m_pImmediateContext->OMGetRenderTargets(1, &renderTargets, &depthStencil);
		
		auto uav = pd3dRTRaster->resourse->GetUAV().p;
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
	float clearColor[] = { color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, color.alpha / 255.0f };
	if (flags&rwCAMERACLEARIMAGE) {
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

const char* CD3DRenderer::getAdapterInfo(UINT n)
{
	if (n>getAdapterCount())
		return "null";
	char* info=new char[80];
	DXGI_ADAPTER_DESC desc;

	m_vAdapters[n]->GetDesc(&desc);
	wcstombs(info, desc.Description, 80);

	return info;
}
