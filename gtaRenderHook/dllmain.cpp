// dllmain.cpp : Defines the entry point for the DLL application.
#include "CDebug.h"
#include "RwRenderEngine.h"
#include "stdafx.h"
CDebug* g_pDebug;
CIRwRenderEngine* g_pRwCustomEngine;
D3DFORMAT RwD3DSupportedFmts[3] = { D3DFMT_R5G6B5, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10 };

bool CreateD3D(int iD3DID,HWND* ptrHWND) {
	switch (iD3DID)
	{
	default:
		RwHWnd = *ptrHWND;
		pD3D = Direct3DCreate9(D3D9b_SDK_VERSION);
		if (!pD3D) {
			g_pDebug->printError("Failed to create pD3D.");
			return false;
		}
		UINT AdapterCount = pD3D->GetAdapterCount();
		if (AdapterCount <= 0) {
			g_pDebug->printError("No adapters found.");
			pD3D->Release();pD3D = NULL;
			return false;
		}
		RwD3DAdapterIndex = 0;
		RwD3DDevType = D3DDEVTYPE_HAL;
		break;
	}
	return true;
}

void CreateDisplayModeList() {
	if(aRwD3DDisplayMode)
		aRwD3DDisplayMode = (RwDisplayMode*)realloc(aRwD3DDisplayMode,4 * (5 * RwD3DAdapterModeCount + 5));
	else
		aRwD3DDisplayMode = (RwDisplayMode*)malloc(4 * (5 * RwD3DAdapterModeCount + 5));
	pD3D->GetAdapterDisplayMode(RwD3DAdapterIndex, &aRwD3DDisplayMode[0]);
	switch (aRwD3DDisplayMode[0].Format)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A2R10G10B10:
		aRwD3DDisplayMode[0].bFullscreen = 1;
		RwD3DDisplayModeCount = 1;
		break;
	default:
		RwD3DDisplayModeCount = 0;
		break;
	}
	for (int i = 0; i < 3; i++)
	{
		int AdapterCount = pD3D->GetAdapterModeCount(RwD3DAdapterIndex, RwD3DSupportedFmts[i]);
		//fprintf(debug->hLogFile, "Adapter mode count: %i.\n", AdapterCount);
		if (AdapterCount > 0) {
			for (int j = 0; j < AdapterCount; j++)
			{
				pD3D->EnumAdapterModes(RwD3DAdapterIndex, RwD3DSupportedFmts[i], j, &aRwD3DDisplayMode[RwD3DDisplayModeCount]);
				/*fprintf(debug->hLogFile, "Display mode format: %i.\nDisplay mode size :%ix%i\nDisplay mode refresh rate :%i\nDisplay mode full screen :%i\n", 
					RwD3DSupportedFmts[i], aRwD3DDisplayMode[RwD3DDisplayModeCount].Width, aRwD3DDisplayMode[RwD3DDisplayModeCount].Height, 
					aRwD3DDisplayMode[RwD3DDisplayModeCount].RefreshRate, aRwD3DDisplayMode[RwD3DDisplayModeCount].bFullscreen);*/
				RwD3DDisplayModeCount++;
			}
		}
	}
}

bool InitDisplayModeList(int AdapterID) {
	RwD3DAdapterIndex = AdapterID;
	RwD3DAdapterModeCount = 0;
	for (int i = 0; i < 3; i++)
		RwD3DAdapterModeCount += pD3D->GetAdapterModeCount(RwD3DAdapterIndex, RwD3DSupportedFmts[i]);
	pD3D->GetAdapterDisplayMode(RwD3DAdapterIndex, &RwD3DDisplayMode);
	CreateDisplayModeList();
	RwD3DMaxMultisamplingLevels = RwD3DMaxMultisamplingLevelsNonMask = RwD3DSelectedMultisamplingLevels = RwD3DSelectedMultisamplingLevelsNonMask = 1;
	return true;
}

bool GetVideoMode(int DisplayModeID,RwVideoMode* outVideoMode){
	int depth;
	if (!aRwD3DDisplayMode) CreateDisplayModeList();
	if (DisplayModeID < 0 || DisplayModeID >= RwD3DDisplayModeCount)
		return false;
	outVideoMode->m_iWidth = aRwD3DDisplayMode[DisplayModeID].Width;
	outVideoMode->m_iHeight = aRwD3DDisplayMode[DisplayModeID].Height;

	switch (aRwD3DDisplayMode[DisplayModeID].Format)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A2R10G10B10:
		outVideoMode->m_iDepth = 32;
		break;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
		outVideoMode->m_iDepth = 16;
		break;
	default:
		outVideoMode->m_iDepth = 0;
		break;
	}
	outVideoMode->m_iAdapter = aRwD3DDisplayMode[DisplayModeID].bFullscreen;
	outVideoMode->m_sRefreshRate = aRwD3DDisplayMode[DisplayModeID].RefreshRate;
	switch (aRwD3DDisplayMode[DisplayModeID].Format)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_A2R10G10B10:
		outVideoMode->m_uiFormat = 1280;
		break;
	case D3DFMT_X8R8G8B8:
		outVideoMode->m_uiFormat = 1536;
		break;
	case D3DFMT_R5G6B5:
		outVideoMode->m_uiFormat = 512;
		break;
	case D3DFMT_A1R5G5B5:
		outVideoMode->m_uiFormat = 256;
		break;
	case D3DFMT_X1R5G5B5:
		outVideoMode->m_uiFormat = 2560;
		break;
	default:
		outVideoMode->m_uiFormat = 0;
		break;
	}
	return true;
}

void SetPresentParameters(D3DDISPLAYMODE* CurrentDisplayMode) {
	RwD3DPresentParams.Windowed = 0;
	RwD3DPresentParams.SwapEffect = D3DSWAPEFFECT_FLIP;
	RwD3DPresentParams.BackBufferCount = 1;
	RwD3DPresentParams.FullScreen_RefreshRateInHz = 60;
	RwD3DPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	RwD3DPresentParams.BackBufferWidth = CurrentDisplayMode->Width;
	RwD3DPresentParams.BackBufferHeight = CurrentDisplayMode->Height;
	RwD3DPresentParams.BackBufferFormat = CurrentDisplayMode->Format;
	RwD3DPresentParams.EnableAutoDepthStencil = 1;
	RwD3DPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;
	dword_C97C3C = 1;
	RwD3DPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	RwD3DPresentParams.MultiSampleQuality = 0;
	dword_C9BEFC = 32;
}
bool Create3DDevice() {
	DWORD BehaviorFlags = 0;HRESULT hr;
	D3DDISPLAYMODE CurrentDisplayMode;
	pD3D->GetAdapterDisplayMode(RwD3DAdapterIndex, &CurrentDisplayMode);
	memset(&RwD3DPresentParams, 0, sizeof(RwD3DPresentParams));
	SetPresentParameters(&CurrentDisplayMode);
	RwD3DPresentParams.hDeviceWindow = RwHWnd;
	RwD3DPresentParams.Flags = 0;

	pD3D->GetDeviceCaps(RwD3DAdapterIndex, RwD3DDevType, &RwD3D9DeviceCaps);

	if (EnableMultithreadSafe)
		BehaviorFlags = D3DCREATE_MULTITHREADED;
	if (RwD3D9DeviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		if (RwD3D9DeviceCaps.DevCaps & 0x100000)
			BehaviorFlags |= (D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING);
		else
			BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		if (EnableSoftwareVertexProcessing)
		{
			RwD3D9DeviceCaps.DevCaps &= 0xFFFEFFFF;
			*(int*)(0xC9BFEC) = 0;
		}
	}
	hr = pD3D->CreateDevice(RwD3DAdapterIndex, RwD3DDevType, RwHWnd, BehaviorFlags, &RwD3DPresentParams, &pD3DDevice);
	if (hr >= 0)
	{
		g_pDebug->printMsg("Device Created.");
		pD3DDevice->GetRenderTarget(0, &RwD3D9RenderSurface);
		RwD3D9RenderSurface->Release();
		pD3DDevice->GetDepthStencilSurface(&RwD3D9DepthStencilSurface);
		RwD3D9DepthStencilSurface->Release();
		CurrentDepthStencilSurface = 0;
		for(auto i=0;i<4;i++)
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
	else {
		std::string sError = "Create Device Failed.\n";
		sError += "Error code: ";
		sError += hr;
		sError += '\n';
		g_pDebug->printError(sError);

		SystemStarted = 0;
		return false;
	}
}
HGLRC ourOpenGLRenderingContext;
bool CreateOpenGLDevice(HWND* ptrHWND) {
	RwHWnd = *ptrHWND;
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,            //The kind of frame buffer. RGBA or palette.
		32,                        //Color depth of the frame buffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                        //Number of bits for the depth buffer
		8,                        //Number of bits for the stencil buffer
		0,                        //Number of Aux buffers in the frame buffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC ourWindowHandleToDeviceContext = GetDC(RwHWnd);

	int  letWindowsChooseThisPixelFormat;
	letWindowsChooseThisPixelFormat = ChoosePixelFormat(ourWindowHandleToDeviceContext, &pfd);
	SetPixelFormat(ourWindowHandleToDeviceContext, letWindowsChooseThisPixelFormat, &pfd);

	ourOpenGLRenderingContext = wglCreateContext(ourWindowHandleToDeviceContext);
	wglMakeCurrent(ourWindowHandleToDeviceContext, ourOpenGLRenderingContext);
	glViewport(0, 0, 800, 600);
	glClear(0);
	MessageBoxA(0, (char*)glGetString(GL_VERSION), "OPENGL VERSION", 0);

	return true;
}
bool D3DSystem(int State, int* a2, void* a3, int a4){
	int depth;
	switch (State)
	{
	case 4:
		g_pDebug->printMsg("RW copies deviceHandle.");
		return RwD3DSystem(State, a2, a3, a4);
	case 0:
		g_pDebug->printMsg("RW creates device.");
		return CreateOpenGLDevice((HWND*)a3);
		//return CreateD3D(0,(HWND*)a3);
	case 13:
		(*a2) = pD3D->GetAdapterCount();
		g_pDebug->printMsg("RW get AdapterCount.");
		/*fprintf(debug->hLogFile, "Adapter count: %i.\n", *a2);*/
		return true;
	case 14:
		D3DADAPTER_IDENTIFIER9 AdpterID;
		pD3D->GetAdapterIdentifier(a4, 0, &AdpterID);
		g_pDebug->printMsg("RW get AdapterIdentifier.");
		/*fprintf(debug->hLogFile, "AdapterIdentifier Description: %s.\n", AdpterID.Description);*/
		return true;
	case 15:
		g_pDebug->printMsg("RW get AdapterID.");
		(*a2) = RwD3DAdapterIndex;
		/*fprintf(debug->hLogFile, "AdapterID: %i.\n", *a2);*/
		return true;
	case 16:
		g_pDebug->printMsg("RW init Display Mode list.");
		return InitDisplayModeList(a4);
	case 5:
		g_pDebug->printMsg("RW get display mode count.");
		if (!aRwD3DDisplayMode) CreateDisplayModeList();
		*a2 = RwD3DDisplayModeCount;
		return true;
	case 6:
		g_pDebug->printMsg("RW get video mode.");
		return GetVideoMode(a4,(RwVideoMode*)a2);
	case 7:
		if (a4 < 0 || a4 >= RwD3DDisplayModeCount)
			return false;
		RwD3D9CurrentModeIndex = a4;
		RwD3DDisplayMode.Width = aRwD3DDisplayMode[RwD3D9CurrentModeIndex].Width;
		RwD3DDisplayMode.Height = aRwD3DDisplayMode[RwD3D9CurrentModeIndex].Height;
		RwD3DDisplayMode.RefreshRate = aRwD3DDisplayMode[RwD3D9CurrentModeIndex].RefreshRate;
		RwD3DDisplayMode.Format = aRwD3DDisplayMode[RwD3D9CurrentModeIndex].Format;
		RwD3DbFullScreen = aRwD3DDisplayMode[RwD3D9CurrentModeIndex].bFullscreen;
		switch (aRwD3DDisplayMode[RwD3D9CurrentModeIndex].Format)
		{
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A2R10G10B10:
			depth = 32;
			break;
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
			depth = 16;
			break;
		default:
			depth = 0;
			break;
		}
		RwD3DDepth = depth;
		return true;
	case 2:
		g_pDebug->printMsg("RW create device.");
		return Create3DDevice();
	case 17:
		g_pDebug->printMsg("RW initializes GPU-CPU stuff.");
		return RwD3DSystem(State, a2, a3, a4);
	case 10:
		g_pDebug->printMsg("RW get current video mode index.");
		*a2 = RwD3D9CurrentModeIndex;
		return 1;
	case 8:
		g_pDebug->printMsg("RW shows/hides window.");
		if (!pD3DDevice)
			return false;
		if (RwD3DPresentParams.Windowed)
			return true;
		ShowWindow(RwHWnd, a4? 9 : 0);
		return true;
	case 1:
		g_pDebug->printMsg("RW shutdown.");

		wglDeleteContext(ourOpenGLRenderingContext);
		//PostQuitMessage(0);
		/*if (aRwD3DDisplayMode)
		{
			free(aRwD3DDisplayMode);
			aRwD3DDisplayMode = 0;
			RwD3DDisplayModeCount = 0;
		}
		if (pD3D)
		{
			pD3D->Release();
			pD3D = 0;
		}*/
		return true;
	default:
		/*fprintf(debug->hLogFile, "Current D3DSystem State is: %i.\n", State);*/
		return RwD3DSystem(State, a2, a3, a4);
	}
}
bool D3DSystemHook(int State, int* a2, void* a3, int a4) {
	return g_pRwCustomEngine->EventHandlingSystem((RwRenderSystemState)State, a2, a3, a4);
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_pDebug = new CDebug("debug.log");
		g_pRwCustomEngine = new CRwD3DEngine(g_pDebug);
		SetPointer(0x8E249C, D3DSystemHook);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete g_pRwCustomEngine;
		delete g_pDebug;
		break;
	}
	return TRUE;
}

