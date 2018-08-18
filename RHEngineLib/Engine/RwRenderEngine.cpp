#include "stdafx.h"
#include "D3D11Impl\D3D11Renderer.h"
#include "D3D12Impl\D3D12Renderer.h"
#include "VulkanImpl\VulkanRenderer.h"
#include "RwRenderEngine.h"
#include "..\DebugUtils\DebugLogger.h"

std::unique_ptr<RHEngine::RwRenderEngine> RHEngine::g_pRWRenderEngine;
bool RHEngine::RwRenderEngine::Focus(bool)
{
	return false;
}

bool RHEngine::RwRenderEngine::GetMode(int &)
{
	return false;
}

bool RHEngine::RwRenderEngine::Standards(int * standards, int numStandardsFunctions)
{
	RwInt32             i;
	RwInt32             numDriverFunctions;
	RwStandardFunc     *standardFunctions;
	RwStandard          rwStandards[] = {
		/* Camera ops */
	/*{ rwSTANDARDCAMERABEGINUPDATE, _rwD3D9CameraBeginUpdate },
	{ rwSTANDARDCAMERAENDUPDATE, _rwD3D9CameraEndUpdate },
	{ rwSTANDARDCAMERACLEAR, _rwD3D9CameraClear },*/

	/* Raster/Pixel operations */
	/*{ rwSTANDARDRASTERSHOWRASTER, _rwD3D9RasterShowRaster },
	{ rwSTANDARDRGBTOPIXEL, _rwD3D9RGBToPixel },
	{ rwSTANDARDPIXELTORGB, _rwD3D9PixelToRGB },
	{ rwSTANDARDRASTERSETIMAGE, _rwD3D9RasterSetFromImage },
	{ rwSTANDARDIMAGEGETRASTER, _rwD3D9ImageGetFromRaster },*/

	/* Raster creation and destruction */
	{ rwSTANDARDRASTERDESTROY, [](void* pOut, void* pInOut, RwInt32 nI)->RwBool { return g_pRWRenderEngine->RasterDestroy((RwRaster*)pInOut); } },
	{ rwSTANDARDRASTERCREATE, [](void* pOut, void* pInOut, RwInt32 nI)->RwBool { return g_pRWRenderEngine->RasterCreate((RwRaster*)pInOut,nI); } },

	/* Finding about a raster type */
	/*{ rwSTANDARDIMAGEFINDRASTERFORMAT, _rwD3D9ImageFindRasterFormat },*/

	/* Texture operations */
	/*{ rwSTANDARDTEXTURESETRASTER, _rwD3D9TextureSetRaster },*/

	/* Locking and releasing */
	/*{ rwSTANDARDRASTERLOCK, _rwD3D9RasterLock },
	{ rwSTANDARDRASTERUNLOCK, _rwD3D9RasterUnlock },
	{ rwSTANDARDRASTERLOCKPALETTE, _rwD3D9RasterLockPalette },
	{ rwSTANDARDRASTERUNLOCKPALETTE, _rwD3D9RasterUnlockPalette },*/

	/* Raster operations */
	/*{ rwSTANDARDRASTERCLEAR, _rwD3D9RasterClear },
	{ rwSTANDARDRASTERCLEARRECT, _rwD3D9RasterClearRect },*/

	/* !! */
	/*{ rwSTANDARDRASTERRENDER, _rwD3D9RasterRender },
	{ rwSTANDARDRASTERRENDERSCALED, _rwD3D9RasterRenderScaled },
	{ rwSTANDARDRASTERRENDERFAST, _rwD3D9RasterRenderFast },*/

	/* Setting the context */
	/*{ rwSTANDARDSETRASTERCONTEXT, _rwD3D9SetRasterContext },*/

	/* Creating sub rasters */
	/*{ rwSTANDARDRASTERSUBRASTER, _rwD3D9RasterSubRaster },*/

	/* Hint for rendering order */
	/*{rwSTANDARDHINTRENDERF2B,         _rwD3D9HintRenderFront2Back}, *

	/* Native texture serialization */
	/*{ rwSTANDARDNATIVETEXTUREGETSIZE, _rwD3D9NativeTextureGetSize },
	{ rwSTANDARDNATIVETEXTUREWRITE, _rwD3D9NativeTextureWrite },
	{ rwSTANDARDNATIVETEXTUREREAD, _rwD3D9NativeTextureRead },*/

	/* Raster Mip Levels */
	/*{ rwSTANDARDRASTERGETMIPLEVELS, _rwD3D9RasterGetMipLevels }*/
	};

	//RWFUNCTION(RWSTRING("D3D9DeviceSystemStandards"));

	standardFunctions = (RwStandardFunc *)standards;
	numDriverFunctions = sizeof(rwStandards) / sizeof(RwStandard);

	/* Clear out all of the standards initially */
	for (i = 0; i < numStandardsFunctions; i++)
	{
		standardFunctions[i] = [](void* pOut, void* pInOut, RwInt32 nI)->RwBool { return true; };
	}

	/* Fill in all of the standards */
	while (numDriverFunctions--)
	{
		if ((rwStandards->nStandard < numStandardsFunctions) &&
			(rwStandards->nStandard >= 0))
		{
			standardFunctions[rwStandards[numDriverFunctions].
				nStandard] =
				rwStandards[numDriverFunctions].fpStandard;
		}
	}
	return true;
}

bool RHEngine::RwRenderEngine::GetTexMemSize(int &)
{
	return false;
}

bool RHEngine::RwRenderEngine::GetNumSubSystems(int &num)
{
	return m_pRenderer->GetAdaptersCount(num);
}

bool RHEngine::RwRenderEngine::GetSubSystemInfo(RwSubSystemInfo & info , int n)
{
	std::wstring str;
	if (!m_pRenderer->GetAdapterInfo(n, str))
		return false;
	strncpy_s(info.name, FromRHString(str).c_str(), 80);
	return true;
}

bool RHEngine::RwRenderEngine::GetCurrentSubSystem(int &)
{
	return false;
}

bool RHEngine::RwRenderEngine::SetSubSystem(int n)
{
	return m_pRenderer->SetCurrentAdapter(n);
}

bool RHEngine::RwRenderEngine::GetNumOutputs(int a, int &n)
{
	return m_pRenderer->GetOutputCount(a,n);
}

bool RHEngine::RwRenderEngine::GetOutputInfo(std::wstring& info, unsigned int n)
{
	return m_pRenderer->GetOutputInfo(n,info);
}

bool RHEngine::RwRenderEngine::GetCurrentOutput(int &n)
{
	return m_pRenderer->GetCurrentOutput(n);
}

bool RHEngine::RwRenderEngine::SetOutput(int n)
{
	return m_pRenderer->SetCurrentOutput(n);
}

bool RHEngine::RwRenderEngine::GetMaxTextureSize(int &)
{
	return false;
}

bool RHEngine::RwRenderEngine::BaseEventHandler(int State, int * a2, void * a3, int a4)
{
	return false;
}

void RHEngine::RwRenderEngine::SetMultiSamplingLevels(int)
{
}

int RHEngine::RwRenderEngine::GetMaxMultiSamplingLevels()
{
	return 0;
}

bool RHEngine::RwRenderEngine::RenderStateSet(RwRenderState, UINT)
{
	return false;
}

bool RHEngine::RwRenderEngine::RenderStateGet(RwRenderState, UINT &)
{
	return false;
}

bool RHEngine::RwRenderEngine::RasterCreate(RwRaster * raster, UINT flags)
{
	/* Initialise structure to something sensible */
	raster->cType = (RwUInt8)(flags & rwRASTERTYPEMASK);
	raster->cFlags = (RwUInt8)(flags & ~rwRASTERTYPEMASK);
	raster->cpPixels = NULL;
	raster->palette = NULL;
	RHImageBufferType imageBufferType = RHImageBufferType::Unknown;
	if (raster->cType & rwRASTERTYPECAMERA)
		imageBufferType = RHImageBufferType::BackBuffer;
	/* Retrieve a pointer to internal raster */
	void** internalRaster = reinterpret_cast<void**>(((RwUInt8*)raster) + sizeof(RwRaster));

	*internalRaster = nullptr;

	*internalRaster = m_pRenderer->AllocateImageBuffer(raster->width, raster->height, imageBufferType);

	return *internalRaster != nullptr;
}

bool RHEngine::RwRenderEngine::RasterDestroy(RwRaster * raster)
{
	/* Retrieve a pointer to internal raster */
	void** internalRaster = reinterpret_cast<void**>(((RwUInt8*)raster) + sizeof(RwRaster));

    RHImageBufferType imageBufferType = RHImageBufferType::Unknown;
    if (raster->cType & rwRASTERTYPECAMERA)
        imageBufferType = RHImageBufferType::BackBuffer;

	return m_pRenderer->FreeImageBuffer(*internalRaster, imageBufferType);
}

bool RHEngine::RwRenderEngine::RasterLock(RwRaster * raster, UINT flags, void ** data)
{
	return false;
}

bool RHEngine::RwRenderEngine::RasterUnlock(RwRaster * raster)
{
	return false;
}

bool RHEngine::RwRenderEngine::RasterShowRaster(RwRaster * raster, UINT flags)
{
	/* Retrieve a pointer to internal raster */
	void** internalRaster = reinterpret_cast<void**>(((RwUInt8*)raster) + sizeof(RwRaster));
	return m_pRenderer->Present(*internalRaster);
}

bool RHEngine::RwRenderEngine::NativeTextureRead(RwStream * stream, RwTexture ** tex)
{
	return false;
}

bool RHEngine::RwRenderEngine::CameraClear(RwCamera * camera, RwRGBA * color, RwInt32 flags)
{
	void **frameBufferInternal = reinterpret_cast<void**>(((RwUInt8*)camera->frameBuffer) + sizeof(RwRaster));

	float clearColor[] = { color->red / 255.0f, color->green / 255.0f, color->blue / 255.0f, color->alpha / 255.0f };

	if(camera->frameBuffer && *frameBufferInternal && flags & rwCAMERACLEARIMAGE)
		m_pRenderer->ClearImageBuffer(RHImageClearType::Color, *frameBufferInternal, clearColor);
	return true;
}

bool RHEngine::RwRenderEngine::CameraBeginUpdate(RwCamera * camera)
{	
	void **frameBufferInternal = reinterpret_cast<void**>(((RwUInt8*)camera->frameBuffer) + sizeof(RwRaster));

	// Compute matricies
	// Resize window if needed
	// Set render targets
	if (camera->frameBuffer && *frameBufferInternal) {
		if (!m_pRenderer->RequestSwapChainImage(*frameBufferInternal))
			return false;
		if (!m_pRenderer->BeginCommandList(nullptr))
			return false;
		m_pRenderer->BindImageBuffers(RHImageBindType::RenderTarget, { {0, *frameBufferInternal } });
	}
	// Set viewport
	return true;
}

bool RHEngine::RwRenderEngine::CameraEndUpdate(RwCamera * camera)
{
	void **frameBufferInternal = reinterpret_cast<void**>(((RwUInt8*)camera->frameBuffer) + sizeof(RwRaster));
	if (camera->frameBuffer == nullptr || *frameBufferInternal == nullptr)
		return false;
	return m_pRenderer->PresentSwapChainImage(*frameBufferInternal) && m_pRenderer->EndCommandList(nullptr);
}

void RHEngine::RwRenderEngine::SetRenderTargets(RwRaster ** rasters, RwRaster * zBuffer, RwUInt32 rasterCount)
{
}

bool RHEngine::RwRenderEngine::Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex * vertices, RwUInt32 numVertices)
{
	return false;
}

bool RHEngine::RwRenderEngine::Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex * vertices, RwUInt32 numVertices, RwImVertexIndex * indices, RwInt32 numIndices)
{
	return false;
}

RwBool RHEngine::RwRenderEngine::Im3DSubmitNode()
{
	return RwBool();
}

void RHEngine::RwRenderEngine::SetTexture(RwTexture * tex, int Stage)
{
}

bool RHEngine::RwRenderEngine::AtomicAllInOneNode(RxPipelineNode * self, const RxPipelineNodeParam * params)
{
	return false;
}

bool RHEngine::RwRenderEngine::SkinAllInOneNode(RxPipelineNode * self, const RxPipelineNodeParam * params)
{
	return false;
}

void RHEngine::RwRenderEngine::DefaultRenderCallback(RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags)
{
}

RwBool RHEngine::RwRenderEngine::DefaultInstanceCallback(void * object, RxD3D9ResEntryHeader * resEntryHeader, RwBool reinstance)
{
	return RwBool();
}

bool RHEngine::RwRenderEngine::EventHandlingSystem(RwRenderSystemRequest request, int * pOut, void * pInOut, int nIn)
{
	std::wstring s = L"Event handled: "; s += ToRHString(request);
	RHDebug::DebugLogger::Log(s.c_str(), RHDebug::LogLevel::Info);
	switch (request)
	{
	case RwRenderSystemRequest::rwDEVICESYSTEMOPEN:
		return Open(*static_cast<HWND*>(pInOut));
	case RwRenderSystemRequest::rwDEVICESYSTEMCLOSE:
		return Close();
	case RwRenderSystemRequest::rwDEVICESYSTEMSTART:
		return Start();
	case RwRenderSystemRequest::rwDEVICESYSTEMSTOP:
		return Stop();
	case RwRenderSystemRequest::rwDEVICESYSTEMGETNUMMODES:
		return GetNumModes(*pOut);
	case RwRenderSystemRequest::rwDEVICESYSTEMGETMODEINFO:
		return GetModeInfo(*(RwVideoMode*)(pOut), nIn);
	case RwRenderSystemRequest::rwDEVICESYSTEMUSEMODE:
		return UseMode(nIn);
	case RwRenderSystemRequest::rwDEVICESYSTEMFOCUS:
		return Focus(nIn != 0);
	case RwRenderSystemRequest::rwDEVICESYSTEMGETMODE:
		return GetMode(*pOut);
	case RwRenderSystemRequest::rwDEVICESYSTEMSTANDARDS:
		return Standards((int*)pOut, nIn);
	case RwRenderSystemRequest::rwDEVICESYSTEMGETNUMSUBSYSTEMS:
		return GetNumSubSystems(*(int*)pOut);
	case RwRenderSystemRequest::rwDEVICESYSTEMGETSUBSYSTEMINFO:
		return GetSubSystemInfo(*(RwSubSystemInfo*)pOut, nIn);
	case RwRenderSystemRequest::rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
		return GetCurrentSubSystem(*pOut);
	case RwRenderSystemRequest::rwDEVICESYSTEMSETSUBSYSTEM:
		return SetSubSystem(nIn);
	case RwRenderSystemRequest::rwDEVICESYSTEMGETTEXMEMSIZE:
		return GetTexMemSize(*pOut);
	case RwRenderSystemRequest::rwDEVICESYSTEMREGISTER:
	case RwRenderSystemRequest::rwDEVICESYSTEMINITPIPELINE:
	case RwRenderSystemRequest::rwDEVICESYSTEMFINALIZESTART:
	case RwRenderSystemRequest::rwDEVICESYSTEMINITIATESTOP:
	case RwRenderSystemRequest::rwDEVICESYSTEMRXPIPELINEREQUESTPIPE:
	case RwRenderSystemRequest::rwDEVICESYSTEMGETMETRICBLOCK:
		break;
	case RwRenderSystemRequest::rwDEVICESYSTEMGETMAXTEXTURESIZE:
		return GetMaxTextureSize(*pOut);
	case RwRenderSystemRequest::rwDEVICESYSTEMGETID:
		*pOut = 2;
		return true;
	default:
		break;
	}
	return BaseEventHandler(static_cast<int>(request), pOut, pInOut, nIn);
}

std::wstring RHEngine::ToRHString(RwRenderSystemRequest req)
{
	switch (req)
	{
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMOPEN:
		return TEXT("Open");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMCLOSE:
		return TEXT("Close");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMSTART:
		return TEXT("Start");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMSTOP:
		return TEXT("Stop");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMREGISTER:
		return TEXT("Register");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETNUMMODES:
		return TEXT("GetNumModes");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETMODEINFO:
		return TEXT("GetModeInfo");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMUSEMODE:
		return TEXT("UseMode");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMFOCUS:
		return TEXT("Focus");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMINITPIPELINE:
		return TEXT("InitPipeLine");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETMODE:
		return TEXT("GetMode");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMSTANDARDS:
		return TEXT("Standards");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETTEXMEMSIZE:
		return TEXT("GetTexMemSize");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETNUMSUBSYSTEMS:
		return TEXT("GetNumSubSystems");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETSUBSYSTEMINFO:
		return TEXT("GetSubSystemInfo");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
		return TEXT("GetCurrentSubSystem");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMSETSUBSYSTEM:
		return TEXT("SetSubSystem");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMFINALIZESTART:
		return TEXT("InitiateStart");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMINITIATESTOP:
		return TEXT("InitiateStop");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETMAXTEXTURESIZE:
		return TEXT("GetMaxTextureSize");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMRXPIPELINEREQUESTPIPE:
		return TEXT("RequestPipe");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETMETRICBLOCK:
		return TEXT("GetMetricBlock");
	case RHEngine::RwRenderSystemRequest::rwDEVICESYSTEMGETID:
		return TEXT("GetID");
	default:
		return TEXT("Unknown");
	}
}

bool RHEngine::RwRenderEngine::Open(HWND window)
{
	HINSTANCE hInst = GetModuleHandle(NULL);
	switch (m_renderingAPI)
	{
	case RHEngine::RHRenderingAPI::DX11:
		m_pRenderer = std::unique_ptr<IRenderer>(new D3D11Renderer(window, hInst));
		break;
	case RHEngine::RHRenderingAPI::DX12:
		m_pRenderer = std::unique_ptr<IRenderer>(new D3D12Renderer(window, hInst));
		break;
	case RHEngine::RHRenderingAPI::Vulkan:
		m_pRenderer = std::unique_ptr<IRenderer>(new VulkanRenderer(window, hInst));
		break;
	case RHEngine::RHRenderingAPI::OpenGL:
		m_pRenderer = nullptr;//std::unique_ptr<IRenderer>(new GLRenderer(window));
		return false;
	default:
		m_pRenderer = nullptr;
		return false;
	}
	return true;
}

bool RHEngine::RwRenderEngine::Close()
{
	m_pRenderer.reset();
	return true;
}

bool RHEngine::RwRenderEngine::Start()
{
	return m_pRenderer->InitDevice();
}

bool RHEngine::RwRenderEngine::Stop()
{
	if(m_pRenderer)
		m_pRenderer->ShutdownDevice();
	return true;
}

bool RHEngine::RwRenderEngine::GetNumModes(int & n)
{
	return m_pRenderer->GetDisplayModeCount(0, n) && m_pRenderer->SetCurrentOutput(0);
}

bool RHEngine::RwRenderEngine::GetModeInfo(RwVideoMode & videomode, int n)
{
	DisplayModeInfo info;
	if(!m_pRenderer->GetDisplayModeInfo(n, info))
		return false;
	videomode.width = info.width;
	videomode.height = info.height;
	videomode.depth = 32;
	videomode.refRate = info.refreshRate;
	videomode.format = rwRASTERFORMATDEFAULT;

	return true;
}

bool RHEngine::RwRenderEngine::UseMode(int n)
{
	return m_pRenderer->SetCurrentDisplayMode(n);
}
