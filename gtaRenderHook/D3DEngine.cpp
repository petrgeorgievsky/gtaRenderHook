#include "stdafx.h"
#include "D3DEngine.h"
#include "RwRenderEngine.h"



bool CRwD3DEngine::BaseEventHandler(int State, int * a2, void * a3, int a4)
{
	return RwD3DSystem(State, a2, a3, a4);
}

bool CRwD3DEngine::Open(HWND* pHWND)
{
	bool errorCode = false;
	// copy window handle and create direct3D
	m_windowHandle = *pHWND;
	m_d3d9 = Direct3DCreate9(D3D9b_SDK_VERSION);
	if (!m_d3d9) {
		m_pDebug->printError("Failed to create m_d3d9.");
		return false;
	}

	// get adapter count
	UINT AdapterCount = m_d3d9->GetAdapterCount();
	if (AdapterCount <= 0) {
		m_pDebug->printError("No adapters found.");
		m_d3d9->Release(); m_d3d9 = nullptr; return false;
	}
	m_adapterIndex = 0;
	m_deviceType = D3DDEVTYPE_HAL;

	// get device capability
	for (errorCode = FAILED(m_d3d9->GetDeviceCaps(m_adapterIndex, m_deviceType, &m_deviceCaps)); (m_adapterIndex < AdapterCount) && errorCode; ++m_adapterIndex)
		;
	if (errorCode)
	{
		m_pDebug->printError("Can't get device capability.");
		m_d3d9->Release(); m_d3d9 = nullptr; return false;
	}

	// get adapter mode count for each format
	m_numAdapterModes = 0;
	for (auto fmt : m_aBackBufferFormat)
		m_numAdapterModes += m_d3d9->GetAdapterModeCount(m_adapterIndex, fmt);

	m_d3d9->GetAdapterDisplayMode(m_adapterIndex, &m_displayMode);
	m_depth = m_getDepthValue(m_displayMode.Format);

	m_windowed = true;
	m_currentModeIndex = 0;
	m_calculateMaxMSLevels();
	return true;
}

bool CRwD3DEngine::Close()
{
	if (m_displayModes)
	{
		free(m_displayModes);
		m_displayModes = 0;
		m_numDisplayModes = 0;
	}
	SAFE_RELEASE(m_d3d9);
	return true;
}

void CRwD3DEngine::m_setPresentParameters(const D3DDISPLAYMODE& CurrentDisplayMode) {
	m_presentParams.Windowed = false;
	m_presentParams.SwapEffect = D3DSWAPEFFECT_FLIP;
	m_presentParams.BackBufferCount = 1;
	m_presentParams.FullScreen_RefreshRateInHz = 60;
	m_presentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	
	m_presentParams.BackBufferWidth = CurrentDisplayMode.Width;
	m_presentParams.BackBufferHeight = CurrentDisplayMode.Height;
	m_presentParams.BackBufferFormat = CurrentDisplayMode.Format;
	m_presentParams.EnableAutoDepthStencil = 1;
	m_presentParams.AutoDepthStencilFormat = D3DFMT_D24S8;
	m_hasStencilBuffer = 1;
	m_presentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_presentParams.MultiSampleQuality = 0;
	m_ZBufferDepth = 32;
}

void CRwD3DEngine::m_setDepthStencilSurface(IDirect3DSurface9 * surf)
{
	if (m_currentDepthStencilSurface != surf)
	{
		m_currentDepthStencilSurface = surf;
		pD3DDevice->SetDepthStencilSurface(surf);
	}
}

void CRwD3DEngine::m_setRenderTarget(int index, IDirect3DSurface9 *surf)
{
	if (m_currentRenderSurface[index] != surf)
	{
		m_currentRenderSurface[index] = surf;
		pD3DDevice->SetRenderTarget(index,surf);
	}
}

void CRwD3DEngine::m_im2DRenderFlush()
{
	pD3DDevice->SetVertexDeclaration(m_vertexDeclIm2D);
	pD3DDevice->SetVertexShader(NULL);
	pD3DDevice->SetPixelShader(NULL);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 0);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
	pD3DDevice->SetStreamSource(0, m_currentVertexBuffer, 0, 28);
}

bool CRwD3DEngine::CameraClear(RwCamera *pOut, RwRGBA *pInOut, RwInt32 nI)
{
	m_pDebug->printMsg("Camera Clear call");
	RECT rect;
	RwRaster* fbParent = pOut->frameBuffer->parent,
			  *zBuffer;
	RwD3D9Raster* r = GetD3D9Raster(fbParent);

	if (r->swapChain) {
		m_pDebug->printMsg("Frame buffer has swap chain!");
		GetClientRect(*r->hwnd, &rect);
		if (rect.right == 0 && rect.bottom == 0)
			return false;
		else
		{
			if (rect.right != fbParent->originalWidth || rect.bottom != fbParent->originalHeight)
			{
				if (pOut->frameBuffer==fbParent)
				{
					RwD3D9CameraAttachWindow(pOut, r->hwnd);
					r = GetD3D9Raster(fbParent);
				}
				zBuffer = pOut->zBuffer;
			} 
			else
			{
				zBuffer = pOut->zBuffer;
				if (zBuffer && zBuffer == zBuffer->parent && (rect.right != zBuffer->width || rect.bottom != zBuffer->height))
				{
					RwRasterDestroy(pOut->zBuffer);
					pOut->zBuffer = nullptr;
					if(zBuffer=RwRasterCreate(rect.right,rect.bottom,0, rwRASTERTYPEZBUFFER))
						pOut->zBuffer = zBuffer;
				}
			}
			m_setRenderTarget(0, r->surface);
			m_setDepthStencilSurface(zBuffer ? GetD3D9Raster(zBuffer->parent)->surface : nullptr);
			D3DVIEWPORT9 vp{ 0,0,pOut->frameBuffer->width,pOut->frameBuffer->height,0,1 };
			pD3DDevice->SetViewport(&vp);
			pD3DDevice->Clear(NULL, nullptr, nI, D3DCOLOR_RGBA(pInOut->red, pInOut->green, pInOut->blue, pInOut->alpha), 0, NULL);
			return true;
		}
	}

	GetClientRect(m_windowHandle, &rect);

	if (rect.right == 0 || rect.bottom == 0 || IsIconic(m_windowHandle))
		return false;
	if (rect.right == m_displayMode.Width && rect.bottom == m_displayMode.Height)
	{
		if (fbParent->cType!=rwRASTERTYPECAMERATEXTURE)
		{
			m_pDebug->printMsg("Frame buffer isn't camera texture!");
			m_setRenderTarget(0, m_renderSurface);
			m_setDepthStencilSurface(m_depthStencilSurface);
			D3DVIEWPORT9 vp{ pOut->frameBuffer->nOffsetX,pOut->frameBuffer->nOffsetY,pOut->frameBuffer->width,pOut->frameBuffer->height,0,1 };
			pD3DDevice->SetViewport(&vp);
			pD3DDevice->Clear(NULL, nullptr, nI, D3DCOLOR_RGBA(pInOut->red, pInOut->green, pInOut->blue, pInOut->alpha), 0, NULL);
			return true;
		}
	}
	//D3D9DeviceReleaseVideoMemory();
	// ****** TODO: Handle other situations!
	m_pDebug->printMsg("CameraClear unhandled code!");
	return false;
}

bool CRwD3DEngine::CameraBeginUpdate(RwCamera *camera)
{
	m_pDebug->printMsg("Camera begin update call");
	RECT rect;
	dgGGlobals = camera;

	RwMatrixInvert(&RwD3D9D3D9ViewTransform, RwFrameGetLTM((RwFrame*)camera->object.object.parent));
	RwD3D9D3D9ViewTransform.right.x = -RwD3D9D3D9ViewTransform.right.x;
	RwD3D9D3D9ViewTransform.up.x = -RwD3D9D3D9ViewTransform.up.x;
	RwD3D9D3D9ViewTransform.at.x = -RwD3D9D3D9ViewTransform.at.x;
	RwD3D9D3D9ViewTransform.pos.x = -RwD3D9D3D9ViewTransform.pos.x;
	RwD3D9D3D9ViewTransform.flags = 0;
	RwD3D9D3D9ViewTransform.pad1 = 0;
	RwD3D9D3D9ViewTransform.pad2 = 0;
	RwD3D9D3D9ViewTransform.pad3 = 0x3F800000;
	SetTransform(D3DTS_VIEW, &RwD3D9D3D9ViewTransform);

	RwD3D9D3D9ProjTransform.right.x = camera->recipViewWindow.x;
	RwD3D9D3D9ProjTransform.up.y = camera->recipViewWindow.y;
	RwD3D9D3D9ProjTransform.at.x = camera->viewOffset.x * camera->recipViewWindow.x;
	RwD3D9D3D9ProjTransform.at.y = camera->viewOffset.y * camera->recipViewWindow.y;
	RwD3D9D3D9ProjTransform.pos.x = -(camera->viewOffset.x * camera->recipViewWindow.x);
	RwD3D9D3D9ProjTransform.pos.y = -(camera->viewOffset.y * camera->recipViewWindow.y);
	if (camera->projectionType == rwPARALLEL)
	{
		RwD3D9D3D9ProjTransform.at.z = 1.0f / (camera->farPlane - camera->nearPlane);
		RwD3D9D3D9ProjTransform.pad2 = 0;
		RwD3D9D3D9ProjTransform.pad3 = 0x3F800000;
	}
	else
	{
		RwD3D9D3D9ProjTransform.at.z = camera->farPlane / (camera->farPlane - camera->nearPlane);
		RwD3D9D3D9ProjTransform.pad2 = 0x3F800000;
		RwD3D9D3D9ProjTransform.pad3 = 0;
	}
	RwD3D9D3D9ProjTransform.pos.z = -(RwD3D9D3D9ProjTransform.at.z * camera->nearPlane);
	SetTransform(D3DTS_PROJECTION, &RwD3D9D3D9ProjTransform);
	RwD3D9ActiveViewProjTransform = 0;
	RwD3D9Raster *fbParent = GetD3D9Raster(camera->frameBuffer->parent);
	if (!fbParent->swapChain) {
		m_pDebug->printMsg("FB has no swap chain!");

		GetClientRect(m_windowHandle, &rect);

		if (rect.right==0 || rect.bottom==0 || IsIconic(m_windowHandle))
			return false;
		if (rect.right != m_displayMode.Width || rect.bottom != m_displayMode.Height) {
			m_pDebug->printMsg("CamBegUpd: Window don't have same size as display mode! Rect r:"+to_string(rect.right)+" b:"+to_string(rect.bottom));
			ReleaseVideoMemory();
			if (pD3DDevice->TestCooperativeLevel() >= 0 && m_presentParams.Windowed)
			{
				m_presentParams.BackBufferWidth = rect.right;
				m_presentParams.BackBufferHeight = rect.bottom;
			}
			if (pD3DDevice->Reset(&m_presentParams) >= 0) {
				m_pDebug->printMsg("CamBegUpd: Device reset call!");
				pD3DDevice->GetRenderTarget(0, &m_renderSurface);
				m_renderSurface->Release();
				pD3DDevice->GetDepthStencilSurface(&m_depthStencilSurface);
				m_depthStencilSurface->Release();
				if (m_videoMemoryRasterListRestore())
				{
					m_dynamicVertexBufferRestore();
					m_pDebug->printMsg("CamBegUpd: Device restore buffers call!");
					//rwD3D9RenderStateReset();
					if (m_maxNumLights <= 0)
					{
						m_im2DRenderOpen();
						Im3DRenderOpen();
						//if (D3D9RestoreDeviceCallback)
						//	D3D9RestoreDeviceCallback();
						m_displayMode.Width = rect.right;
						m_displayMode.Height = rect.bottom;
					}
				}
				else {
					//if (D3D9RestoreDeviceCallback)
					//	D3D9RestoreDeviceCallback();
					ReleaseVideoMemory();
				}
			}
		}
		if (camera->frameBuffer->parent->cType != rwRASTERTYPECAMERATEXTURE)
		{
			m_pDebug->printMsg("CamBegUpd: FB is not camera texture!");
			m_setRenderTarget(0, m_renderSurface);
			m_setDepthStencilSurface(m_depthStencilSurface);

			D3DVIEWPORT9 vp{ camera->frameBuffer->nOffsetX,camera->frameBuffer->nOffsetY,camera->frameBuffer->width,camera->frameBuffer->height,0,1.0 };
			pD3DDevice->SetViewport(&vp);
			HRESULT hr = pD3DDevice->TestCooperativeLevel();
			if (hr == D3DERR_DEVICENOTRESET)
			{
				m_pDebug->printMsg("CamBegUpd: Device reset call!");
				if (!m_resetDevice()) {
					m_insideScene = false;
					return false;
				}
			}
			if (hr >= 0)
			{
				if (!m_insideScene && pD3DDevice->BeginScene() >= 0)
					m_insideScene = true;
				return m_insideScene;
			}
			m_insideScene = 0;
			return m_insideScene;
		}
	}
	m_pDebug->printMsg("CameraBeginUpdate unhandled code!");
	return false;
	//return rwD3D9CameraBeginUpdate(0, camera, 0);
}

bool CRwD3DEngine::CameraEndUpdate(RwCamera *camera)
{
	m_pDebug->printMsg("Camera end update call");
	dgGGlobals = nullptr;
	return true;
}

bool CRwD3DEngine::RasterCreate(RwRaster *raster, UINT flags)
{
	m_pDebug->printMsg("Raster create call");

	RwD3D9Raster* d3dRaster = GetD3D9Raster(raster);
	raster->cpPixels = 0;
	raster->palette = 0;
	raster->cType = flags & rwRASTERTYPEMASK;
	raster->cFlags = flags & 0xF8;
	d3dRaster->texture = 0;
	d3dRaster->palette = 0;
	d3dRaster->alpha = 0;
	d3dRaster->textureFlags = 0;
	d3dRaster->cubeTextureFlags = 0;
	d3dRaster->lockFlags = -1;
	d3dRaster->lockedSurface = 0;
	d3dRaster->format = D3DFMT_UNKNOWN;
	m_pDebug->printMsg("Raster format:" + to_string(flags & 0x7));
	if (!m_checkTextureFormat(raster, flags))
		return false;
	else {
		if (raster->width == 0 && raster->height == 0)
		{
			m_pDebug->printMsg("Width and height is 0");
			raster->cFlags = rwRASTERDONTALLOCATE;
			raster->stride = 0;
			if (raster->depth == 0)
				raster->depth = 16;
			if (raster->cType == rwRASTERTYPECAMERA)
				m_videoMemoryRasterListAdd(raster);
			return true;
		}
		else {
			switch (raster->cType)
			{
			case rwRASTERTYPENORMAL: case rwRASTERTYPETEXTURE:
				m_pDebug->printMsg("Texture raster");
				if (raster->cFlags < 0)
					return true;
				if(!m_createTextureRaster(raster, d3dRaster))
					return false;
				else
					return true;
			case rwRASTERTYPECAMERATEXTURE:
				m_pDebug->printMsg("Camera texture raster: unhandled code");
				if (raster->cFlags < 0)
					return true;
				/*if (!rwD3D9CreateCameraTextureRaster(d3dRaster, raster))
					return false;*/
				break;
			case rwRASTERTYPEZBUFFER:
				m_pDebug->printMsg("ZBuffer raster");
				if (raster->cFlags < 0)
					return true;
				if (!m_createZBufferRaster(raster, d3dRaster))
					return false;
				break;
			case rwRASTERTYPECAMERA:
				m_pDebug->printMsg("Camera raster");
				RECT rect;
				GetClientRect(m_windowHandle, &rect);
				if (!(raster->cFlags & 0x80) && (rect.right < raster->width || rect.bottom < raster->height))
					return false;
				if (m_presentParams.BackBufferFormat == D3DFMT_A8R8G8B8
					|| m_presentParams.BackBufferFormat == D3DFMT_X8R8G8B8
					|| m_presentParams.BackBufferFormat == D3DFMT_A2R10G10B10)
					raster->depth = 32;
				else if (m_presentParams.BackBufferFormat == D3DFMT_R5G6B5
					|| m_presentParams.BackBufferFormat == D3DFMT_X1R5G5B5
					|| m_presentParams.BackBufferFormat == D3DFMT_A1R5G5B5)
					raster->depth = 16;
				else
					raster->depth = 0;
				raster->cFlags = rwRASTERDONTALLOCATE;
				raster->stride = 0;
				raster->cpPixels = 0;
				raster->originalWidth = raster->width;
				raster->originalHeight = raster->height;
				d3dRaster->format = m_presentParams.BackBufferFormat;
				break;
			default:
				return true;
			}
			m_videoMemoryRasterListAdd(raster);
			return true;
		}
	}
	m_pDebug->printMsg("unhandled code");	
	return rwD3D9RasterCreate(nullptr, raster, flags);
}

bool CRwD3DEngine::RasterDestroy(RwRaster *raster)
{
	RwD3D9Raster* d3dRaster = GetD3D9Raster(raster);
	if (raster->parent != raster) {
		if (raster->cType == rwRASTERTYPECAMERA)
			m_videoMemoryRasterListRemove(raster);
		return true;
	}
	else {
		switch (raster->cType)
		{
		case rwRASTERTYPENORMAL: case rwRASTERTYPETEXTURE:
			if (raster->cFlags < 0)
				return true;
			if (!d3dRaster->palette)
			{
				if (d3dRaster->format >= D3DFMT_D16_LOCKABLE && d3dRaster->format <= D3DFMT_D16)
					m_videoMemoryRasterListRemove(raster);
			}
			if (d3dRaster->texture)
				d3dRaster->texture->Release();
			return true;
		case rwRASTERTYPEZBUFFER:
			if (raster->cFlags < 0)
				return true;
			m_videoMemoryRasterListRemove(raster);
			if (d3dRaster->surface != m_depthStencilSurface && d3dRaster->surface)
				d3dRaster->surface->Release();
			return true;
		case rwRASTERTYPECAMERA:
			m_videoMemoryRasterListRemove(raster);
			return true;
		case rwRASTERTYPECAMERATEXTURE:
			if (raster->cFlags < 0)
				return true;
			m_videoMemoryRasterListRemove(raster);
			if (d3dRaster->texture)
				d3dRaster->texture->Release();
			return true;
		default:
			return false;
		}
	}
}

bool CRwD3DEngine::RasterLock(RwRaster *raster, UINT flags, void** data)
{
	m_pDebug->printMsg("Raster lock call");
	RwD3D9Raster* d3dRaster = GetD3D9Raster(raster);
	if (raster->cpPixels)
		return false;
	RwRaster* parentRaster = raster->parent;
	RwRaster* topLevelRaster = parentRaster;
	do
	{
		topLevelRaster = parentRaster;
		parentRaster = parentRaster->parent;
	} while (topLevelRaster != parentRaster);
	int d3dLockFlags = D3DLOCK_NOSYSLOCK;
	if (!(flags & rwRASTERLOCKWRITE))
		d3dLockFlags = D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY;
	RwD3D9Raster* d3dTopLevelRaster = GetD3D9Raster(topLevelRaster);
	switch (raster->cType & rwRASTERTYPEMASK)
	{
	case rwRASTERTYPENORMAL: case rwRASTERTYPETEXTURE:
		
		if (d3dTopLevelRaster->texture->GetSurfaceLevel(flags >> 8, &d3dRaster->lockedSurface) < 0)
			return false;
		if (topLevelRaster == raster)
		{
			if (d3dRaster->lockedSurface->LockRect(&d3dRaster->lockedRect, NULL, d3dLockFlags) < 0)
				return false;
		}
		else
		{
			RECT rect;
			rect.left = raster->nOffsetX;
			rect.right = raster->nOffsetX + raster->width;
			rect.bottom = raster->nOffsetY + raster->height;
			rect.top = raster->nOffsetY;
			if (d3dRaster->lockedSurface->LockRect(&d3dRaster->lockedRect, &rect, d3dLockFlags) < 0)
				return false;
		}
		if (flags & rwRASTERLOCKREAD)
			raster->privateFlags |= rwRASTERPIXELLOCKEDREAD;
		if (flags & rwRASTERLOCKWRITE)
			raster->privateFlags |= rwRASTERPIXELLOCKEDWRITE;
		raster->cpPixels = (RwUInt8 *)d3dRaster->lockedRect.pBits;
		raster->originalWidth = raster->width;
		raster->originalHeight = raster->height;

		raster->width = (raster->width >> (flags >> 8));
		raster->height = (raster->height >> (flags >> 8));
		if (raster->width == 0)
			raster->width = 1;
		if (raster->height == 0)
			raster->height = 1;
		raster->stride = d3dRaster->lockedRect.Pitch;
		d3dRaster->lockFlags = flags >> 8;
		*data = (void*)raster->cpPixels;
		return 1;
	default:
		return false;
	}
}

bool CRwD3DEngine::RasterUnlock(RwRaster *raster)
{
	m_pDebug->printMsg("Raster unlock call");
	RwD3D9Raster* d3dRaster = GetD3D9Raster(raster);
	//RwRaster* parentRaster;
	//RwRaster* topLevelRaster;
	if (raster->cpPixels==nullptr)
		return false;
	switch (raster->cType & rwRASTERTYPEMASK)
	{
	case rwRASTERTYPENORMAL: case rwRASTERTYPETEXTURE:
		d3dRaster->lockedSurface->UnlockRect();
		d3dRaster->lockedSurface->Release();
		raster->width = raster->originalWidth;
		raster->height = raster->originalHeight;
		raster->stride = 0;
		raster->cpPixels = 0;
		if (!(raster->privateFlags & rwRASTERPIXELLOCKEDWRITE) || !(raster->cFormat & 0x10) || d3dRaster->lockFlags || d3dRaster->textureFlags & 0xF)
		{
			raster->privateFlags &= 0xF9;
			return true;
		}
		raster->privateFlags &= 0xF9;
		d3dRaster->lockFlags = -1;
		/*parentRaster = raster->parent;
		do
		{
			topLevelRaster = parentRaster;
			parentRaster = parentRaster->parent;
		} while (topLevelRaster != parentRaster);*/

		return 1;
	default:
		return false;
	}

}

bool CRwD3DEngine::RasterShowRaster(RwRaster *raster, UINT flags)
{
	m_pDebug->printMsg("Raster show raster call");
	if (m_insideScene)
	{
		pD3DDevice->EndScene();
		m_insideScene = false;
	}
	// ****** TODO: Add V-Sync handling code
	/*if (flags & rwRASTERFLIPWAITVSYNC)
	{
		if (m_presentParams.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE) {
			m_presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			m_resetDevice();
		}
	}
	else
	{
		if (m_presentParams.PresentationInterval == D3DPRESENT_INTERVAL_ONE) {
			m_presentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			m_resetDevice();
		}
	}*/
	HRESULT hr;
	if (GetD3D9Raster(raster)->swapChain)
		hr = GetD3D9Raster(raster)->swapChain->Present(NULL, NULL, NULL, NULL, NULL);
	else
		hr = pD3DDevice->Present(NULL, NULL, NULL, NULL);
	if (hr == D3DERR_DEVICELOST)
	{
		hr = pD3DDevice->TestCooperativeLevel();
		if (hr == D3DERR_DEVICENOTRESET)
			m_resetDevice();
	}
	//rwD3D9DynamicVertexBufferManagerForceDiscard();
	return hr >= 0;
}

bool CRwD3DEngine::NativeTextureRead(RwStream *stream, RwTexture** tex)
{
	TextureFormat textureInfo; RasterFormat rasterInfo;
	unsigned int lengthOut, versionOut, numLevels; unsigned char savedFormat;
	RwRaster *raster; RwTexture *texture;
	RwD3D9Raster* d3dRaster;

	if (!RwStreamFindChunk(stream, rwID_STRUCT, &lengthOut, &versionOut) || versionOut < 0x34000 || versionOut > rwLIBRARYVERSION36003 ||
		RwStreamRead(stream, &textureInfo, sizeof(TextureFormat)) != sizeof(TextureFormat) || textureInfo.platformId != rwID_PCD3D9 ||
		RwStreamRead(stream, &rasterInfo, sizeof(RasterFormat)) != sizeof(RasterFormat))
		return false;
	if (rasterInfo.compressed)
	{
		if (m_checkValidTextureFormat(rasterInfo.d3dFormat))
		{
			raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType | rwRASTERDONTALLOCATE);
			if (!raster)
				return false;
		}
		else 
			return false;
		d3dRaster = GetD3D9Raster(raster);
		numLevels = (raster->cFormat & FORMATMIPMAP) ? rasterInfo.numLevels : 1;
		if (!rasterInfo.cubeTexture)
		{
			//if ((rasterInfo.rasterFormat & rwRASTERFORMATMIPMAP) && (rasterInfo.rasterFormat & rwRASTERFORMATAUTOMIPMAP) &&
			//	rwD3D9CheckAutoMipmapGenCubeTextureFormat(rasterInfo.d3dFormat))
				d3dRaster->textureFlags |= 1;
			if (pD3DDevice->CreateTexture(raster->width, raster->height, numLevels, D3DUSAGE_AUTOGENMIPMAP, rasterInfo.d3dFormat,
				D3DPOOL_MANAGED, &d3dRaster->texture, NULL) != D3D_OK)
			{
				RwRasterDestroy(raster);
				return false;
			}
		}
		d3dRaster->textureFlags |= 0x10;
		d3dRaster->format = rasterInfo.d3dFormat;
	}
	else
	{
		if (!rasterInfo.cubeTexture)
		{
			/*if ((rasterInfo.rasterFormat & rwRASTERFORMATMIPMAP) && (rasterInfo.rasterFormat & rwRASTERFORMATAUTOMIPMAP))
			{
				raster = RwD3D9RasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.d3dFormat, rasterInfo.rasterType |
					(rasterInfo.rasterFormat & (rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP)));
				if (!raster)
					return false;
			}*/
			//else
			//{
				raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType);
				if (!raster)
					return false;
				d3dRaster = GetD3D9Raster(raster);
				//raster->d3dRaster.hasCompression = 0; // Зачем??
			//}
		}
	}
	raster->cFlags ^= rwRASTERDONTALLOCATE;
	d3dRaster->alpha = rasterInfo.alpha;

	/*if (rasterInfo.autoMipMaps && !raster->d3dRaster.autoMipMapping || rasterInfo.rasterFormat != ((raster->cFormat) << 8) ||
		rasterInfo.d3dFormat != raster->d3dRaster.format || rasterInfo.width != raster->width || rasterInfo.height != raster->height)
	{
		RwRasterDestroy(raster);
		return false;
	}*/

	/*// Читаем палитру, если надо
	if (rasterInfo.rasterFormat & rwRASTERFORMATPAL4)
	{
		if (RwStreamRead(stream, RwRasterLockPalette(raster, rwRASTERLOCKWRITE), 128) != 128)
		{
			RwRasterUnlockPalette(raster);
			RwRasterDestroy(raster);
			return false;
		}
	}
	else if (rasterInfo.rasterFormat & rwRASTERFORMATPAL8)
	{
		if (RwStreamRead(stream, RwRasterLockPalette(raster, rwRASTERLOCKWRITE), 1024) != 1024)
		{
			RwRasterUnlockPalette(raster);
			RwRasterDestroy(raster);
			return false;
		}
	}
	RwRasterUnlockPalette(raster);*/

	savedFormat = raster->cFormat;
	raster->cFormat ^= FORMATAUTOMIPMAP;
	for (int i = 0; i < 1; i++)
	{
		//d3dRaster->cubeTextureFlags = 0;
		for (int j = 0; j < 1; j++)
		{
			if (RwStreamRead(stream, &lengthOut, 4) == 4)
			{
				if (RwStreamRead(stream, RwRasterLock(raster, j, rwRASTERLOCKWRITE), lengthOut) == lengthOut)
				{
					RwRasterUnlock(raster);
					continue;
				}
			}
			RwRasterUnlock(raster);
			RwRasterDestroy(raster);
			return false;
		}
	}
	raster->cFormat = savedFormat;

	texture = RwTextureCreate(raster);
	if (!texture)
	{
		RwRasterDestroy(raster);
		return false;
	}
	RwTextureSetFilterModeMacro(texture, textureInfo.filterMode);
	RwTextureSetAddressingUMacro(texture, textureInfo.uAddressing);
	RwTextureSetAddressingVMacro(texture, textureInfo.vAddressing);
	RwTextureSetName(texture, textureInfo.name);
	RwTextureSetMaskName(texture, textureInfo.maskName);
	*tex = texture;
	return true;
}

bool CRwD3DEngine::CreateVertexDeclaration(D3DVERTEXELEMENT9* elements, IDirect3DVertexDeclaration9** decl)
{
	int elementCount = 0;
	while (elements[elementCount].Type != D3DDECLTYPE_UNUSED)
		elementCount++;
	if (m_numVertexDeclarations > 0) {
		for (int i = 0;i < m_numVertexDeclarations;i++) {
			if (m_vertexDeclarations[i].elements_count == elementCount &&	m_vertexDeclarations[i].decl==*decl) {
				if(m_vertexDeclarations[i].decl)
					m_vertexDeclarations[i].decl->AddRef();
				return true;
			}
		}
	}
	if (m_numVertexDeclarations >= m_maxVertexDeclarations)
	{
		m_maxVertexDeclarations += 16;
		if (m_vertexDeclarations)
			m_vertexDeclarations = (RwD3D9VertexDecl*)realloc(m_vertexDeclarations, 12 * m_maxVertexDeclarations);
		else
			m_vertexDeclarations = (RwD3D9VertexDecl*)malloc(12 * m_maxVertexDeclarations);
	}
	if (FAILED(pD3DDevice->CreateVertexDeclaration(elements, decl)))
	{
		*decl = 0;
		return false;
	}
	else
	{
		m_vertexDeclarations[m_numVertexDeclarations].decl = &**decl;
		m_vertexDeclarations[m_numVertexDeclarations].elements_count = elementCount;
		m_vertexDeclarations[m_numVertexDeclarations].elements = (D3DVERTEXELEMENT9*)malloc(elementCount*sizeof(D3DVERTEXELEMENT9));
		memcpy(m_vertexDeclarations[m_numVertexDeclarations].elements, elements, elementCount * sizeof(D3DVERTEXELEMENT9));
		m_numVertexDeclarations++;
		return true;
	}
}

void CRwD3DEngine::DeleteVertexDeclaration(IDirect3DVertexDeclaration9* decl)
{
	if (m_numVertexDeclarations>0)
	{
		for (int i = 0;i < m_numVertexDeclarations;i++) {
			if (m_vertexDeclarations[i].decl == decl)
			{
				m_vertexDeclarations[i].decl->Release();
				m_vertexDeclarations[i].decl = nullptr;
			}
		}
	}
}

bool CRwD3DEngine::CreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader)
{
	return SUCCEEDED(pD3DDevice->CreatePixelShader(pFunction, ppShader));
}

bool CRwD3DEngine::DynamicVertexBufferCreate(UINT size, IDirect3DVertexBuffer9** vertexbuffer)
{
	
	if (m_dynamicVertexBufferList==nullptr)
		m_dynamicVertexBufferList = new list<dVB>;
	if (pD3DDevice->CreateVertexBuffer(size,520,0,D3DPOOL_DEFAULT,vertexbuffer,0))
		return false;
	else
	{
		m_dynamicVertexBufferList->push_back({size,*vertexbuffer});
		return true;
	}
}

bool CRwD3DEngine::DynamicVertexBufferLock(RwUInt32 vertexSize, RwUInt32 numVertex, IDirect3DVertexBuffer9 **vertexBufferOut, void **vertexDataOut, RwUInt32 *baseIndexOut)
{
	int lockSize = numVertex * vertexSize,
		lockOffset;
	if (m_deviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		lockOffset = m_dynamicVertexBufferMgrs[m_currentDVBMgr].offset;
		if (lockOffset + lockSize>m_dynamicVertexBufferMgrs[m_currentDVBMgr].size)
		{
			for (m_currentDVBMgr++; m_currentDVBMgr < 4; m_currentDVBMgr++)
			{
				lockOffset = m_dynamicVertexBufferMgrs[m_currentDVBMgr].offset;
				if (lockOffset + lockSize<=m_dynamicVertexBufferMgrs[m_currentDVBMgr].size)
					break;
			}
			if (m_currentDVBMgr >= 4) {
				lockOffset = 0;
				m_currentDVBMgr = 0;
			}
		}
	}
	if (lockSize > m_dynamicVertexBufferMgrs[m_currentDVBMgr].size)
	{
		/*RwD3D9DynamicVertexBuffer* dvbList = DynamicVertexBufferList;
		if (dvbList)
		{
			while (dvbList->pParent!=nullptr && dvbList->pVB != m_dynamicVertexBufferMgrs[m_currentDVBMgr].vertexbuffer)
			{
				dvbList = dvbList->pParent;
			}
			if (dvbList->pParent != nullptr) {
				dvbList->unk = 0;
				dvbList->ppVB = 0;
			}
		}*/
		m_dynamicVertexBufferMgrs[m_currentDVBMgr].size = lockSize;
		DynamicVertexBufferCreate(lockSize, &m_dynamicVertexBufferMgrs[m_currentDVBMgr].vertexbuffer);
	}
	if (lockOffset)
	{
		if (FAILED(m_dynamicVertexBufferMgrs[m_currentDVBMgr].vertexbuffer->Lock(lockOffset,lockSize,vertexDataOut, D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE)))
			return false;
		m_dynamicVertexBufferMgrs[m_currentDVBMgr].offset = lockOffset + lockSize;
		*baseIndexOut = lockOffset / vertexSize;
	}
	else
	{
		if (FAILED(m_dynamicVertexBufferMgrs[m_currentDVBMgr].vertexbuffer->Lock(lockOffset, lockSize, vertexDataOut, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD)))
			return false;
		m_dynamicVertexBufferMgrs[m_currentDVBMgr].offset = lockSize;
		*baseIndexOut = 0;
	}
	*vertexBufferOut = m_dynamicVertexBufferMgrs[m_currentDVBMgr].vertexbuffer;
	return true;
}

void CRwD3DEngine::SetTransform(D3DTRANSFORMSTATETYPE state, RwMatrix *matrix)
{
	pD3DDevice->SetTransform(state, (D3DMATRIX*)matrix);
}

void CRwD3DEngine::ReleaseVideoMemory()
{
	//m_clearCacheShaders();
	m_clearCacheMatrix();

	for (auto i = 0;i < 8;i++)
		pD3DDevice->SetTexture(i, NULL);
	pD3DDevice->SetIndices(NULL);
	for (auto i = 0;i < 4;i++)
		pD3DDevice->SetStreamSource(i, NULL, 0, 0);
	pD3DDevice->SetPixelShader(NULL);
	pD3DDevice->SetVertexDeclaration(NULL);
	pD3DDevice->SetVertexShader(NULL);
	m_setRenderTarget(0, m_renderSurface);
	for (auto i = 1; i < 4; i++)
		m_setRenderTarget(i, NULL);
	m_setDepthStencilSurface(m_depthStencilSurface);

	m_im2DRenderClose();
	rwD3D9Im3DRenderClose();
	rwD3D9DynamicVertexBufferRelease();
	rxD3D9VideoMemoryRasterListRelease();

	RwFreeListPurgeAllFreeLists();
}

int CRwD3DEngine::GetMaxMultiSamplingLevels()
{
	return m_maxMultisamplingLevels;
}

void CRwD3DEngine::SetMultiSamplingLevels(int value)
{
	value = max(value, m_maxMultisamplingLevels);
	m_selectedMultisamplingLevels = value;
	if (value >= 2)
	{
		for (m_selectedMultisamplingLevels; m_selectedMultisamplingLevels >= 2; m_selectedMultisamplingLevels--)
			if (m_d3d9->CheckDeviceMultiSampleType(m_adapterIndex, m_deviceType, m_displayMode.Format, m_windowed, (D3DMULTISAMPLE_TYPE)m_selectedMultisamplingLevels, 0) >= 0)
				break;
		
		m_selectedMultisamplingLevelsNonMask = value * m_maxMultisamplingLevelsNonMask / m_maxMultisamplingLevels;
	}
	else
	{
		m_selectedMultisamplingLevels = 1;
		m_selectedMultisamplingLevelsNonMask = 1;
	}
}

bool CRwD3DEngine::RenderStateSet(RwRenderState nState, void * pParam)
{
	if (nState > rwRENDERSTATEALPHATESTFUNCTIONREF)
		return false;
	switch (nState)
	{
	case rwRENDERSTATETEXTURERASTER:
		if (pParam)
			pD3DDevice->SetTexture(0, GetD3D9Raster(pParam)->texture);
		//else
		//	pD3DDevice->SetTexture(0, NULL);
		break;
	case rwRENDERSTATETEXTUREADDRESS:
		pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, (int)pParam);
		pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, (int)pParam);
		break;
	case rwRENDERSTATEZTESTENABLE:
		pD3DDevice->SetRenderState(D3DRS_ZENABLE,(int)pParam);
		break;
	case rwRENDERSTATEZWRITEENABLE:
		pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, (int)pParam);
		break;
	case rwRENDERSTATESHADEMODE:
		pD3DDevice->SetRenderState(D3DRS_SHADEMODE, (int)pParam);
		break;
	case rwRENDERSTATEVERTEXALPHAENABLE:
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, (int)pParam);
		break;
	/*case rwRENDERSTATETEXTUREFILTER:
		pD3DDevice->SetRenderState(D3DRS_SHADEMODE, (int)pParam);
		break;*/
	default:
		m_pDebug->printMsg("Unhandled rs set call! :" + to_string(nState));
		break;
	}
	return true;
	//throw std::logic_error("The method or operation is not implemented.");
}

bool CRwD3DEngine::RenderStateGet(RwRenderState, void *)
{
	throw std::logic_error("The method or operation is not implemented.");
}


bool createPixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader) {
	return static_cast<CRwD3DEngine*>(g_pRwCustomEngine)->CreatePixelShader(pFunction, ppShader);
}


void CRwD3DEngine::PatchGame()
{
	
	RedirectCall(0x816007, createPixelShader);
	RedirectCall(0x816016, createPixelShader);
	RedirectCall(0x816025, createPixelShader);
	RedirectCall(0x816034, createPixelShader);
	RedirectCall(0x816043, createPixelShader);
	RedirectCall(0x816052, createPixelShader);
}

bool CRwD3DEngine::Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices)
{
	RwRaster* raster;
	void * vertexData=nullptr;
	int primCount;
	if (dgGGlobals)
		raster = dgGGlobals->frameBuffer;
	switch (primType)
	{
	case rwPRIMTYPELINELIST:
		primCount = numVertices / 2;
		break;
	case rwPRIMTYPEPOLYLINE:
		primCount = numVertices - 1;
		break;
	case rwPRIMTYPETRILIST:
		primCount = numVertices / 3;
		break;
	case rwPRIMTYPETRISTRIP:
	case rwPRIMTYPETRIFAN:
		primCount = numVertices - 2;
		break;
	default:
		primCount = 0;
		break;
	}
	if (m_useOwnVertexBuffer)
	{
		if ((m_currentBaseIndex + numVertices) <= 9362)
			m_currentVertexBuffer->Lock(sizeof(RwIm2DVertex) * m_currentBaseIndex, sizeof(RwIm2DVertex) * numVertices, (void**)&vertexData, D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE);
		else
		{
			m_currentBaseIndex = 0;
			m_currentVertexBuffer->Lock(0, sizeof(RwIm2DVertex) * numVertices, &vertexData, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD);
		}
	}
	else
	{
		DynamicVertexBufferLock(28, numVertices, &m_currentVertexBuffer, &vertexData, &m_currentBaseIndex);
	}
	if (vertexData == nullptr)
		return false;
	else {
		if (raster->nOffsetX == 0 && raster->nOffsetY == 0)
			memcpy(vertexData, vertices, 4 * ((sizeof(RwIm2DVertex) * numVertices) >> 2));
		else
			m_pDebug->printError("Fuck raster has offsets!");
		m_currentVertexBuffer->Unlock();
		m_im2DRenderFlush();
		m_pDebug->printMsg("Base index:"+to_string(m_currentBaseIndex));
		if (FAILED(pD3DDevice->DrawPrimitive(m_primConvTable[primType], m_currentBaseIndex, primCount))) {
			m_pDebug->printError("Fuck failed to draw!");
			return false;
		}
		m_currentBaseIndex += numVertices;
		return true;
	}
	//throw std::logic_error("The method or operation is not implemented.");
}

void CRwD3DEngine::m_clearCacheShaders()
{
	rwD3D9LastFVFUsed = 0xFFFFFFFF;
	rwD3D9LastVertexDeclarationUsed = -1;
	rwD3D9LastVertexShaderUsed = -1;
	rwD3D9LastPixelShaderUsed = -1;
	rwD3D9LastIndexBufferUsed = -1;
	if (LastVertexStreamUsed) {
		for (int i = 0; i < 4; i++)
		{
			LastVertexStreamUsed[i].vertexBuffer = (void*)-1;
			LastVertexStreamUsed[i].offset = 0;
			LastVertexStreamUsed[i].stride = 0;
		}
	}
}

void CRwD3DEngine::m_clearCacheMatrix()
{
	LastWorldMatrixUsedIdentity = false;
	if (MatrixFreeList == nullptr)
	{
		MatrixFreeList = RwFreeListCreate(64, 15, 16, rwMEMHINTDUR_EVENT | rwID_DRIVERMODULE);
		memset(LastMatrixUsed, 0, 64 * 16);
	}
	else
	{
		for (auto i = 0; i < 260; i++)
		{
			if (LastMatrixUsed[i])
			{
				_rwFreeListFree(MatrixFreeList, LastMatrixUsed[i]);
				LastMatrixUsed[i] = nullptr;
			}
		}
		RwFreeListPurge(MatrixFreeList);
	}

	u_C98070 = (RwMatrix*)_rwFreeListAlloc(MatrixFreeList, rwMEMHINTDUR_EVENT | rwID_DRIVERMODULE);
	memcpy(u_C98070, &IdentityMatrix, 0x40);
}

void CRwD3DEngine::m_clearCacheLights()
{
	m_maxNumLights = 0;
	if (LightsCache)
	{
		free(LightsCache);
		LightsCache = 0;
	}
}

bool CRwD3DEngine::m_resetDevice()
{
	ReleaseVideoMemory();
	HRESULT hr = pD3DDevice->Reset(&m_presentParams);
	if (hr < 0) {
		m_pDebug->printError(to_string(hr));
		return false;
	}
	pD3DDevice->GetRenderTarget(0, &m_renderSurface);
	m_renderSurface->Release();
	pD3DDevice->GetDepthStencilSurface(&m_depthStencilSurface);
	m_depthStencilSurface->Release();

	if (m_videoMemoryRasterListRestore() )
	{
		m_dynamicVertexBufferRestore();
		//rwD3D9RenderStateReset();
		if (m_maxNumLights <= 0)
		{
			m_im2DRenderOpen();
			Im3DRenderOpen();
		}
	}
	if (D3D9RestoreDeviceCallback)
		D3D9RestoreDeviceCallback();
	return true;
}

void CRwD3DEngine::m_createDisplayModeList()
{
	if (m_displayModes)
		m_displayModes = (RwDisplayMode*)realloc(m_displayModes, 20 * (m_numAdapterModes + 1));
	else
		m_displayModes = (RwDisplayMode*)malloc(20 * (m_numAdapterModes + 1));

	m_d3d9->GetAdapterDisplayMode(m_adapterIndex, &m_displayModes[0]);
	switch (m_displayModes[0].Format)
	{
	case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8:	case D3DFMT_R5G6B5:	
	case D3DFMT_X1R5G5B5: case D3DFMT_A1R5G5B5:	case D3DFMT_A2R10G10B10:
		m_displayModes[0].flags = (RwVideoModeFlag)0;
		m_numDisplayModes = 1;
		break;
	default:
		m_numDisplayModes = 0;
		break;
	}
	int adapterCount=0;
	for (auto fmt : m_aBackBufferFormat)
	{
		int AdapterCount = m_d3d9->GetAdapterModeCount(m_adapterIndex, fmt);
		if (AdapterCount > 0) {
			for (int i = 0; i < AdapterCount; i++)
			{
				m_d3d9->EnumAdapterModes(m_adapterIndex, fmt, i, &m_displayModes[adapterCount]);
				bool bTest = true;
				for (int j = 1;j < adapterCount-1;j++) {
					if (m_displayModes[j].Width == m_displayModes[adapterCount].Width	&&
						m_displayModes[j].Height == m_displayModes[adapterCount].Height&&
						m_displayModes[j].Format == m_displayModes[adapterCount].Format)
						bTest = false;
				}
				if (bTest)
					adapterCount = m_numDisplayModes++ + 1;
			}
		}
	}
	m_pDebug->printMsg(to_string(adapterCount));
	m_pDebug->printMsg(to_string(m_numDisplayModes));
	if (adapterCount< m_numAdapterModes + 1)
		m_displayModes = (RwDisplayMode*)realloc(m_displayModes, 20 * adapterCount);
}

void CRwD3DEngine::m_calculateMaxMSLevels()
{
	m_maxMultisamplingLevels = 1;
	m_selectedMultisamplingLevels = 1;
	m_maxMultisamplingLevelsNonMask = 1;
	m_selectedMultisamplingLevelsNonMask = 1;
	for (int i = D3DMULTISAMPLE_2_SAMPLES; i <= D3DMULTISAMPLE_16_SAMPLES; i++)
		if (m_d3d9->CheckDeviceMultiSampleType(m_adapterIndex, m_deviceType, m_displayMode.Format, m_windowed, static_cast<D3DMULTISAMPLE_TYPE>(i), 0) >= 0)
			m_maxMultisamplingLevels = i;
	int tmp;
	if (m_d3d9->CheckDeviceMultiSampleType(m_adapterIndex, m_deviceType, m_displayMode.Format, m_windowed, D3DMULTISAMPLE_NONMASKABLE, (DWORD*)&tmp) >= 0)
		m_maxMultisamplingLevels = max((m_selectedMultisamplingLevelsNonMask = tmp + 1), m_maxMultisamplingLevels);
}

void CRwD3DEngine::m_rasterOpen()
{
	VideoMemoryRastersFreeList = RwFreeListCreate(8, 127, 4, rwMEMHINTDUR_EVENT | rwID_DRIVERMODULE);
}

void CRwD3DEngine::m_im2DRenderOpen()
{
	m_useOwnVertexBuffer = false;
	if (!(m_deviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		&& pD3DDevice->GetAvailableTextureMem() > 0x800000
		&& pD3DDevice->CreateVertexBuffer(0x40000, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP, 0, D3DPOOL_DEFAULT, &m_currentVertexBuffer, 0) == D3D_OK)
	{
		m_useOwnVertexBuffer = true;
		m_currentBaseIndex = 0;
	}
	IB2DOffset = 0;
	if (pD3DDevice->CreateIndexBuffer(20000, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_DONOTCLIP, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &IndexBuffer2D, 0) != D3D_OK)
	{
		if (m_vertexDeclIm2D)
		{
			DeleteVertexDeclaration(m_vertexDeclIm2D);
			m_vertexDeclIm2D = 0;
		}
		if (m_useOwnVertexBuffer && m_currentVertexBuffer)
		{
			m_currentVertexBuffer->Release();
			m_currentVertexBuffer = 0;
		}
		if (IndexBuffer2D)
		{
			IndexBuffer2D->Release();
			IndexBuffer2D = 0;
		}
	}
	else
	{
		D3DVERTEXELEMENT9 elements[4]{	{ 0,0,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_POSITIONT,0 },
										{ 0,16,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0 },
										{ 0,20,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0 },
										D3DDECL_END() };
		
		CreateVertexDeclaration(elements, &m_vertexDeclIm2D);
	}
}

void CRwD3DEngine::Im3DRenderOpen()
{
	if (pD3DDevice->CreateIndexBuffer(0x1FFFE, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &IndexBuffer3D, nullptr)!=D3D_OK)
	{

		if (IndexBuffer3D)
		{
			IndexBuffer3D->Release();
			IndexBuffer3D = 0;
		}
		if (VertexDeclIm3DNoTex)
		{
			DeleteVertexDeclaration(VertexDeclIm3DNoTex);
			VertexDeclIm3DNoTex = 0;
		}
		if (VertexDeclIm3D)
		{
			DeleteVertexDeclaration(VertexDeclIm3D);
			VertexDeclIm3D = 0;
		}
		if (VertexDeclIm3DOld)
		{
			DeleteVertexDeclaration(VertexDeclIm3DOld);
			VertexDeclIm3DOld = 0;
		}
	}
	else
	{
		D3DVERTEXELEMENT9 im3dOld_elements[5]{	{ 0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0 }, { 0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0 },
												{ 0,24,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0 }, { 0,28,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0 }, D3DDECL_END() };
		D3DVERTEXELEMENT9 im3d_elements[4]{ { 0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0 },{ 0,12,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0 },
											{ 0,16,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0 },	D3DDECL_END() };
		D3DVERTEXELEMENT9 im3dNoTex_elements[3]{ { 0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0 },{ 0,12,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0 },	D3DDECL_END() };
		CreateVertexDeclaration(im3dOld_elements, &VertexDeclIm3DOld);
		CreateVertexDeclaration(im3d_elements, &VertexDeclIm3D);
		CreateVertexDeclaration(im3dNoTex_elements, &VertexDeclIm3DNoTex);
	}
}

void CRwD3DEngine::m_vertexBufferManagerOpen()
{
	m_vertexBufferManagerClose();
	StrideFreeList = RwFreeListCreate(16, 16, 4, rwMEMHINTDUR_GLOBAL | rwID_DRIVERMODULE);
	FreeVBFreeList = RwFreeListCreate(20, 100, 4, 263185);
	CreatedVBFreeList = RwFreeListCreate(8, 100, 4, 263185);
	//DynamicVertexBufferFreeList = RwFreeListCreate(20, 42, 4, 263185);
	m_dynamicVertexBufferManagerCreate();
}

void CRwD3DEngine::m_rasterClose()
{
	RwVideoMemoryRaster *last;

	if (VideoMemoryRasters)
	{
		do
		{
			last = VideoMemoryRasters->pParent;
			_rwFreeListFree(VideoMemoryRastersFreeList, VideoMemoryRasters);
			VideoMemoryRasters = last;
		} while (last);
	}
	if (VideoMemoryRastersFreeList)
	{
		_RwFreeListDestroy(VideoMemoryRastersFreeList);
		VideoMemoryRastersFreeList = nullptr;
	}
}

void CRwD3DEngine::m_im2DRenderClose()
{
	if (m_vertexDeclIm2D)
	{
		DeleteVertexDeclaration(m_vertexDeclIm2D);
		m_vertexDeclIm2D = 0;
	}
	if (m_useOwnVertexBuffer && m_currentVertexBuffer)
	{
		m_currentVertexBuffer->Release();
		m_currentVertexBuffer = 0;
	}
	if (IndexBuffer2D)
	{
		IndexBuffer2D->Release();
		IndexBuffer2D = 0;
	}
}

void CRwD3DEngine::m_vertexBufferManagerClose()
{
	m_currentDVBMgr = 0;
	//RwD3D9DynamicVertexBuffer* pEntry = DynamicVertexBufferList;
	for (int i=0;i<4;i++)
	{
		m_dynamicVertexBufferMgrs[i].offset = 0;
		m_dynamicVertexBufferMgrs[i].size = 0;
		if (m_dynamicVertexBufferMgrs[i].vertexbuffer)
		{
			if (m_dynamicVertexBufferList)
			{
				
				/*while (pEntry->pVB != m_dynamicVertexBufferMgrs[i].vertexbuffer)
				{
					pEntry = pEntry->pParent;
					if (pEntry) {
						pEntry->unk = 0;
						pEntry->ppVB = nullptr;
						pEntry = DynamicVertexBufferList;
					}
					else {
						pEntry = DynamicVertexBufferList;
						break;
					}
				}*/
			}
			m_dynamicVertexBufferMgrs[i].vertexbuffer = 0;
		}
	}
	if (m_dynamicVertexBufferList)
	{
		for (dVB& VB : *m_dynamicVertexBufferList)
		{
			if (VB.vb)
			{
				VB.vb->Release();
				VB.vb = nullptr;
			}
		}
		delete m_dynamicVertexBufferList;
		m_dynamicVertexBufferList = nullptr;
	}
	/*while (pEntry!=nullptr)
	{

		pEntry = pEntry->pParent;
		if (pEntry->pVB)
		{
			pEntry->pVB->Release();
			pEntry->pVB = 0;
			if (pEntry->ppVB)
				*pEntry->ppVB = 0;
		}
		_rwFreeListFree(DynamicVertexBufferFreeList, pEntry);
		DynamicVertexBufferList = pEntry;
	}*/
	/*if (DynamicVertexBufferFreeList)
	{
		_RwFreeListDestroy(DynamicVertexBufferFreeList);
		DynamicVertexBufferFreeList = nullptr;
	}*/
	RwD3D9Stride*pSL_Entry = StrideList;
	if (StrideList)
	{
		while (pSL_Entry->pParent!=nullptr)
		{
			FreeVB* pFreeVBList = pSL_Entry->pFreeVB;
			while (pFreeVBList->pParent!=nullptr)
			{
				_rwFreeListFree(FreeVBFreeList, pFreeVBList);
				pFreeVBList = pFreeVBList->pParent;
			}
			CreatedVB* pCreatedVBList = pSL_Entry->pCreatedVB;
			while (pCreatedVBList->pParent != nullptr)
			{
				if (pCreatedVBList->pVB)
					pCreatedVBList->pVB->Release();
				_rwFreeListFree(FreeVBFreeList, pCreatedVBList);
				pCreatedVBList = pCreatedVBList->pParent;
			}
			_rwFreeListFree(StrideFreeList, pSL_Entry);
			pSL_Entry = pSL_Entry->pParent;
			StrideList = pSL_Entry->pParent;
		}
	}
	if (CreatedVBFreeList)
	{
		_RwFreeListDestroy(CreatedVBFreeList);
		CreatedVBFreeList = nullptr;
	}
	if (FreeVBFreeList)
	{
		_RwFreeListDestroy(FreeVBFreeList);
		FreeVBFreeList = nullptr;
	}
	if (StrideFreeList)
	{
		_RwFreeListDestroy(StrideFreeList);
		StrideFreeList = nullptr;
	}
}

bool CRwD3DEngine::m_videoMemoryRasterListRestore()
{
	RECT rect;
	if (VideoMemoryRasters)
	{
		RwVideoMemoryRaster* vmrList = VideoMemoryRasters;
		while (vmrList->pParent)
		{
			RwRaster* raster = vmrList->pCurrent;
			RwD3D9Raster* d3draster = GetD3D9Raster(raster);
			if (raster->parent == raster)
			{
				switch (raster->cType)
				{
				case rwRASTERTYPENORMAL:
				case rwRASTERTYPETEXTURE:
					if (!(raster->cFormat & 0x60))
					{
						if (d3draster->format >= D3DFMT_D16_LOCKABLE && d3draster->format <= D3DFMT_D16)
							if (pD3DDevice->CreateTexture(raster->width, raster->height, 1, D3DUSAGE_DEPTHSTENCIL, d3draster->format, D3DPOOL_DEFAULT, &d3draster->texture, 0) < 0)
								return false;
					}
					break;
				case rwRASTERTYPECAMERATEXTURE:
					/*if (d3draster->texture)
						break;*/
					break;
				case rwRASTERTYPEZBUFFER:
					if (d3draster->surface)
					{
						GetClientRect(m_windowHandle, &rect);
						raster->width = rect.right;
						raster->height = rect.bottom;
						d3draster->surface = m_depthStencilSurface;
						d3draster->format = m_presentParams.AutoDepthStencilFormat;
						break;
					}
					if (m_ZBufferDepth == 16)
					{
						d3draster->format = m_checkValidZBufferFormat(D3DFMT_D15S1) != 0 ? D3DFMT_D15S1 : D3DFMT_D16;
					}
					else if(m_ZBufferDepth == 32)
					{
						if (m_checkValidZBufferFormat(D3DFMT_D24S8))
							d3draster->format = D3DFMT_D24S8;
						else if (m_checkValidZBufferFormat(D3DFMT_D24X4S4))
							d3draster->format = D3DFMT_D24X4S4;
						else
							d3draster->format = m_checkValidZBufferFormat(D3DFMT_D32) ? D3DFMT_D32 : D3DFMT_D24X8;
					}
					if (pD3DDevice->CreateDepthStencilSurface(raster->width, raster->height, d3draster->format,
						m_presentParams.MultiSampleType, m_presentParams.MultiSampleQuality, 0, &d3draster->surface, 0) < 0)
						return false;
					break;
				case rwRASTERTYPECAMERA:
					if (d3draster->swapChain)
					{
						D3DPRESENT_PARAMETERS tmp;
						memcpy(&tmp, &m_presentParams, sizeof(tmp));
						tmp.hDeviceWindow = *d3draster->hwnd;
						GetClientRect(*d3draster->hwnd, &rect);
						d3draster->swapChain = 0;
						tmp.BackBufferWidth = rect.right == 0;
						tmp.BackBufferHeight = rect.bottom == 0;
						tmp.EnableAutoDepthStencil = 0;
						if (SUCCEEDED(pD3DDevice->CreateAdditionalSwapChain(&tmp, &d3draster->swapChain)))
						{
							if (d3draster->swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &d3draster->surface) < 0)
								return false;
							d3draster->surface->Release();
							d3draster->format = tmp.BackBufferFormat;
						}
					}
					else
					{
						if (m_displayMode.Width!=0)
						{
							if (raster->width != m_presentParams.BackBufferWidth)
								raster->width *= m_presentParams.BackBufferWidth / m_displayMode.Width;
						}
						if (m_displayMode.Height != 0)
						{
							if (raster->height != m_presentParams.BackBufferHeight)
								raster->height *= m_presentParams.BackBufferHeight / m_displayMode.Height;
						}
						raster->depth = 32;
						raster->cFormat = m_getRasterFormat(m_presentParams.BackBufferFormat);
						d3draster->format = m_presentParams.BackBufferFormat;
						d3draster->alpha = 0;
					}
					break;
				default:
					break;
				}
			}
			vmrList = vmrList->pParent;
		}
	}
	return true;
}

void CRwD3DEngine::m_dynamicVertexBufferRestore()
{
	if (m_deviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		if (m_dynamicVertexBufferList)
		{
			for (dVB& VB : *m_dynamicVertexBufferList)
			{
				if (VB.vb==nullptr)
				{
					if (FAILED(pD3DDevice->CreateVertexBuffer(VB.size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, NULL, D3DPOOL_DEFAULT, &VB.vb, nullptr)))
						return;
				}
			}
			/*RwD3D9DynamicVertexBuffer* dynVBlist = DynamicVertexBufferList;
			while (dynVBlist->pParent)
			{
				if (dynVBlist->pVB==nullptr)
				{
					if (FAILED(pD3DDevice->CreateVertexBuffer(dynVBlist->length, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, NULL, D3DPOOL_DEFAULT, &dynVBlist->pVB, nullptr)))
						return;
					*dynVBlist->ppVB = *&dynVBlist->pVB;
				}
				dynVBlist = dynVBlist->pParent;
			}*/
		}
	}
	m_dynamicVertexBufferManagerCreate();
}

void CRwD3DEngine::m_videoMemoryRasterListAdd(RwRaster* r)
{
	RwVideoMemoryRaster* vmRaster = (RwVideoMemoryRaster*)_rwFreeListAlloc(VideoMemoryRastersFreeList, 0x30411);
	vmRaster->pCurrent = r;
	vmRaster->pParent = VideoMemoryRasters;
	VideoMemoryRasters = vmRaster;
}

void CRwD3DEngine::m_videoMemoryRasterListRemove(RwRaster* r)
{
	RwVideoMemoryRaster* vmRasterList = VideoMemoryRasters;
	RwVideoMemoryRaster* pp=nullptr;
	if (VideoMemoryRasters)
	{
		while (vmRasterList->pCurrent != r)
		{
			pp = vmRasterList;
			vmRasterList = vmRasterList->pParent;
			if (!vmRasterList)
				return;
		}
		if (pp)
			pp->pParent = vmRasterList->pParent;
		else if (VideoMemoryRasters == vmRasterList)
		{
			VideoMemoryRasters = vmRasterList->pParent;
		}
		_rwFreeListFree(VideoMemoryRastersFreeList, vmRasterList);
	}
}

void CRwD3DEngine::m_dynamicVertexBufferManagerCreate()
{
	m_currentDVBMgr = 0;
	if (m_deviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		for (int i = 0; i < 4; i++)
		{
			m_dynamicVertexBufferMgrs[i].offset = 0;
			m_dynamicVertexBufferMgrs[i].size = 0x40000;
			if (!DynamicVertexBufferCreate(0x40000u, &m_dynamicVertexBufferMgrs[i].vertexbuffer))
				return;
		}
	}
	else
	{
		m_dynamicVertexBufferMgrs[0].offset = 0;
		m_dynamicVertexBufferMgrs[0].size = 0x40000;
		if (DynamicVertexBufferCreate(0x40000u, &m_dynamicVertexBufferMgrs[0].vertexbuffer))
		{
			m_dynamicVertexBufferMgrs[1].offset = 0;
			m_dynamicVertexBufferMgrs[1].vertexbuffer = nullptr;
			m_dynamicVertexBufferMgrs[1].size = 0;
			m_dynamicVertexBufferMgrs[2].offset = 0;
			m_dynamicVertexBufferMgrs[2].vertexbuffer = nullptr;
			m_dynamicVertexBufferMgrs[2].size = 0;
			m_dynamicVertexBufferMgrs[3].offset = 0;
			m_dynamicVertexBufferMgrs[3].vertexbuffer = nullptr;
			m_dynamicVertexBufferMgrs[3].size = 0;
		}
	}
}

bool CRwD3DEngine::m_checkTextureFormat(RwRaster* raster, UINT flags)
{
	raster->cType = flags & rwRASTERTYPEMASK;
	int rasterFmt = flags & rwRASTERFORMATMASK;
	raster->cFlags = flags & 0xF8;
	int rasterPixelFmt = flags & rwRASTERFORMATPIXELFORMATMASK;
	switch (raster->cType)
	{
	case rwRASTERTYPENORMAL: case rwRASTERTYPETEXTURE:
		if (rasterPixelFmt)
		{
			if (flags&rwRASTERFORMATPAL4)
				return false;
			if (flags&rwRASTERFORMATPAL8)
			{
				if (!m_checkValidTextureFormat(D3DFMT_P8) || rasterPixelFmt != rwRASTERFORMAT888 && 
					(rasterPixelFmt != rwRASTERFORMAT8888 || !(m_deviceCaps.TextureCaps & D3DPTEXTURECAPS_ALPHA)))
				{
					return false;
				}
				raster->depth = 8;
			}
			else {
				D3DFORMAT d3dRasterFmt = m_rasterConvertTable[(rasterFmt >> 8) & 0xF].fmt;
				if (d3dRasterFmt < 70 || d3dRasterFmt > 80)
				{
					if (!m_checkValidTextureFormat(m_rasterConvertTable[(rasterFmt >> 8) & 0xF].fmt))
						return false;
					raster->depth = m_rasterConvertTable[(rasterFmt >> 8) & 0xF].depth;
				}
				else
				{
					if (!m_checkValidZBufferTextureFormat(m_rasterConvertTable[(rasterFmt >> 8) & 0xF].fmt))
						return false;
					raster->depth = m_rasterConvertTable[(rasterFmt >> 8) & 0xF].depth;
				}
			}
		}
		else if (flags & rwRASTERFORMATPAL8)
		{
			if (!m_checkValidTextureFormat(D3DFMT_P8))
				return false;
			if (!(m_deviceCaps.TextureCaps & D3DPTEXTURECAPS_ALPHA))
			{
				rasterFmt |= rwRASTERFORMAT888;
				raster->depth = 8;
			}
			else
			{
				rasterFmt |= rwRASTERFORMAT8888;
				raster->depth = 8;
			}
		}
		else if (m_depth > 16 && m_checkValidTextureFormat(D3DFMT_A8R8G8B8))
		{
			rasterFmt |= rwRASTERFORMAT8888;
			raster->depth = 32;
		}
		else if (m_checkValidTextureFormat(D3DFMT_A4R4G4B4))
		{
			rasterFmt |= rwRASTERFORMAT4444;
			raster->depth = 16;
		}
		else
		{
			rasterFmt |= rwRASTERFORMAT1555;
			raster->depth = 16;
		}
		if (!(rasterFmt & rwRASTERFORMATPAL8) && (flags & (rwRASTERFORMATAUTOMIPMAP | rwRASTERFORMATMIPMAP)) == (rwRASTERFORMATAUTOMIPMAP | rwRASTERFORMATMIPMAP)
			|| m_checkAutoMipmapGenTextureFormat(m_rasterConvertTable[(rasterFmt >> 8) & 0xF].fmt))
			GetD3D9Raster(raster)->textureFlags = GetD3D9Raster(raster)->textureFlags&0xF1|1;
		raster->cFormat = rasterFmt >> 8;
		return true;
	case rwRASTERTYPECAMERA:
		if (rasterPixelFmt)
		{
			if (m_getRasterFormat(m_presentParams.BackBufferFormat) != rasterFmt)
				return 0;
			raster->depth = m_depth;
			raster->cFormat = rasterFmt >> 8;
		}
		else
		{
			raster->depth = m_depth;
			raster->cFormat = m_getRasterFormat(m_presentParams.BackBufferFormat)>>8;
		}
		return true;
	case rwRASTERTYPEZBUFFER:
		if (rasterPixelFmt)
		{
			if (rasterPixelFmt==rwRASTERFORMAT16 && m_ZBufferDepth ==16)
			{
				raster->depth = 16;
				raster->cFormat = rasterPixelFmt >> 8;
			}
			else if (rasterPixelFmt == rwRASTERFORMAT32 && m_ZBufferDepth == 32)
			{
				raster->depth = 32;
				raster->cFormat = rasterPixelFmt >> 8;
			}
			else
				return false;
		}
		else
		{
			if (m_ZBufferDepth == 16)
			{
				raster->depth = 16;
				raster->cFormat = 7;
			}
			else if (m_ZBufferDepth == 32)
			{
				raster->depth = 32;
				raster->cFormat = 9;
			}
			else
				raster->cFormat = rasterPixelFmt >> 8;
		}
		return true;
	default:
		break;
	}
	return false;
}

bool CRwD3DEngine::m_createZBufferRaster(RwRaster *raster, RwD3D9Raster *d3dRaster)
{
	if (m_ZBufferDepth == 16) {
		d3dRaster->format = m_checkValidZBufferFormat(D3DFMT_D15S1) ? D3DFMT_D15S1 : D3DFMT_D16;
	}
	else if (m_ZBufferDepth == 32) {
		if (m_checkValidZBufferFormat(D3DFMT_D24S8))
			d3dRaster->format = D3DFMT_D24S8;
		else {
			if (m_checkValidZBufferFormat(D3DFMT_D24X4S4)) 
				d3dRaster->format = D3DFMT_D24X4S4;
			else
				d3dRaster->format= m_checkValidZBufferFormat(D3DFMT_D32) ? D3DFMT_D32 : D3DFMT_D24X8;
		}
	}
	d3dRaster->alpha = 0;
	RECT rect;
	GetClientRect(m_windowHandle, &rect);
	if (rect.right == raster->width && rect.bottom == raster->height)
	{
		d3dRaster->surface = m_depthStencilSurface;
		return true;
	}
	pD3DDevice->EvictManagedResources();
	HRESULT hr = pD3DDevice->CreateDepthStencilSurface(raster->width, raster->height, d3dRaster->format,
		m_presentParams.MultiSampleType, m_presentParams.MultiSampleQuality, 0, &d3dRaster->surface, 0);
	if (hr != D3DERR_OUTOFVIDEOMEMORY && hr >= 0)
		return 1;
	return 0;
}

bool CRwD3DEngine::m_createTextureRaster(RwRaster *raster, RwD3D9Raster *d3dRaster)
{
	unsigned short rasterFormat = raster->cFormat<<8;
	int rasterPixelFmt = rasterFormat & rwRASTERFORMATPIXELFORMATMASK;
	if (rasterFormat & (rwRASTERFORMATPAL8 | rwRASTERFORMATPAL4))
	{
		m_pDebug->printMsg("Texture raster has palette");
		if (rasterPixelFmt != rwRASTERFORMAT8888 && rasterPixelFmt != rwRASTERFORMAT888)
			return false;
		d3dRaster->format = D3DFMT_P8;
	}
	else
		d3dRaster->format = m_rasterConvertTable[(rasterFormat >> 8) & 0xF].fmt;
	d3dRaster->alpha = m_rasterConvertTable[(rasterFormat >> 8) & 0xF].alpha;
	//check size
	if (d3dRaster->format < D3DFMT_D16_LOCKABLE || d3dRaster->format > D3DFMT_D16)
	{
		if (pD3DDevice->CreateTexture(raster->width, raster->height, (~rasterFormat >> 15) & 1,
			(d3dRaster->textureFlags & 0xF) != 0 ? D3DUSAGE_AUTOGENMIPMAP : NULL, d3dRaster->format, D3DPOOL_MANAGED, &d3dRaster->texture, nullptr)<0)
			return false;
	}
	return true;
}

bool CRwD3DEngine::m_checkValidTextureFormat(D3DFORMAT fmt)
{
	return m_d3d9->CheckDeviceFormat(m_adapterIndex, m_deviceType, m_displayMode.Format, 0, D3DRTYPE_TEXTURE, fmt) >= 0;
}

bool CRwD3DEngine::m_checkValidZBufferFormat(D3DFORMAT fmt)
{
	return	(m_d3d9->CheckDeviceFormat(m_adapterIndex, m_deviceType, m_displayMode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, fmt) >= 0) &&
			(m_d3d9->CheckDepthStencilMatch(m_adapterIndex, m_deviceType, m_displayMode.Format, m_displayMode.Format, fmt) >= 0);
}

bool CRwD3DEngine::m_checkValidZBufferTextureFormat(D3DFORMAT fmt)
{
	return	(m_d3d9->CheckDeviceFormat(m_adapterIndex, m_deviceType, m_displayMode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, fmt) >= 0) &&
		(m_d3d9->CheckDepthStencilMatch(m_adapterIndex, m_deviceType, m_displayMode.Format, m_displayMode.Format, fmt) >= 0);
}

bool CRwD3DEngine::m_checkAutoMipmapGenTextureFormat(D3DFORMAT fmt)
{
	if (m_deviceCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
		return m_d3d9->CheckDeviceFormat(m_adapterIndex, m_deviceType, m_displayMode.Format, D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, fmt) == 0;
	return false;
}

int CRwD3DEngine::m_getRasterFormat(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_A8R8G8B8:
		return rwRASTERFORMAT8888;
	case D3DFMT_X8R8G8B8:
		return rwRASTERFORMAT888;
	case D3DFMT_R5G6B5:
		return rwRASTERFORMAT565;
	case D3DFMT_X1R5G5B5:
		return rwRASTERFORMAT555;
	case D3DFMT_A1R5G5B5:
		return rwRASTERFORMAT1555;
	case D3DFMT_A4R4G4B4:
		return rwRASTERFORMAT4444;
	case D3DFMT_P8:
		return rwRASTERFORMATPAL8;
	case D3DFMT_L8:
		return rwRASTERFORMATLUM8;
	case D3DFMT_D16_LOCKABLE:	case D3DFMT_D15S1:	case D3DFMT_D16:
		return rwRASTERFORMAT16;
	case D3DFMT_D32:	case D3DFMT_D24S8:	case D3DFMT_D24X8:
	case D3DFMT_D24X4S4:	case D3DFMT_D32F_LOCKABLE:	case D3DFMT_D24FS8:
		return rwRASTERFORMAT32;
	default:
		break;
	}
	return rwRASTERFORMATDEFAULT;
}

int CRwD3DEngine::m_getDepthValue(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_A8R8G8B8:	case D3DFMT_X8R8G8B8:	case D3DFMT_A2R10G10B10:
		return 32;
	case D3DFMT_R5G6B5:	case D3DFMT_X1R5G5B5:	case D3DFMT_A1R5G5B5:
		return 16;
	default:
		return 0;
	}
}

bool CRwD3DEngine::Start()
{
	DWORD BehaviorFlags = 0;HRESULT hr;
	D3DDISPLAYMODE CurrentDisplayMode;
	m_d3d9->GetAdapterDisplayMode(m_adapterIndex, &CurrentDisplayMode);
	memset(&m_presentParams, 0, sizeof(m_presentParams));
	m_setPresentParameters(CurrentDisplayMode);
	m_presentParams.hDeviceWindow = m_windowHandle;
	m_presentParams.Flags = 0;

	m_d3d9->GetDeviceCaps(m_adapterIndex, m_deviceType, &RwD3D9DeviceCaps);
	m_d3d9->GetDeviceCaps(m_adapterIndex, m_deviceType, &m_deviceCaps);

	if (m_enableMultithreadSafe)
		BehaviorFlags = D3DCREATE_MULTITHREADED;
	if (m_deviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		if (m_deviceCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
			BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		if (m_enableSoftwareVertexProcessing)
		{
			m_deviceCaps.DevCaps &= ~D3DDEVCAPS_HWTRANSFORMANDLIGHT;
			m_deviceCaps.DeclTypes = 0;
		}
	}

	if ((hr = m_d3d9->CreateDevice(m_adapterIndex, m_deviceType, m_windowHandle, BehaviorFlags, &m_presentParams, &pD3DDevice)) < 0)
	{
		m_pDebug->printError( string{ "Create Device Failed.\n" }+string{ "Error code: " }+to_string(hr) + '\n');
		m_systemStarted = 0;
		return false;
	}
	else
	{
		m_pDebug->printMsg("Device Created.");

		pD3DDevice->GetRenderTarget(0, &m_renderSurface);
		m_renderSurface->Release();
		pD3DDevice->GetDepthStencilSurface(&m_depthStencilSurface);
		m_depthStencilSurface->Release();

		m_currentDepthStencilSurface = nullptr;
		for (auto i = 0;i < 4;i++)
			m_currentRenderSurface[i] = nullptr;

		// Clear cache
		m_clearCacheShaders();
		m_clearCacheMatrix();
		m_clearCacheLights();

		m_rasterOpen();
		m_im2DRenderOpen();
		//_rwD3D9RenderStateOpen();
		m_vertexBufferManagerOpen();
		m_systemStarted = 1;
		return true;
	}
}

bool CRwD3DEngine::Stop()
{
	m_systemStarted = 0;
	if (m_displayModes)
	{
		free(m_displayModes);
		m_displayModes = NULL;
		m_numDisplayModes = 0;
	}
	rwProcessorRelease();
	if (rwD3D9CPUSupportsSSE)
		_mm_setcsr(_mm_getcsr() & 0xFFFF7FFF);
	for (auto i = 0; i < 260; i++)
	{
		if (LastMatrixUsed[i])
		{
			_rwFreeListFree(MatrixFreeList, LastMatrixUsed[i]);
			LastMatrixUsed[i] = NULL;
		}
	}
	_RwFreeListDestroy(MatrixFreeList);
	MatrixFreeList = NULL;
	m_clearCacheLights();

	for (auto i = 0;i < 8;i++)
		pD3DDevice->SetTexture(i, NULL);
	pD3DDevice->SetIndices(NULL);
	for (auto i = 0;i < 4;i++)
		pD3DDevice->SetStreamSource(i, NULL,0,0);
	pD3DDevice->SetPixelShader(NULL);
	pD3DDevice->SetVertexDeclaration(NULL);
	pD3DDevice->SetVertexShader(NULL);

	if (m_numVertexDeclarations > 0)
	{
		for (auto i = 0; i < m_numVertexDeclarations; i++)
		{
			SAFE_RELEASE(m_vertexDeclarations[i].decl);

			if (m_vertexDeclarations[i].elements)
				free(m_vertexDeclarations[i].elements);
		}
	}
	if (m_vertexDeclarations)
	{
		free(m_vertexDeclarations);
		m_vertexDeclarations = nullptr;
	}
	m_numVertexDeclarations = 0;
	m_maxVertexDeclarations = 0;

	m_rasterClose();
	m_vertexBufferManagerClose();
	m_im2DRenderClose();
	pD3DDevice->Release();
	pD3DDevice = NULL;
	return true;
}

bool CRwD3DEngine::GetNumModes(int& displayModeCount)
{
	if (!m_displayModes)
		m_createDisplayModeList();
	displayModeCount = m_numDisplayModes;
	return true;
}

bool CRwD3DEngine::GetModeInfo(RwVideoMode& videoMode, int n)
{
	if (!m_displayModes)
		m_createDisplayModeList();
	if (n < 0 || n >= m_numDisplayModes)
		return false;
	videoMode.width = m_displayModes[n].Width;
	videoMode.height = m_displayModes[n].Height;
	videoMode.depth = m_getDepthValue(m_displayModes[n].Format);
	videoMode.flags = m_displayModes[n].flags;
	videoMode.refRate = m_displayModes[n].RefreshRate;
	switch (m_displayModes[n].Format)
	{
	case D3DFMT_A8R8G8B8:	case D3DFMT_A2R10G10B10:
		videoMode.format = 0x500;
		break;
	case D3DFMT_X8R8G8B8:
		videoMode.format = 0x600;
		break;
	case D3DFMT_R5G6B5:
		videoMode.format = 0x200;
		break;
	case D3DFMT_A1R5G5B5:
		videoMode.format = 0x100;
		break;
	case D3DFMT_X1R5G5B5:
		videoMode.format = 0xA00;
		break;
	default:
		videoMode.format = 0;
		break;
	}
	return true;
}

bool CRwD3DEngine::UseMode(int n)
{
	if (m_systemStarted || n < 0 || n >= m_numDisplayModes)
		return false;
	m_currentModeIndex = n;
	m_displayMode.Width = m_displayModes[n].Width;
	m_displayMode.Height = m_displayModes[n].Height;
	m_displayMode.RefreshRate = m_displayModes[n].RefreshRate;
	m_displayMode.Format = m_displayModes[n].Format;
	m_windowed = !m_displayModes[n].flags;
	m_depth = m_getDepthValue(m_displayModes[n].Format);
	m_calculateMaxMSLevels();
	return true;
}

bool CRwD3DEngine::GetMode(int& mode)
{
	mode = m_currentModeIndex;
	return true;
}





bool mTest(void * a, void * b, int c) {

	return dynamic_cast<CRwD3DEngine*>(g_pRwCustomEngine)->Test(a, b, c);
}

bool CRwD3DEngine::Standards(int* fnPtrArray, int arraySize)
{
// get the pointer to the member function
	bool(*pDefstd)(void *, void *, int ) = &mDefStd;
	bool(*pClear)(void *, void *, int) = &mCamClear;
	bool(*pBeginUpdate)(void *, void *, int) = &mCamBU;
	bool(*pEndUpdate)(void *, void *, int) = &mCameraEndUpdate;
	
	bool(*pRasterCreate)(void *, void *, int) = &mRasterCreate;
	bool(*pRasterDestroy)(void *, void *, int) = &mRasterDestroy;
	
	bool(*pRasterLock)(void *, void *, int) = &mRasterLock;
	bool(*pRasterUnlock)(void *, void *, int) = &mRasterUnlock;
	bool(*pRasterShowRaster)(void *, void *, int) = &mRasterShowRaster;
	bool(*pNativeTextureRead)(void *, void *, int) = &mNativeTextureRead;
	
	bool(*pTest)(void *, void *, int) = &mTest;

	fnPtrArray[0] = (int)(void*&)pDefstd;
	fnPtrArray[1] = (int)(void*&)pBeginUpdate;// 0x7F8F20;//CameraBeginUpdate !!must do!!
	fnPtrArray[2] = 0x7FEE20;//RGBToPixel not used in menu
	fnPtrArray[3] = 0x7FF070;//PixelToRGB not used in menu
	fnPtrArray[4] = (int)(void*&)pRasterCreate;//0x4CCE60;//RasterCreate !!must do!!
	fnPtrArray[5] = (int)(void*&)pRasterDestroy;//0x4CBB00;//RasterDestroy !!must do!!
	fnPtrArray[6] = 0x7FF270;//ImageGetFromRaster not used in menu
	fnPtrArray[7] = 0x8001E0;//RasterSetFromImage not used in menu
	fnPtrArray[8] = 0x4CBD40;//TextureSetRaster not used in menu
	fnPtrArray[9] = 0x7FFF00;//ImageFindRasterFormat not used in menu
	fnPtrArray[10] = (int)(void*&)pEndUpdate;//0x7F98D0;//CameraEndUpdate !!must do!!
	fnPtrArray[11] = 0x4CB524;//SetRasterContext not used in menu
	fnPtrArray[12] = 0x4CBD50;//RasterSubRaster not used in menu
	fnPtrArray[13] = 0x4CB4C0;//RasterClearRect not used in menu
	fnPtrArray[14] = 0x4CB4E0;//RasterClear not used in menu
	fnPtrArray[15] = (int)(void*&)pRasterLock;//0x4C9F90;//RasterLock !!must do!!
	fnPtrArray[16] = (int)(void*&)pRasterUnlock;//0x4CA290;//RasterUnlock !!must do!!
	fnPtrArray[17] = 0x4CAE40;//RasterRender not used in menu
	fnPtrArray[18] = 0x4CAE80;//RasterRenderScaled not used in menu
	fnPtrArray[19] = 0x4CAE60;//RasterRenderFast not used in menu
	fnPtrArray[20] = (int)(void*&)pRasterShowRaster;//0x7F99B0;//RasterShowRaster !!must do!!
	fnPtrArray[21] = (int)(void*&)pClear;
	fnPtrArray[22] = (int)(void*&)pDefstd;
	fnPtrArray[23] = 0x4CA4E0;//RasterLockPalette not used in menu
	fnPtrArray[24] = 0x4CA540;//RasterUnlockPalette not used in menu
	fnPtrArray[25] = 0x4CD360;//NativeTextureGetSize not used in menu
	fnPtrArray[26] = (int)(void*&)pNativeTextureRead;//0x4CD820;//NativeTextureRead !!must do!!
	fnPtrArray[27] = 0x4CD4D0;//NativeTextureWrite not used in menu
	fnPtrArray[28] = (int)(void*&)pTest;//0x4CBCB0;//RasterGetMipLevels not used in menu
	return true;
}

bool CRwD3DEngine::GetNumSubSystems(int& n)
{
	n = m_d3d9->GetAdapterCount();
	m_pDebug->printMsg(to_string(n));
	return true;
}

bool CRwD3DEngine::GetSubSystemInfo(RwSubSystemInfo& info, int n)
{
	D3DADAPTER_IDENTIFIER9 id;
	m_d3d9->GetAdapterIdentifier(n, NULL, &id);
	strncpy_s(info.name, id.Description, 80);
	return true;
}

bool CRwD3DEngine::GetCurrentSubSystem(int& subSystem)
{
	subSystem = m_adapterIndex;
	return true;
}

bool CRwD3DEngine::SetSubSystem(int n)
{
	m_adapterIndex = n;
	m_numAdapterModes = 0;
	for (auto fmt: m_aBackBufferFormat)
		m_numAdapterModes += m_d3d9->GetAdapterModeCount(m_adapterIndex, fmt);

	m_d3d9->GetAdapterDisplayMode(m_adapterIndex,&m_displayMode);
	m_createDisplayModeList();
	m_calculateMaxMSLevels();
	return true;
}

bool CRwD3DEngine::Focus(bool gainFocus)
{
	if (!pD3DDevice)
		return false;
	if (m_presentParams.Windowed)
		return true;
	if(gainFocus)
		ShowWindow(m_windowHandle, 9);
	else
		ShowWindow(m_windowHandle, 0);
	return true;
}

bool CRwD3DEngine::GetTexMemSize(int& memSize)
{
	memSize = pD3DDevice->GetAvailableTextureMem();
	return true;
}

bool CRwD3DEngine::GetMaxTextureSize(int& maxTexSize)
{
	maxTexSize = max(m_deviceCaps.MaxTextureWidth, m_deviceCaps.MaxTextureHeight);
	return true;
}

bool CRwD3DEngine::Test(void *pOut, void *pInOut, RwInt32 nI)
{
	m_pDebug->printMsg("Test call");
	return rwD3D9RasterGetMipLevels(pOut, pInOut, nI);
}

