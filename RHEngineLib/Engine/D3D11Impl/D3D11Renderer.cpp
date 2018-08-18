#include "stdafx.h"
#include "D3D11Renderer.h"
#include "D3D11Common.h"
#include "ImageBuffers/D3D11BackBuffer.h"

RHEngine::D3D11Renderer::D3D11Renderer(HWND window, HINSTANCE inst): IRenderer(window,inst)
{
	// dxgi factory initialization
	if (!CALL_D3D_API(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pdxgiFactory)),
		L"Failed to create DXGI Factory.")) return;

	IDXGIAdapter * pAdapter;
	// Init adapter list.
	for (UINT i = 0; m_pdxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		m_vAdapters.push_back(pAdapter);

	SetCurrentAdapter(0);
	SetCurrentOutput(0);
}

RHEngine::D3D11Renderer::~D3D11Renderer()
{
	for (auto output : m_vOutputs) {
		output->Release();
		output = nullptr;
	}
	for (auto adapter : m_vAdapters) {
		adapter->Release();
		adapter = nullptr;
	}
}

bool RHEngine::D3D11Renderer::InitDevice()
{
	RHDebug::DebugLogger::Log(TEXT("Device initialization start"));
	// return if something is wrong, e.g. we have set some wrong parameters.
	// TODO: try to avoid these errors, for example reset current adapter mode to 0 or something like that, but perhaps this is just a garbage thought
	if (m_vDisplayModes.size() < 1 ||
		m_uiCurrentDisplayMode >= m_vDisplayModes.size())
		return false;

	auto currentAdapterMode = m_vDisplayModes[m_uiCurrentDisplayMode];
	auto currentAdapter = m_vAdapters[m_uiCurrentAdapter];
	bool isWindowed = true;//gDebugSettings.GetToggleField("Windowed");
	HRESULT hr = S_OK;
	// resize and reposition rendering window if required
	// TODO: add ability to choose wether to center it if we are in windowed mode
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	auto windowWidth = rc.right - rc.left;
    auto windowHeight = rc.bottom - rc.top;
	if (windowWidth != currentAdapterMode.Width || windowHeight != currentAdapterMode.Height)
		SetWindowPos(m_hWnd, nullptr, 0, 0, currentAdapterMode.Width, currentAdapterMode.Height, 0);
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
		hr = D3D11CreateDevice(m_driverType == D3D_DRIVER_TYPE_HARDWARE ? nullptr : currentAdapter,
			m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

		// if we failed to create device with invalid arg error, 
		// probably it's because current feature level is not supported by GPU,
		// so we will try to find supported feature level set
		if (hr == E_INVALIDARG)
		{

			// try to find supported feature level by iterating over whole set and removing one feature level after another until success
			for (UINT i = 1; i < numFeatureLevels - 1; i++) {
                hr = D3D11CreateDevice( m_driverType == D3D_DRIVER_TYPE_HARDWARE ? nullptr : currentAdapter, m_driverType, 
                    nullptr, createDeviceFlags, &featureLevels[i], numFeatureLevels - i,
                    D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext );
                if (SUCCEEDED(hr))
                    break;
            }
		}

		if (SUCCEEDED(hr))
			break;
	}

	if (!CALL_D3D_API(hr, TEXT("Failed to create device, invalid input arguments!")))
		return false;
	
	RHDebug::DebugLogger::Log(TEXT("D3D11 Device created"));
	// Query debug interface
	m_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pDebug);

	// Create swap chain for device.  
	// DirectX 11.0 systems
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = currentAdapterMode.Width;
	sd.BufferDesc.Height = currentAdapterMode.Height;
	sd.BufferDesc.Format = currentAdapterMode.Format;
	sd.BufferDesc.RefreshRate = currentAdapterMode.RefreshRate;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = isWindowed;
	sd.Flags = isWindowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
	if (!CALL_D3D_API(m_pdxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain), TEXT("Failed to create swap chain using DX11 API")))
		return false;
	m_pdxgiFactory->Release();
	m_pRenderStateCache = new D3D11RenderStateCache();
	return true;
}

bool RHEngine::D3D11Renderer::ShutdownDevice()
{
	delete m_pRenderStateCache;
	if (m_pImmediateContext) {
		m_pImmediateContext->ClearState();
		m_pImmediateContext->Flush();
	}
	if (m_pSwapChain) {
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
	if (m_pImmediateContext) {
		m_pImmediateContext->Release();
		m_pImmediateContext = nullptr;
	}
    if (m_pDebug) {
        m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
        m_pDebug->Release();
        m_pDebug = nullptr;
    }
	if (m_pd3dDevice) {
		m_pd3dDevice->Release();
		m_pd3dDevice = nullptr;
	}
	return true;
}

bool RHEngine::D3D11Renderer::GetAdaptersCount(int &count)
{
	if (!m_vAdapters.empty()) 
	{
		count = m_vAdapters.size();
		return true;
	}
	return false;
}

bool RHEngine::D3D11Renderer::GetAdapterInfo(unsigned int n, std::wstring &info)
{
	if(n >= m_vAdapters.size())
		return false;
	
	DXGI_ADAPTER_DESC desc;
	if (!CALL_D3D_API(m_vAdapters[n]->GetDesc(&desc),
		L"Failed to get adapter description!"))
		return false;
	info = desc.Description;
	return true;
}

bool RHEngine::D3D11Renderer::GetOutputCount(unsigned int adapterId, int &c)
{
	// TODO: Consider n
	c = m_vOutputs.size();
	return true;
}

bool RHEngine::D3D11Renderer::GetOutputInfo(unsigned int n, std::wstring &info)
{
	if (n >= m_vOutputs.size())
		return false;

	DXGI_OUTPUT_DESC desc;
	if (!CALL_D3D_API(m_vOutputs[n]->GetDesc(&desc),
        TEXT("Failed to get output description!")))
		return false;
	info = desc.DeviceName;
	return true;
}

bool RHEngine::D3D11Renderer::SetCurrentOutput(unsigned int id)
{
	if (id >= m_vOutputs.size())
		return false;
	m_uiCurrentOutput = id;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// retrieve avaliable display mode count.
	UINT modeCount = 0;
	if (!CALL_D3D_API(m_vOutputs[m_uiCurrentOutput]->GetDisplayModeList(format, 0, &modeCount, nullptr),
        TEXT("Failed to get display mode count."))) {
		return false;
	}

	std::vector<DXGI_MODE_DESC> modeDescriptions{ modeCount };
	// get display mode list
	if (!CALL_D3D_API(m_vOutputs[m_uiCurrentOutput]->GetDisplayModeList(format, 0, &modeCount, modeDescriptions.data()),
        TEXT("Failed to retrieve display mode list."))) {
		return false;
	}
	// populate adapter mode list
	for (UINT i = 0; i < modeDescriptions.size(); i++)
		if (modeDescriptions[i].RefreshRate.Denominator != 0)
			m_vDisplayModes.push_back(modeDescriptions[i]);
	return true;
}

bool RHEngine::D3D11Renderer::SetCurrentAdapter(unsigned int n)
{
	if(n>=m_vAdapters.size())
		return false;
	m_uiCurrentAdapter = n;
	m_uiCurrentOutput = 0;
	// Clear last output list
	for (auto output: m_vOutputs)
		output->Release();
	m_vOutputs.clear();

	// Init output list.
	IDXGIOutput* output;
	for (UINT i = 0; m_vAdapters[m_uiCurrentAdapter]->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; ++i)
		m_vOutputs.push_back(output);
	m_vDisplayModes.clear();
	// If no outputs found - something is wrong
	if (m_vOutputs.size() <= 0)
		return false;
	return true;
}

bool RHEngine::D3D11Renderer::GetDisplayModeCount(unsigned int outputId, int & c)
{
	if(outputId >=m_vOutputs.size())
		return false;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// retrieve avaliable display mode count.
	UINT modeCount = 0;
	if (!CALL_D3D_API(m_vOutputs[outputId]->GetDisplayModeList(format, 0, &modeCount, nullptr),
        TEXT("Failed to get display mode count."))) {
		return false;
	}
	c = modeCount;
	return true;
}

bool RHEngine::D3D11Renderer::SetCurrentDisplayMode(unsigned int id)
{
	if (m_vDisplayModes.size() == 0 ||
		id >= m_vDisplayModes.size())
		return false;
	m_uiCurrentDisplayMode = id;
	return true;
}

bool RHEngine::D3D11Renderer::GetDisplayModeInfo(unsigned int id, DisplayModeInfo & info)
{
	if(id >= m_vDisplayModes.size())
		return false;
	info.width = m_vDisplayModes[id].Width;
	info.height = m_vDisplayModes[id].Height;
	info.refreshRate = m_vDisplayModes[id].RefreshRate.Denominator == 0 ?
        0 : m_vDisplayModes[id].RefreshRate.Numerator / m_vDisplayModes[id].RefreshRate.Denominator;
	return true;
}

bool RHEngine::D3D11Renderer::GetCurrentAdapter(int & n)
{
	if(m_vAdapters.size()==0 || 
		m_uiCurrentAdapter>=m_vAdapters.size())
		return false;
	n = m_uiCurrentAdapter;
	return true;
}

bool RHEngine::D3D11Renderer::GetCurrentOutput(int & n)
{
	if (m_vOutputs.size() == 0 ||
		m_uiCurrentOutput >= m_vOutputs.size())
		return false;
	n = m_uiCurrentOutput;
	return true;
}

bool RHEngine::D3D11Renderer::GetCurrentDisplayMode(int & n)
{
	if (m_vDisplayModes.size() == 0 ||
		m_uiCurrentDisplayMode >= m_vDisplayModes.size())
		return false;
	n = m_uiCurrentDisplayMode;
	return true;
}

bool RHEngine::D3D11Renderer::Present(void* image)
{
	return CALL_D3D_API(m_pSwapChain->Present(0,0), TEXT("Main SwapChain Present() crash!"));
}

void * RHEngine::D3D11Renderer::AllocateImageBuffer(unsigned int width, unsigned int height, RHImageBufferType type)
{
	switch (type) 
	{
	case RHImageBufferType::BackBuffer:	// Default backbuffer, with swap-chain bound to main window
		return new D3D11BackBuffer(m_pd3dDevice, m_pSwapChain);
	default:
		return nullptr;
	}
}

bool RHEngine::D3D11Renderer::FreeImageBuffer(void * buffer, RHImageBufferType type)
{
    switch (type)
    {
    case RHEngine::RHImageBufferType::Unknown:
        break;
    case RHEngine::RHImageBufferType::BackBuffer:
        delete static_cast<D3D11BackBuffer*>(buffer);
        break;
    case RHEngine::RHImageBufferType::TextureBuffer:
        break;
    case RHEngine::RHImageBufferType::DepthBuffer:
        break;
    case RHEngine::RHImageBufferType::RenderTargetBuffer:
        break;
    default:
        break;
    }
	return true;
}

bool RHEngine::D3D11Renderer::BindImageBuffers(RHImageBindType bindType,const std::unordered_map<int, void*>& buffers)
{
	switch (bindType) 
	{
	case RHImageBindType::RenderTarget:
		m_pRenderStateCache->GetRTCache().SetRenderTargets(buffers);
		m_pRenderStateCache->FlushRenderTargets(m_pImmediateContext);
		return true;
	default:
		return false;
	}
}

bool RHEngine::D3D11Renderer::ClearImageBuffer(RHImageClearType clearType, void* buffer,const float clearColor[4])
{
	ID3D11RenderTargetViewable * rtv = reinterpret_cast<ID3D11RenderTargetViewable*>(buffer);
	switch (clearType) {
	case RHImageClearType::Color:
		
		if (!rtv) 
			return false;

		m_pImmediateContext->ClearRenderTargetView(rtv->GetRenderTargetView(), clearColor);
		return true;
	default:
		return false;
	}
}

bool RHEngine::D3D11Renderer::BeginCommandList(void * cmdList)
{
	return false;
}

bool RHEngine::D3D11Renderer::EndCommandList(void * cmdList)
{
	return false;
}

bool RHEngine::D3D11Renderer::RequestSwapChainImage(void * frameBuffer)
{
	return false;
}

bool RHEngine::D3D11Renderer::PresentSwapChainImage(void * frameBuffer)
{
	return false;
}
