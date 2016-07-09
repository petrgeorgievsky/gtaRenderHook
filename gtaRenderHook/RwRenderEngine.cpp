#include "stdafx.h"
#include "RwRenderEngine.h"

bool CIRwRenderEngine::EventHandlingSystem(RwRenderSystemState State, int * a2, void * a3, int a4)
{
	string s = "Event catched: "; s += to_string(static_cast<int>(State));
	m_pDebug->printMsg(s);
	switch (State)
	{
	case RwRenderSystemState::RW_RENDER_INIT:
		return Create((HWND*)a3);
	case RwRenderSystemState::RW_RENDER_SHUTDOWN:
		return Shutdown();
	case RwRenderSystemState::RW_CREATE_RENDER_DEVICE:
		return CreateRenderDevice();
	case RwRenderSystemState::RW_SHUTDOWN_RENDER_DEVICE:
		break;
	default:
		break;
	}
	return BaseEventHandler(static_cast<int>(State),a2,a3,a4);
}

bool CRwD3DEngine::BaseEventHandler(int State, int * a2, void * a3, int a4)
{
	return RwD3DSystem(State,a2,a3,a4);
}

bool CRwD3DEngine::Create(HWND* pHWND)
{
	HRESULT errorCode=0;
	// copy window handle and create direct3D
	RwHWnd = *pHWND;
	pD3D = Direct3DCreate9(D3D9b_SDK_VERSION);
	if (!pD3D) {
		m_pDebug->printError("Failed to create pD3D.");
		return false;
	}

	// get adapter count
	UINT AdapterCount = pD3D->GetAdapterCount();
	if (AdapterCount <= 0) {
		m_pDebug->printError("No adapters found.");
		pD3D->Release(); pD3D = nullptr; return false;
	}
	RwD3DAdapterIndex = 0;
	RwD3DDevType = D3DDEVTYPE_HAL;

	// get device capability
	for (errorCode = pD3D->GetDeviceCaps(RwD3DAdapterIndex, RwD3DDevType, &RwD3D9DeviceCaps) < 0; (RwD3DAdapterIndex < AdapterCount) && errorCode; ++RwD3DAdapterIndex)
		;
	if (errorCode)
	{
		m_pDebug->printError("Can't get device capability.");
		pD3D->Release(); pD3D = nullptr; return false;
	}

	// get adapter mode count for each format
	RwD3DAdapterModeCount = 0;
	for (auto fmt: m_aBackBufferFormat)
		RwD3DAdapterModeCount += pD3D->GetAdapterModeCount(RwD3DAdapterIndex, fmt);

	pD3D->GetAdapterDisplayMode(RwD3DAdapterIndex, &RwD3DDisplayMode);
	switch (RwD3DDisplayMode.Format)
	{
	case 0x15:	case 0x16: case 0x23:
		RwD3DDepth = 32;
		break;
	case 0x17: case 0x18: case 0x19:
		RwD3DDepth = 16;
		break;
	default:
		RwD3DDepth = 0;
	}

	RwD3DbFullScreen = false;
	RwD3D9CurrentModeIndex = 0;
	D3D9CalculateMaxMultisamplingLevels();
	return true;
}

bool CRwD3DEngine::Shutdown()
{
	if (aRwD3DDisplayMode)
	{
		free(aRwD3DDisplayMode);
		aRwD3DDisplayMode = 0;
		RwD3DDisplayModeCount = 0;
	}
	SAFE_RELEASE(pD3D);
	return true;
}
void CRwD3DEngine::m_setPresentParameters(const D3DDISPLAYMODE& CurrentDisplayMode) {
	RwD3DPresentParams.Windowed = 0;
	RwD3DPresentParams.SwapEffect = D3DSWAPEFFECT_FLIP;
	RwD3DPresentParams.BackBufferCount = 1;
	RwD3DPresentParams.FullScreen_RefreshRateInHz = 60;
	RwD3DPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	RwD3DPresentParams.BackBufferWidth = CurrentDisplayMode.Width;
	RwD3DPresentParams.BackBufferHeight = CurrentDisplayMode.Height;
	RwD3DPresentParams.BackBufferFormat = CurrentDisplayMode.Format;
	RwD3DPresentParams.EnableAutoDepthStencil = 1;
	RwD3DPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;
	dword_C97C3C = 1;
	RwD3DPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	RwD3DPresentParams.MultiSampleQuality = 0;
	dword_C9BEFC = 32;
}
bool CRwD3DEngine::CreateRenderDevice()
{
	DWORD BehaviorFlags = 0;HRESULT hr;
	D3DDISPLAYMODE CurrentDisplayMode;
	pD3D->GetAdapterDisplayMode(RwD3DAdapterIndex, &CurrentDisplayMode);
	memset(&RwD3DPresentParams, 0, sizeof(RwD3DPresentParams));
	m_setPresentParameters(CurrentDisplayMode);
	RwD3DPresentParams.hDeviceWindow = RwHWnd;
	RwD3DPresentParams.Flags = 0;

	pD3D->GetDeviceCaps(RwD3DAdapterIndex, RwD3DDevType, &RwD3D9DeviceCaps);

	if (EnableMultithreadSafe)
		BehaviorFlags = D3DCREATE_MULTITHREADED;
	if (RwD3D9DeviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		if (RwD3D9DeviceCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
			BehaviorFlags |=  D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		if (EnableSoftwareVertexProcessing)
		{
			RwD3D9DeviceCaps.DevCaps &= ~D3DDEVCAPS_HWTRANSFORMANDLIGHT;
			RwD3D9DeviceCaps.DeclTypes = 0;
		}
	}

	if ((hr=pD3D->CreateDevice(RwD3DAdapterIndex, RwD3DDevType, RwHWnd, BehaviorFlags, &RwD3DPresentParams, &pD3DDevice)) < 0)
	{
		std::string sError = "Create Device Failed.\n";
		sError += "Error code: ";
		sError += to_string(hr);
		sError += '\n';
		m_pDebug->printError(sError);

		SystemStarted = 0;
		return false;
	} 
	else
	{
		m_pDebug->printMsg("Device Created.");

		pD3DDevice->GetRenderTarget(0, &RwD3D9RenderSurface);
		RwD3D9RenderSurface->Release();
		pD3DDevice->GetDepthStencilSurface(&RwD3D9DepthStencilSurface);
		RwD3D9DepthStencilSurface->Release();
		CurrentDepthStencilSurface = 0;

		for (auto i = 0;i < 4;i++)
			CurrentRenderSurface[i] = 0;

		rwD3D9InitLastUsedObjects();
		rwD3D9InitMatrixList();
		MaxNumLights = 0;
		if (LightsCache)
		{
			free(LightsCache);
			LightsCache = 0;
		}
		_rwD3D9RasterOpen();
		_rwD3D9Im2DRenderOpen();
		_rwD3D9RenderStateOpen();
		_rwD3D9VertexBufferManagerOpen();
		SystemStarted = 1;
		return true;
	}
}

bool CRwD3DEngine::ShutdownRenderDevice()
{
	return false;
}
