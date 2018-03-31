#include "stdafx.h"
#include "RwD3D1XEngine.h"
#include "D3D1XStateManager.h"
#include "D3D1XTexture.h"
#include "D3D1XDefaultPipeline.h"
#include "D3D1XSkinPipeline.h"
#include "D3D1XEnumParser.h"
#include "D3DRenderer.h"
#include "D3D1XIm2DPipeline.h"
#include "D3D1XIm3DPipeline.h"
#include "CustomBuildingPipeline.h"
#include "CustomBuildingDNPipeline.h"
#include "CustomCarFXPipeline.h"
#include "DeferredRenderer.h"
#include "D3D1XVertexDeclarationManager.h"
#include "D3D1XRenderBuffersManager.h"
#include "D3D1XTextureMemoryManager.h"
#include "CustomSeabedPipeline.h"
#include "CustomWaterPipeline.h"
#include "D3D1XVertexBufferManager.h"
#ifdef USE_ANTTWEAKBAR
#include "AntTweakBar.h"
#endif
extern CD3D1XStateManager* g_pStateMgr=nullptr;
extern CD3D1XRenderBuffersManager* g_pRenderBuffersMgr = nullptr;
extern CCustomBuildingPipeline* g_pCustomBuildingPipe = nullptr;
extern CCustomBuildingDNPipeline* g_pCustomBuildingDNPipe = nullptr;
extern CCustomCarFXPipeline* g_pCustomCarFXPipe= nullptr;
extern CDeferredRenderer* g_pDeferredRenderer = nullptr;
extern CCustomSeabedPipeline* g_pCustomSeabedPipe = nullptr;
extern CCustomWaterPipeline* g_pCustomWaterPipe = nullptr;
ShaderRenderStateBuffer g_shaderRenderStateBuffer;

CRwD3D1XEngine::CRwD3D1XEngine(CDebug *d) : CIRwRenderEngine{ d }
{
}

// MAIN FUNCTIONS, handle initialization and deinitialization

bool CRwD3D1XEngine::Open(HWND window)
{
	m_pRenderer = new CD3DRenderer(window);
	return true;
}

bool CRwD3D1XEngine::Close()
{
	delete m_pRenderer;
	return true;
}

// Initializes device, all managers and rendering pipelines.
bool CRwD3D1XEngine::Start()
{
	m_pRenderer->InitDevice();
	g_pStateMgr				= new CD3D1XStateManager();
	g_pRenderBuffersMgr		= new CD3D1XRenderBuffersManager();
	m_pIm2DPipe				= new CD3D1XIm2DPipeline();
	m_pIm3DPipe				= new CD3D1XIm3DPipeline();
	m_pDefaultPipe			= new CD3D1XDefaultPipeline();
	m_pSkinPipe				= new CD3D1XSkinPipeline();
#ifdef USE_ANTTWEAKBAR
	if (!TwInit(TwGraphAPI::TW_DIRECT3D11, m_pRenderer->getDevice()))
		g_pDebug->printMsg("Failed to initialize AntTweakBar, in-game settings will be disabled.", 0);
	else {
		TwWindowSize(m_pRenderer->getCurrentAdapterModeDesc().Width, m_pRenderer->getCurrentAdapterModeDesc().Height);
	}
#endif
	g_pStateMgr->SetScreenSize(static_cast<float>(m_pRenderer->getCurrentAdapterModeDesc().Width), static_cast<float>(m_pRenderer->getCurrentAdapterModeDesc().Height));
	return true;
}

// Deallocates pipelines and managers.
bool CRwD3D1XEngine::Stop()
{
#ifdef USE_ANTTWEAKBAR
	TwTerminate();
#endif
	delete m_pSkinPipe;
	delete m_pDefaultPipe;
	delete m_pIm2DPipe;
	delete m_pIm3DPipe;
	delete g_pRenderBuffersMgr;
	delete g_pStateMgr;
	CD3D1XVertexBufferManager::Shutdown();
	CD3D1XVertexDeclarationManager::Shutdown();
	CD3D1XTextureMemoryManager::Shutdown();
	m_pRenderer->DeInitDevice();
	return true;
}

// VIDEOMODE FUNCTIONS, handle video adapter modes(screen size, refresh rate etc.)

// Returns video adapter mode count
bool CRwD3D1XEngine::GetNumModes(int& n)
{
	n = m_pRenderer->getAdapterModeCount();
	return true;
}

// Returns video mode information
bool CRwD3D1XEngine::GetModeInfo(RwVideoMode& mode, int n)
{
	DXGI_MODE_DESC desc = m_pRenderer->getAdapterModeDesc(n);
	mode = { static_cast<RwInt32>(desc.Width), static_cast<RwInt32>(desc.Height), 32, rwVIDEOMODEEXCLUSIVE, static_cast<RwInt32>(desc.RefreshRate.Numerator), rwRASTERFORMATDEFAULT };
	return true;
}

// Selects current adapter mode.
bool CRwD3D1XEngine::UseMode(int n)
{
	m_pRenderer->setCurrentAdapterMode(n);
	return true;
}

// Returns current adapter mode.
bool CRwD3D1XEngine::GetMode(int& n)
{
	n = m_pRenderer->getCurrentAdapterMode();
	return true;
}


bool CRwD3D1XEngine::Focus(bool)
{
	g_pDebug->printError("The method or operation is not implemented.");
	return true;
}

bool CRwD3D1XEngine::Standards(int* fnPtrArray, int)
{
	bool(*pDefstd)(void *, void *, int) = &mDefStd;
	bool(*pRasterCreate)(void *, void *, int) = &mRasterCreate;
	bool(*pNativeTextureRead)(void *, void *, int) = &mNativeTextureRead;
	bool(*pRasterLock)(void *, void *, int) = &mRasterLock;
	bool(*pRasterUnlock)(void *, void *, int) = &mRasterUnlock;
	bool(*pClear)(void *, void *, int) = &mCamClear;
	bool(*pBeginUpdate)(void *, void *, int) = &mCamBU;
	bool(*pEndUpdate)(void *, void *, int) = &mCameraEndUpdate;
	bool(*pRasterShowRaster)(void *, void *, int) = &mRasterShowRaster;
	bool(*pRasterDestroy)(void *, void *, int) = &mRasterDestroy;

	fnPtrArray[0] = (int)(void*&)pDefstd;
	fnPtrArray[1] = (int)(void*&)pBeginUpdate;//0x7F8F20;//CameraBeginUpdate !!must do!!
	fnPtrArray[2] = 0x7FEE20;//RGBToPixel not used in menu
	fnPtrArray[3] = 0x7FF070;//PixelToRGB not used in menu
	fnPtrArray[4] = (int)(void*&)pRasterCreate;//0x4CCE60;//RasterCreate !!must do!!
	fnPtrArray[5] = (int)(void*&)pRasterDestroy;//0x4CBB00;//RasterDestroy !!must do!!
	fnPtrArray[6] = 0x7FF270;//ImageGetFromRaster not used in menu
	fnPtrArray[7] = (int)(void*&)pDefstd;//0x8001E0;//RasterSetFromImage not used in menu
	fnPtrArray[8] = 0x4CBD40;//TextureSetRaster not used in menu
	fnPtrArray[9] = (int)(void*&)pDefstd;//0x7FFF00;//ImageFindRasterFormat not used in menu
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
	fnPtrArray[21] = (int)(void*&)pClear;//0x4CB4E0;//CameraClear !!must do!!
	fnPtrArray[22] = (int)(void*&)pDefstd;
	fnPtrArray[23] = 0x4CA4E0;//RasterLockPalette not used in menu
	fnPtrArray[24] = 0x4CA540;//RasterUnlockPalette not used in menu
	fnPtrArray[25] = 0x4CD360;//NativeTextureGetSize not used in menu
	fnPtrArray[26] = (int)(void*&)pNativeTextureRead;//0x4CD820;//NativeTextureRead !!must do!!
	fnPtrArray[27] = 0x4CD4D0;//NativeTextureWrite not used in menu
	fnPtrArray[28] = 0x4CBCB0;//RasterGetMipLevels not used in menu
	return true;
}

// SUB-SYSTEM aka VIDEOCARD FUNCTIONS, handle currently used video adapter

bool CRwD3D1XEngine::GetNumSubSystems(int& n)
{
	n = m_pRenderer->getAdapterCount();
	return true;
}

bool CRwD3D1XEngine::GetSubSystemInfo(RwSubSystemInfo& info, int n)
{
	strncpy_s(info.name, m_pRenderer->getAdapterInfo(n), 80);
	return true;
}

bool CRwD3D1XEngine::GetCurrentSubSystem(int& n)
{
	n = m_pRenderer->getCurrentAdapter();
	return true;
}

bool CRwD3D1XEngine::SetSubSystem(int n)
{
	m_pRenderer->setCurrentAdapter(n);
	return true;
}

// UNUSED FUNCTIONS, for multisampling and stuff(TODO: make them great again!)

bool CRwD3D1XEngine::GetTexMemSize(int& memSize)
{
	g_pDebug->printError("The method or operation is not implemented.");
	return true;
}

bool CRwD3D1XEngine::GetMaxTextureSize(int&)
{
	g_pDebug->printError("The method or operation is not implemented.");
	return true;
}

int  CRwD3D1XEngine::GetMaxMultiSamplingLevels()
{
	g_pDebug->printError("The method or operation is not implemented.");
	return 0;
}

void CRwD3D1XEngine::SetMultiSamplingLevels(int)
{
	g_pDebug->printError("The method or operation is not implemented.");
}


// Standard video system function
bool CRwD3D1XEngine::BaseEventHandler(int State, int* a2, void* a3, int a4)
{
	return RwD3DSystem(State, a2, a3, a4);
}

// RENDERSTATE FUNCTIONS

bool CRwD3D1XEngine::RenderStateSet(RwRenderState rs, UINT data)
{
	switch (rs)
	{
	case rwRENDERSTATETEXTURERASTER:												// Sets texture raster renderstate, try to avoid using it 
		if (data)
		{
			RwD3D1XRaster* d3dRaster = GetD3D1XRaster(static_cast<intptr_t>(data));
			if (d3dRaster->resourse) {
				if (!d3dRaster->resourse->isRendering()) {
					g_pStateMgr->SetTextureEnable(true);
				}
			}
			g_pStateMgr->SetRaster((RwRaster*)data);
		}
		else 
		{
			g_pStateMgr->SetTextureEnable(false);
			g_pStateMgr->SetRaster(nullptr);
		}
		break;
	case rwRENDERSTATETEXTUREADDRESS:
		g_pStateMgr->SetTextureAdressUV(static_cast<RwTextureAddressMode>((int)data));
		break;
	case rwRENDERSTATETEXTUREADDRESSU:
		g_pStateMgr->SetTextureAdressU(static_cast<RwTextureAddressMode>((int)data));
		break;
	case rwRENDERSTATETEXTUREADDRESSV:
		g_pStateMgr->SetTextureAdressV(static_cast<RwTextureAddressMode>((int)data));
		break;
	case rwRENDERSTATETEXTUREPERSPECTIVE:
		break;
	case rwRENDERSTATEZTESTENABLE:
		g_pStateMgr->SetDepthEnable(data!=0);
		break;
	case rwRENDERSTATESHADEMODE:
		break;
	case rwRENDERSTATEZWRITEENABLE:
		g_pStateMgr->SetZWriteEnable(data != 0);
		break;
	case rwRENDERSTATETEXTUREFILTER:
		g_pStateMgr->SetTextureFilterMode(static_cast<RwTextureFilterMode>((int)data));
		break;
	case rwRENDERSTATESRCBLEND:
		g_pStateMgr->SetSrcAlphaBlend(static_cast<RwBlendFunction>((int)data));
		break;
	case rwRENDERSTATEDESTBLEND:
		g_pStateMgr->SetDestAlphaBlend(static_cast<RwBlendFunction>((int)data));
		break;
	case rwRENDERSTATEVERTEXALPHAENABLE:
		g_pStateMgr->SetAlphaBlendEnable(data != 0);
		break;
	case rwRENDERSTATEBORDERCOLOR:
		g_pStateMgr->SetTextureBorderColor((*(RwRGBA*)&data));
		break;
	case rwRENDERSTATEFOGENABLE:
		break;
	case rwRENDERSTATEFOGCOLOR:
		break;
	case rwRENDERSTATEFOGTYPE:
		break;
	case rwRENDERSTATEFOGDENSITY:
		break;
	case rwRENDERSTATECULLMODE:
	{
		D3D11_CULL_MODE cm = D3D11_CULL_NONE;
		RwCullMode rwCM = static_cast<RwCullMode>((int)data);
		switch (rwCM)
		{
		case rwCULLMODECULLBACK:
			cm = D3D11_CULL_BACK;
			break;
		case rwCULLMODECULLFRONT:
			cm = D3D11_CULL_FRONT;
			break;
		default:
			break;
		}
		g_pStateMgr->SetCullMode(cm);
	}
		break;
	case rwRENDERSTATESTENCILENABLE:
		g_pStateMgr->SetStencilEnable(data != 0);
		break;
	case rwRENDERSTATESTENCILFAIL:
		g_pStateMgr->SetStencilFail(static_cast<RwStencilOperation>((int)data));
		break;
	case rwRENDERSTATESTENCILZFAIL:
		g_pStateMgr->SetStencilZFail(static_cast<RwStencilOperation>((int)data));
		break;
	case rwRENDERSTATESTENCILPASS:
		g_pStateMgr->SetStencilPass(static_cast<RwStencilOperation>((int)data));
		break;
	case rwRENDERSTATESTENCILFUNCTION:
		g_pStateMgr->SetStencilFunc(static_cast<RwStencilFunction>((int)data));
		break;
	case rwRENDERSTATESTENCILFUNCTIONREF:
		g_pStateMgr->SetStencilFuncRef((int)data);
		break;
	case rwRENDERSTATESTENCILFUNCTIONMASK:
		g_pStateMgr->SetStencilFuncMask((int)data);
		break;
	case rwRENDERSTATESTENCILFUNCTIONWRITEMASK:
		g_pStateMgr->SetStencilFuncWriteMask((int)data);
		break;
	case rwRENDERSTATEALPHATESTFUNCTION:
		g_pStateMgr->SetAlphaTestFunc((RwAlphaTestFunction)data);
		break;
	case rwRENDERSTATEALPHATESTFUNCTIONREF:
		//if (data == 0)
		//	g_pStateMgr->SetAlphaTestEnable(false);
		//else
		g_pStateMgr->SetAlphaTestRef(data / 255.0f);
		break;
	default:
		break;
	}
	return true;
}

bool CRwD3D1XEngine::RenderStateGet(RwRenderState rs, UINT& data)
{
	data = 0;
	switch (rs)
	{
	case rwRENDERSTATETEXTURERASTER:
		data = (UINT)g_pStateMgr->GetRaster();
		break;
	case rwRENDERSTATETEXTUREADDRESS:
		data = (UINT)g_pStateMgr->GetTextureAdressUV();
		break;
	case rwRENDERSTATETEXTUREADDRESSU:
		data = (UINT)g_pStateMgr->GetTextureAdressU();
		break;
	case rwRENDERSTATETEXTUREADDRESSV:
		data = (UINT)g_pStateMgr->GetTextureAdressV();
		break;
	case rwRENDERSTATETEXTUREPERSPECTIVE:
		break;
	case rwRENDERSTATEZTESTENABLE:
		data = g_pStateMgr->GetDepthEnable();
		break;
	case rwRENDERSTATESHADEMODE:
		break;
	case rwRENDERSTATEZWRITEENABLE:
		data = g_pStateMgr->GetZWriteEnable();
		break;
	case rwRENDERSTATETEXTUREFILTER:
		data = (UINT)g_pStateMgr->GetTextureFilterMode();
		break;
	case rwRENDERSTATESRCBLEND:
		data = (UINT)g_pStateMgr->GetSrcAlphaBlend();
		break;
	case rwRENDERSTATEDESTBLEND:
		data = (UINT)g_pStateMgr->GetDestAlphaBlend();
		break;
	case rwRENDERSTATEVERTEXALPHAENABLE:
		data = (UINT)g_pStateMgr->GetAlphaBlendEnable();
		break;
	case rwRENDERSTATEBORDERCOLOR:
		break;
	case rwRENDERSTATEFOGENABLE:
		break;
	case rwRENDERSTATEFOGCOLOR:
		break;
	case rwRENDERSTATEFOGTYPE:
		break;
	case rwRENDERSTATEFOGDENSITY:
		break;
	case rwRENDERSTATECULLMODE:
		data = (UINT)g_pStateMgr->GetCullMode();
		break;
	case rwRENDERSTATESTENCILENABLE:
		break;
	case rwRENDERSTATESTENCILFAIL:
		break;
	case rwRENDERSTATESTENCILZFAIL:
		break;
	case rwRENDERSTATESTENCILPASS:
		break;
	case rwRENDERSTATESTENCILFUNCTION:
		break;
	case rwRENDERSTATESTENCILFUNCTIONREF:
		break;
	case rwRENDERSTATESTENCILFUNCTIONMASK:
		break;
	case rwRENDERSTATESTENCILFUNCTIONWRITEMASK:
		break;
	case rwRENDERSTATEALPHATESTFUNCTION:
		break;
	case rwRENDERSTATEALPHATESTFUNCTIONREF:
		data = g_pStateMgr->GetAlphaTestRef();
		break;
	default:
		break;
	}
	return true;
}

// IM2D PIPELINE FUNCTIONS

bool CRwD3D1XEngine::Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices)
{
	m_pIm2DPipe->Draw(primType, vertices, numVertices);
	return true;
}

bool CRwD3D1XEngine::Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices)
{
	m_pIm2DPipe->DrawIndexed(primType, vertices, numVertices,indices,numIndices);
	return true;
}

// RASTER FUNCTIONS

bool CRwD3D1XEngine::RasterCreate(RwRaster *raster, UINT flags)
{
	// Init renderware raster data
	raster->cpPixels = 0;
	raster->palette = 0;
	raster->cType = flags & rwRASTERTYPEMASK;
	raster->cFlags = flags & 0xF8;

	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(raster);
	// Init directx raster data
	d3dRaster->resourse = nullptr;
	d3dRaster->palette = 0;
	d3dRaster->alpha = 0;
	d3dRaster->textureFlags = 0;
	d3dRaster->cubeTextureFlags = 0;
	d3dRaster->lockFlags = 0;
	d3dRaster->lockedSurface = 0;
	d3dRaster->format = DXGI_FORMAT_UNKNOWN;

	RwUInt32 rasterPixelFmt = flags & rwRASTERFORMATPIXELFORMATMASK;
	raster->cFormat = static_cast<RwUInt8>(rasterPixelFmt >> 8);

	CD3D1XEnumParser::ConvertRasterFormat(raster, flags);

	// If raster size exceeds resonable limit(2^14 for dx11) we shouldn't create it.
	if (raster->width > 8192*2 || raster->height > 8192*2)
		return false;
	// If somehow after format conversion, raster format is still unknown, and it isn't camera raster we shouldn't create it.
	if (d3dRaster->format == DXGI_FORMAT_UNKNOWN && raster->cType != rwRASTERTYPECAMERA)
		return false;

	if (raster->cType == rwRASTERTYPETEXTURE || raster->cType == rwRASTERTYPENORMAL)
		d3dRaster->resourse = new CD3D1XTexture(raster, (flags&rwRASTERFORMATMIPMAP)!=0,(flags&rwRASTERFORMATPAL8)!=0);	
	else if(raster->cType == rwRASTERTYPEZBUFFER || raster->cType == rwRASTERTYPECAMERA)
		d3dRaster->resourse = new CD3D1XTexture(raster, false);
	else if (raster->cType == rwRASTERTYPECAMERATEXTURE)
		d3dRaster->resourse = new CD3D1XTexture(raster, (flags&rwRASTERFORMATMIPMAP) != 0);

	// If we succes in allocating texture we should add it to texture memory manager to avoid some unresonable memory leaks.
	if(d3dRaster->resourse)
		CD3D1XTextureMemoryManager::AddNew(d3dRaster->resourse);
	return true;
}

bool CRwD3D1XEngine::RasterDestroy(RwRaster *raster)
{
	auto d3dRaster = GetD3D1XRaster(raster);
	if (raster&&d3dRaster->resourse) {
		CD3D1XTextureMemoryManager::Remove(d3dRaster->resourse);
		d3dRaster->resourse = nullptr;
	}
	return true;
}

// NATIVE TEXTURE FUNCTIONS 

bool CRwD3D1XEngine::NativeTextureRead(RwStream *stream, RwTexture** tex)
{
	TextureFormat textureInfo; RasterFormat rasterInfo;
	unsigned int lengthOut, versionOut; unsigned char savedFormat;
	RwRaster *raster=nullptr; RwTexture *texture;
	if (!RwStreamFindChunk(stream, rwID_STRUCT, &lengthOut, &versionOut))
		return false;
	if (versionOut >= 0x34000 && versionOut <= rwLIBRARYVERSION36003) // GTA SA
	{
		if (RwStreamRead(stream, &textureInfo, sizeof(TextureFormat)) != sizeof(TextureFormat) || textureInfo.platformId != rwID_PCD3D9 ||
			RwStreamRead(stream, &rasterInfo, sizeof(RasterFormat)) != sizeof(RasterFormat))
			return false;
		if (rasterInfo.compressed)
		{
			//if (rasterInfo.rasterFormat == rwRASTERFORMAT8888) {
				if (rasterInfo.d3dFormat == D3DFORMAT::D3DFMT_DXT5) {
					rasterInfo.rasterFormat = rwRASTERFORMAT8888;
				}
			//}

			raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType | rwRASTERDONTALLOCATE | (rasterInfo.numLevels>1 ? rwRASTERFORMATMIPMAP : 0));
			if (!raster)
				return false;
		}
		else
		{
			if (!rasterInfo.cubeTexture)
			{
				if (rasterInfo.d3dFormat == D3DFORMAT::D3DFMT_DXT5) {
					rasterInfo.rasterFormat = rwRASTERFORMAT8888;
				}
				raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType | (rasterInfo.numLevels>1 ? rwRASTERFORMATMIPMAP : 0));
				if (!raster)
					return false;
			}
			else
				g_pDebug->printError("The method or operation is not implemented.");
		}

		/*
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

		for (int i = 0; i < 1; i++)
		{
			for (RwUInt8 j = 0; j < rasterInfo.numLevels; j++)
			{
				if (RwStreamRead(stream, &lengthOut, sizeof(RwUInt32)) == sizeof(RwUInt32))
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
	}
	else {
		if (RwStreamRead(stream, &textureInfo, sizeof(TextureFormat)) != sizeof(TextureFormat) || textureInfo.platformId != rwID_PCD3D8 ||
			RwStreamRead(stream, &rasterInfo, sizeof(RasterFormat)) != sizeof(RasterFormat))
			return false;
		if (rasterInfo.compression)
		{
			raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType | rwRASTERDONTALLOCATE | (rasterInfo.numLevels>1 ? rwRASTERFORMATMIPMAP : 0));
			if (!raster)
				return false;
		}
		else
		{
			raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType | (rasterInfo.numLevels>1 ? rwRASTERFORMATMIPMAP : 0));
			if (!raster)
				return false;
		}
		//raster->cFlags ^= rwRASTERDONTALLOCATE;

		// 
		if (rasterInfo.rasterFormat & rwRASTERFORMATPAL4)
		{
			g_pDebug->printError("The method or operation is not implemented.");
		}
		else if (rasterInfo.rasterFormat & rwRASTERFORMATPAL8)
		{
			auto d3draster = GetD3D1XRaster(raster)->resourse;
			if (RwStreamRead(stream, &d3draster->GetPalettePtr()[0], 1024) != 1024)
			{
				RwRasterDestroy(raster);
				return false;
			}
		}

		savedFormat = raster->cFormat;

		for (int i = 0; i < 1; i++)
		{
			for (RwUInt8 j = 0; j < rasterInfo.numLevels; j++)
			{
				if (RwStreamRead(stream, &lengthOut, sizeof(RwUInt32)) == sizeof(RwUInt32))
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
	}
	
	return true;
}

bool CRwD3D1XEngine::RasterLock(RwRaster *raster, UINT flags, void** data)
{
	UINT level = flags >> 8;
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(raster);

	// If we already had locked this raster we shouldn't continue.
	if (raster->cpPixels != nullptr)
		return false;

	d3dRaster->lockFlags = static_cast<RwUInt8>(level);

	// Keep original texture size and calculate new texture size by taking square root.
	raster->originalWidth	= raster->width;
	raster->originalHeight	= raster->height;
	raster->width	= max(raster->width >> level, 1);
	raster->height	= max(raster->height >> level, 1);

	// Calculate stride (bytes per line) for BC1 compression we have 8 byte blocks, for BC2+ 16 byte blocks.
	if (d3dRaster->format == DXGI_FORMAT_BC1_UNORM)
		raster->stride = max(1, ((raster->width + 3) / 4)) * 8; 
	else if (d3dRaster->format == DXGI_FORMAT_BC2_UNORM || d3dRaster->format == DXGI_FORMAT_BC3_UNORM)
		raster->stride = max(1, ((raster->width + 3) / 4)) * 16;
	else
		raster->stride = (raster->width * 32 + 7) / 8;

	// Lower bound is 64 to avoid textures smaller than 8x8.
	size_t pixelCount = max(static_cast<size_t>(raster->stride*raster->height), 64);
	raster->cpPixels = (RwUInt8*)malloc(pixelCount);

	*data = (void*)raster->cpPixels; 
	if (flags&rwRASTERLOCKREAD || d3dRaster->resourse->hasPalette()) {
		*data = d3dRaster->resourse->LockToRead();
	}
	return true;
}

bool CRwD3D1XEngine::RasterUnlock(RwRaster *raster)
{
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(raster);
	if (raster->cpPixels == nullptr)
		return false;
	if ((raster->width > 4) && (raster->height > 4))
	{
		if (!d3dRaster->resourse->IsLockedToRead())
			m_pRenderer->getContext()->UpdateSubresource(d3dRaster->resourse->GetTexture(), d3dRaster->lockFlags, nullptr, raster->cpPixels, raster->stride, 0);
		else
			d3dRaster->resourse->UnlockFromRead();
	}
	free(raster->cpPixels);
	raster->cpPixels = nullptr;
	raster->stride = 0;
	raster->width = raster->originalWidth;
	raster->height = raster->originalHeight;
	d3dRaster->lockFlags = 0;
	return true;
}

bool CRwD3D1XEngine::CameraClear(RwCamera *camera, RwRGBA *color, RwInt32 flags)
{
	m_pRenderer->Clear(camera,*color, flags);
	return true;
}

bool CRwD3D1XEngine::CameraBeginUpdate(RwCamera *camera)
{
	RwProcessorForceSinglePrecision();
	dgGGlobals = camera;
	RwMatrix viewTransform = {};
	RwMatrix projTransform = {};
#if (GTA_SA)
	RwMatrix *viewTransformRef = &RwD3D9D3D9ViewTransform;
	RwMatrix *projTransformRef = &RwD3D9D3D9ProjTransform;
#else
	RwMatrix *viewTransformRef = &viewTransform;
	RwMatrix *projTransformRef = &projTransform;
#endif
	RwMatrixInvert(viewTransformRef, RwFrameGetLTM(static_cast<RwFrame*>(camera->object.object.parent)));
	viewTransformRef->right.x = -viewTransformRef->right.x;
	viewTransformRef->up.x = -viewTransformRef->up.x; 
	viewTransformRef->at.x = -viewTransformRef->at.x;
	viewTransformRef->pos.x = -viewTransformRef->pos.x;
	viewTransformRef->flags = 0;
	viewTransformRef->pad1 = 0;
	viewTransformRef->pad2 = 0;
	viewTransformRef->pad3 = 0x3F800000;

	projTransformRef->right.x = camera->recipViewWindow.x;
	projTransformRef->up.y = camera->recipViewWindow.y;
	projTransformRef->at.x = camera->viewOffset.x * camera->recipViewWindow.x;
	projTransformRef->at.y = camera->viewOffset.y * camera->recipViewWindow.y;
	projTransformRef->pos.x = -(camera->viewOffset.x * camera->recipViewWindow.x);
	projTransformRef->pos.y = -(camera->viewOffset.y * camera->recipViewWindow.y);
	if (camera->projectionType == rwPARALLEL)
	{
		projTransformRef->at.z = 1.0f / (camera->farPlane - camera->nearPlane);
		projTransformRef->pad2 = 0;
		projTransformRef->pad3 = 0x3F800000;
	}
	else
	{
		projTransformRef->at.z = camera->farPlane / (camera->farPlane - camera->nearPlane);
		projTransformRef->pad2 = 0x3F800000;
		projTransformRef->pad3 = 0;
	}
	projTransformRef->pos.z = -(projTransformRef->at.z * camera->nearPlane);
#if (GTA_SA)
	RwD3D9ActiveViewProjTransform = nullptr;
#endif
	g_pRenderBuffersMgr->UpdateViewProjMatricles(*viewTransformRef, *projTransformRef);
	
	RECT rc;
	GetClientRect(m_pRenderer->getHWND(), &rc);

	DXGI_MODE_DESC currModeDesc = m_pRenderer->getCurrentAdapterModeDesc();
	if (camera->frameBuffer && RwRasterGetType(camera->frameBuffer) != rwRASTERTYPECAMERATEXTURE) {
		// If we have different client window sizes we must resize it.
		if (static_cast<UINT>(rc.right) != currModeDesc.Width || static_cast<UINT>(rc.bottom) != currModeDesc.Height) 
		{
			auto swapChain = m_pRenderer->getSwapChain();
			g_pStateMgr->SetScreenSize(static_cast<float>(currModeDesc.Width), static_cast<float>(currModeDesc.Height));
			/*m_pRastersToReload.push_back(camera->frameBuffer);*/
			m_pRastersToReload.push_back(camera->zBuffer);
			if (camera->frameBuffer)
				RwRasterDestroy(camera->frameBuffer);
			swapChain->ResizeTarget(&currModeDesc);
			swapChain->ResizeBuffers(1, currModeDesc.Width, currModeDesc.Height, currModeDesc.Format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
			camera->frameBuffer = RwRasterCreate(currModeDesc.Width, currModeDesc.Height, 32, rwRASTERDONTALLOCATE | rwRASTERTYPECAMERA);
			m_bScreenSizeChanged = true;

			SetWindowPos(m_pRenderer->getHWND(), nullptr, 0, 0, currModeDesc.Width, currModeDesc.Height, SWP_NOMOVE | SWP_NOZORDER);
		}
	}
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(camera->frameBuffer);
	if (camera->frameBuffer&&camera->frameBuffer->cType == rwRASTERTYPECAMERATEXTURE)
		d3dRaster->resourse->BeginRendering();
	if (m_bScreenSizeChanged) {
		ReloadTextures();
	}
	m_pRenderer->BeginUpdate(camera);
	return true;
}

bool CRwD3D1XEngine::CameraEndUpdate(RwCamera *camera)
{
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(camera->frameBuffer);
	if(camera->frameBuffer&&camera->frameBuffer->cType==rwRASTERTYPECAMERATEXTURE)
		d3dRaster->resourse->EndRendering();
	m_pRenderer->EndUpdate(camera);
	dgGGlobals = nullptr;
	return true;
}

bool CRwD3D1XEngine::RasterShowRaster(RwRaster *raster, UINT flags)
{
	m_pRenderer->Present(flags&1);
	return true;
}

void destroyNotify(RwResEntry * resEntry) {
	RxD3D9ResEntryHeader resEntryHeader = static_cast<RxInstanceData*>(resEntry)->header;
	if (resEntryHeader.indexBuffer)
	{
		static_cast<ID3D11Buffer*>(resEntryHeader.indexBuffer)->Release();
		resEntryHeader.indexBuffer = nullptr;
	}
	if (resEntryHeader.vertexStream[0].vertexBuffer)
	{
		ID3D11Buffer* buffptr = static_cast<ID3D11Buffer*>(resEntryHeader.vertexStream[0].vertexBuffer);
		CD3D1XVertexBufferManager::Remove(buffptr);
	}
	if (resEntryHeader.vertexDeclaration)
	{
		//((ID3D11Buffer*)(resEntryHeader.vertexDeclaration))->Release();
		resEntryHeader.vertexDeclaration = nullptr;
	}
}

void destroyNotifySkin(RwResEntry * resEntry) {
	RxD3D9ResEntryHeader resEntryHeader = ((RxInstanceData*)resEntry)->header;
	if (resEntryHeader.indexBuffer)
	{
		((ID3D11Buffer*)(resEntryHeader.indexBuffer))->Release();
		resEntryHeader.indexBuffer = nullptr;
	}
	if (resEntryHeader.vertexStream[0].vertexBuffer)
	{
		ID3D11Buffer* buffptr = static_cast<ID3D11Buffer*>(resEntryHeader.vertexStream[0].vertexBuffer);
		CD3D1XVertexBufferManager::Remove(buffptr);
	}
	if (resEntryHeader.vertexDeclaration)
	{
		((ID3D11Buffer*)(resEntryHeader.vertexDeclaration))->Release();
		resEntryHeader.vertexDeclaration = nullptr;
	}
}

int SortTriangles(const void* a, const void* b) {
	return rwD3D9SortTriangles(a, b);
}

bool CRwD3D1XEngine::AtomicAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params)
{
	RpAtomic* atomic = (RpAtomic*)params->dataParam;
	RpGeometry* geom = atomic->geometry;
	RxInstanceData* entryData = nullptr;
	RxD3D9Pipelines* callbacks = (RxD3D9Pipelines*)self->privateData;


	if (geom->numVertices <= 0)
		return true;

	RpD3DMeshHeader* mesh = (RpD3DMeshHeader*)geom->mesh;
	if (!mesh->numMeshes)
		return true;

	RwUInt32 flags = geom->flags;
	if ((flags & rpGEOMETRYNATIVEFLAGSMASK) != rpGEOMETRYNATIVE)
	{
		// If geometry has morph target we use it's resource entry, otherwise atomic one
		if (geom->numMorphTargets == 1)
			entryData = (RxInstanceData*)geom->repEntry;
		else
			entryData = (RxInstanceData*)atomic->repEntry;
		
		if (entryData) {
			if (entryData->header.serialNumber == mesh->serialNum) {
				if (geom->lockedSinceLastInst || geom->numMorphTargets != 1) {
					auto reinstance = callbacks->reinstance;
					if (reinstance && !reinstance(atomic, entryData, callbacks->instance))
					{
						_RwResourcesFreeResEntry(entryData);
						return false;
					}
					atomic->interpolator.flags &= 0xFE;
					geom->lockedSinceLastInst = 0;
				}
				//_RwResourcesUseResEntry(entryData);
				if (entryData->link.next) {
					rwLinkListRemoveLLLink(&entryData->link);
					const RwModuleInfo ResModule = RpResModule;
					const UINT engineOffset = reinterpret_cast<UINT>(*static_cast<RwGlobals**>(RwEngineInstance));
					auto globalPtr = reinterpret_cast<rwResourcesGlobals*>(engineOffset + ResModule.globalsOffset);
					rwLinkListAddLLLink(globalPtr->res.usedEntries, &entryData->link);
				}
				goto RenderCB;
			}
			_RwResourcesFreeResEntry(entryData);
		}
		if (geom->numMorphTargets == 1)
			entryData = m_D3DInstance(atomic, geom, 1, &geom->repEntry, mesh, callbacks->instance,false);
		else
			entryData = m_D3DInstance(atomic, atomic, 1, &atomic->repEntry, mesh, callbacks->instance, false);
		if (entryData != nullptr)
			geom->lockedSinceLastInst = 0;
	}
	else
		entryData = static_cast<RxInstanceData*>(geom->repEntry);

RenderCB:
	if ((flags & rpGEOMETRYNATIVEFLAGSMASK) == rpGEOMETRYNATIVEINSTANCE)
		return true;
	{
		auto ltm = RwFrameGetLTM(static_cast<RwFrame*>(atomic->object.object.parent));
		g_pRenderBuffersMgr->UpdateWorldMatrix(ltm);
		g_pRenderBuffersMgr->SetMatrixBuffer();
		if (callbacks->render)
			callbacks->render(entryData, atomic, 1, geom->flags);
	}

	return true;
}

void CRwD3D1XEngine::DefaultRenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags)
{
	m_pDefaultPipe->Render(repEntry, object, type, flags);
}

RwBool CRwD3D1XEngine::DefaultInstanceCallback(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
{
	return m_pDefaultPipe->Instance(object,resEntryHeader,reinstance);
}

RwBool CRwD3D1XEngine::Im3DSubmitNode()
{
	rwIm3DPool* pool = rwD3D9ImmPool;
	if(pool->stash.flags&rwIM3D_ALLOPAQUE)
		g_pStateMgr->SetAlphaBlendEnable(false);
	else
		g_pStateMgr->SetAlphaBlendEnable(true);
	RwMatrix *ltm = pool->stash.ltm;
	if (ltm) {
		g_pRenderBuffersMgr->UpdateWorldMatrix(ltm);
	}
	else {
		RwMatrix identity{};
		identity.right.x = 1.0f;
		identity.up.y = 1.0f;
		identity.at.z = 1.0f;
		identity.pad3 = 0x3F800000;
		g_pRenderBuffersMgr->UpdateWorldMatrix(&identity);
	}
	g_pRenderBuffersMgr->SetMatrixBuffer();
	return m_pIm3DPipe->SubmitNode();
}

RxInstanceData * CRwD3D1XEngine::m_D3DInstance(void * object, void * instanceObject, RwUInt8 type, RwResEntry ** repEntry, RpD3DMeshHeader * mesh, RxD3D9AllInOneInstanceCallBack instance, int bNativeInstance)
{
	bool	convertToTriList	= false,
			createIndexBuffer	= false;
	RxInstanceData *entry;
	RpAtomic* atomic = (RpAtomic*)object;
	size_t size = sizeof(RxD3D9InstanceData) * mesh->numMeshes + sizeof(RxD3D9ResEntryHeader);

	if (bNativeInstance) {
		entry = (RxInstanceData*)_RwMalloc(sizeof(RxD3D9InstanceData) * mesh->numMeshes + sizeof(RxD3D9ResEntryHeader) + sizeof(RwResEntry), rwMEMHINTDUR_EVENT | rwID_WORLDPIPEMODULE);
		*repEntry = entry;
		entry->link.next		= nullptr;
		entry->link.prev		= nullptr;
		entry->size				= size;
		entry->owner			= nullptr;
		entry->ownerRef			= nullptr;
		entry->destroyNotify	= destroyNotify;
	}
	else
		entry = (RxInstanceData*)_RwResourcesAllocateResEntry(instanceObject, repEntry, size, destroyNotify);
	
	memset(&entry->header, 0, size);
	entry->header.serialNumber	= mesh->serialNum;
	entry->header.numMeshes		= mesh->numMeshes;
	entry->header.indexBuffer	= 0;
	entry->header.totalNumIndex = 0;

	auto primType = mesh->flags & rpMESHHEADERPRIMMASK;

	if (!(mesh->flags & rpMESHHEADERUNINDEXED)) {
		for (auto i = 0; i < mesh->numMeshes; i++)
			entry->header.totalNumIndex += mesh->meshes[i].numIndices;
		if (entry->header.totalNumIndex > 0)
			createIndexBuffer = true;
		if (primType == rpMESHHEADERTRISTRIP&&atomic->geometry->numTriangles * 3 > 0) {
			convertToTriList = true;
			entry->header.totalNumIndex = atomic->geometry->numTriangles * 3;
		}
	}
	if(convertToTriList)
		entry->header.primType = rwPRIMTYPETRILIST;
	else if (primType == rpMESHHEADERTRISTRIP)
		entry->header.primType = rwPRIMTYPETRISTRIP;
	else if (primType == rpMESHHEADERTRIFAN)
		entry->header.primType = rwPRIMTYPETRIFAN;
	else if (primType == rpMESHHEADERLINELIST)
		entry->header.primType = rwPRIMTYPELINELIST;
	else if (primType == rpMESHHEADERPOLYLINE)
		entry->header.primType = rwPRIMTYPEPOLYLINE;
	else if (primType == rpMESHHEADERPOINTLIST)
		entry->header.primType = rwPRIMTYPEPOINTLIST;
	else if (primType == 0)
		entry->header.primType = rwPRIMTYPETRILIST;

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = entry->header.totalNumIndex * sizeof(RxVertexIndex);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	std::vector<RxVertexIndex> indexBufferData(entry->header.totalNumIndex*3);
	//	Load index buffer data.
	if (mesh->numMeshes > 0)
	{
		RxD3D9InstanceData* meshData = nullptr;
		size_t currentIndexOffset = 0;
		for (size_t i = 0; i < mesh->numMeshes; i++)
		{
			meshData = &entry->models[i];
			RwUInt32	minVert = 0,
				numVert = 0;
			if (mesh->flags & rpMESHHEADERPOINTLIST)
			{
				numVert = mesh->meshes[i].numIndices;
				minVert = static_cast<UINT>(currentIndexOffset);
			}
			else if (mesh->meshes[i].numIndices > 0)
			{
				minVert = UINT_MAX;
				UINT maxIndex = 0;
				for (size_t j = 0; j < static_cast<size_t>(mesh->meshes[i].numIndices); j++)
				{
					minVert = min(minVert, mesh->meshes[i].indices[j]);
					maxIndex = max(maxIndex, mesh->meshes[i].indices[j]);
				}
				numVert = maxIndex - minVert + 1;
			}
			meshData->numVertices = numVert;
			meshData->minVert = minVert;
			meshData->vertexShader = nullptr;
			meshData->material = mesh->meshes[i].material;
			meshData->vertexAlpha = false;
			if (currentIndexOffset>indexBufferData.size())
			{
				meshData->numIndex = 0;
				meshData->startIndex = 0;
			}
			else {
				meshData->numIndex = mesh->meshes[i].numIndices;
				meshData->startIndex = static_cast<RwUInt32>(currentIndexOffset);
				if (convertToTriList) {
					meshData->numIndex = rwD3D9ConvertToTriList(&indexBufferData[currentIndexOffset], mesh->meshes[i].indices, mesh->meshes[i].numIndices, meshData->minVert);
				}
				else if (meshData->minVert > 0)
				{
					for (size_t j = 0; j < static_cast<size_t>(mesh->meshes[i].numIndices); j++)
						indexBufferData[currentIndexOffset + j] = mesh->meshes[i].indices[j] - static_cast<RwUInt16>(meshData->minVert);
				}
				else
					memcpy(&indexBufferData[currentIndexOffset], mesh->meshes[i].indices, static_cast<size_t>(mesh->meshes[i].numIndices) * sizeof(RxVertexIndex));
				if (entry->header.primType == rwPRIMTYPETRILIST)
					qsort(&indexBufferData[currentIndexOffset], meshData->numIndex / 3, 6u, SortTriangles);
				currentIndexOffset += static_cast<size_t>(meshData->numIndex);

			}
			switch (entry->header.primType)
			{
			case rwPRIMTYPEPOLYLINE:
				meshData->numPrimitives = mesh->meshes[i].numIndices >> 1;// numPrimitives
				break;
			case rwPRIMTYPETRILIST:
				meshData->numPrimitives = mesh->meshes[i].numIndices - 1;// numPrimitives
				break;
			case rwPRIMTYPETRISTRIP:
				meshData->numPrimitives = mesh->meshes[i].numIndices / 3;// numPrimitives
				break;
			case rwPRIMTYPETRIFAN:
			case rwPRIMTYPEPOINTLIST:
				meshData->numPrimitives = mesh->meshes[i].numIndices - 2;// numPrimitives
				break;
			default:
				meshData->numPrimitives = 0;          // numPrimitives
				break;
			}
			meshData->baseIndex = 0;
		}
	}

	if (createIndexBuffer) {
		D3D11_SUBRESOURCE_DATA	InitData			= {};
								InitData.pSysMem	= indexBufferData.data();
		if (FAILED(m_pRenderer->getDevice()->CreateBuffer(&bd, &InitData, (ID3D11Buffer**)&entry->header.indexBuffer))) {
			g_pDebug->printError("failed to create IB");
			return nullptr;
		}
		g_pDebug->SetD3DName((ID3D11DeviceChild*)entry->header.indexBuffer, "IndexBuffer::" + std::to_string( entry->header.serialNumber ));
	}
	else
		entry->header.indexBuffer = nullptr;
	if (!instance || instance(object, &entry->header, 0))
		return entry;
	else if (bNativeInstance)
		_RwFree(entry);
	else
		_RwResourcesFreeResEntry(entry);
	return nullptr;
}

RxInstanceData * CRwD3D1XEngine::m_D3DSkinInstance(void * object, void * instanceObject, RwResEntry ** repEntry, RpD3DMeshHeader * mesh)
{
	bool	convertToTriList = false,
		createIndexBuffer = false;
	RxInstanceData *entry;
	RpAtomic* atomic = (RpAtomic*)object;
	size_t size = sizeof(RxD3D9InstanceData) * mesh->numMeshes + sizeof(RxD3D9ResEntryHeader);
	entry = (RxInstanceData*)_RwResourcesAllocateResEntry(instanceObject, repEntry, size, destroyNotifySkin);

	memset(&entry->header, 0, size);
	entry->header.serialNumber = mesh->serialNum;
	entry->header.numMeshes = mesh->numMeshes;
	entry->header.indexBuffer = 0;
	entry->header.totalNumIndex = 0;
	for (auto i = 0; i < mesh->numMeshes; i++)
		entry->header.totalNumIndex += mesh->meshes[i].numIndices;
	if (entry->header.totalNumIndex > 0)
		createIndexBuffer = true;
	if ((mesh->flags & rpMESHHEADERPRIMMASK) == rpMESHHEADERTRISTRIP&&atomic->geometry->numTriangles * 3 > 0) {
		convertToTriList = true;
		entry->header.totalNumIndex = atomic->geometry->numTriangles * 3;
	}
	if(convertToTriList)
		entry->header.primType = rwPRIMTYPETRILIST;
	else if ((mesh->flags & rpMESHHEADERPRIMMASK) == rpMESHHEADERTRISTRIP)
		entry->header.primType = rwPRIMTYPETRISTRIP;
	else if ((mesh->flags & rpMESHHEADERPRIMMASK) == rpMESHHEADERTRIFAN)
		entry->header.primType = rwPRIMTYPETRIFAN;
	else if ((mesh->flags & rpMESHHEADERPRIMMASK) == rpMESHHEADERLINELIST)
		entry->header.primType = rwPRIMTYPELINELIST;
	else if ((mesh->flags & rpMESHHEADERPRIMMASK) == rpMESHHEADERPOLYLINE)
		entry->header.primType = rwPRIMTYPEPOLYLINE;
	else if ((mesh->flags & rpMESHHEADERPRIMMASK) == rpMESHHEADERPOINTLIST)
		entry->header.primType = rwPRIMTYPEPOINTLIST;
	else if ((mesh->flags & rpMESHHEADERPRIMMASK) == 0)
		entry->header.primType = rwPRIMTYPETRILIST;

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = entry->header.totalNumIndex * sizeof(RxVertexIndex);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	std::vector<RxVertexIndex> indexBufferData(entry->header.totalNumIndex);
	//	Load index buffer data.
	if (mesh->numMeshes > 0)
	{
		RxD3D9InstanceData* meshData = nullptr;
		size_t currentIndexOffset = 0;
		for (size_t i = 0; i < mesh->numMeshes; i++)
		{
			meshData = &entry->models[i];
			RwUInt32	minVert = 0,
				numVert = 0;
			if (mesh->flags & rpMESHHEADERPOINTLIST)
			{
				numVert = mesh->meshes[i].numIndices;
				minVert = static_cast<UINT>(currentIndexOffset);
			}
			else if (mesh->meshes[i].numIndices > 0)
			{
				minVert = UINT_MAX;
				UINT maxIndex = 0;
				for (size_t j = 0; j < static_cast<size_t>(mesh->meshes[i].numIndices); j++)
				{
					minVert = min(minVert, mesh->meshes[i].indices[j]);
					maxIndex = max(maxIndex, mesh->meshes[i].indices[j]);
				}
				numVert = maxIndex - minVert + 1;
			}
			meshData->numVertices = numVert;
			meshData->minVert = minVert;
			meshData->vertexShader = nullptr;
			meshData->material = mesh->meshes[i].material;
			meshData->vertexAlpha = false;
			if (&indexBufferData[currentIndexOffset] == nullptr)
			{
				meshData->numIndex = 0;
				meshData->startIndex = 0;
			}
			else {
				meshData->numIndex = mesh->meshes[i].numIndices;
				meshData->startIndex = static_cast<RwUInt32>(currentIndexOffset);
				if (convertToTriList) {
					meshData->numIndex = rwD3D9ConvertToTriList(&indexBufferData[currentIndexOffset], mesh->meshes[i].indices, mesh->meshes[i].numIndices, meshData->minVert);
				}
				else if (meshData->minVert > 0)
				{
					for (size_t j = 0; j < static_cast<size_t>(mesh->meshes[i].numIndices); j++)
						indexBufferData[currentIndexOffset + j] = mesh->meshes[i].indices[j] - static_cast<RwUInt16>(meshData->minVert);
				}
				else
					memcpy(&indexBufferData[currentIndexOffset], mesh->meshes[i].indices, static_cast<size_t>(mesh->meshes[i].numIndices) * sizeof(RxVertexIndex));
				if (entry->header.primType == rwPRIMTYPETRILIST)
					qsort(&indexBufferData[currentIndexOffset], meshData->numIndex / 3, 6u, SortTriangles);
				currentIndexOffset += static_cast<size_t>(meshData->numIndex);

			}
			switch (entry->header.primType)
			{
			case rwPRIMTYPEPOLYLINE:
				meshData->numPrimitives = mesh->meshes[i].numIndices >> 1;// numPrimitives
				break;
			case rwPRIMTYPETRILIST:
				meshData->numPrimitives = mesh->meshes[i].numIndices - 1;// numPrimitives
				break;
			case rwPRIMTYPETRISTRIP:
				meshData->numPrimitives = mesh->meshes[i].numIndices / 3;// numPrimitives
				break;
			case rwPRIMTYPETRIFAN:
			case rwPRIMTYPEPOINTLIST:
				meshData->numPrimitives = mesh->meshes[i].numIndices - 2;// numPrimitives
				break;
			default:
				meshData->numPrimitives = 0;          // numPrimitives
				break;
			}
			meshData->baseIndex = 0;
		}
	}

	if (createIndexBuffer) {
		D3D11_SUBRESOURCE_DATA	InitData = {};
		InitData.pSysMem = indexBufferData.data();
		//	Reenable GPU access to the index buffer data.
		if (FAILED(m_pRenderer->getDevice()->CreateBuffer(&bd, &InitData, (ID3D11Buffer**)&entry->header.indexBuffer))) {
			g_pDebug->printError("failed to create IB");
			return nullptr;
		}
	}
	RpHAnimHierarchy* atomicHier	= AtomicGetHAnimHier(atomic);
	RpSkin* geomSkin	= GeometryGetSkin(atomic->geometry);
	if (geomSkin)
	{
		if (geomSkin->meshBoneRLECount)
			geomSkin->boneCount = geomSkin->boneLimit;
		else
			geomSkin->boneCount = geomSkin->numBoneIds;
		if (atomicHier)
			geomSkin->useVS = true;
		else
			geomSkin->useVS = false;
	}
	entry->header.totalNumVertex = atomic->geometry->numVertices;
	if (m_pSkinPipe->Instance(atomic, &entry->header, false))
		return entry;
	return nullptr;
}

void CRwD3D1XEngine::SetTexture(RwTexture * tex, int Stage)
{
	if (tex == nullptr) {
		g_pStateMgr->SetTextureEnable(false);
		return;
	}

	//RenderStateSet(rwRENDERSTATETEXTUREFILTER, RwTextureGetFilterMode(tex));
	//RenderStateSet(rwRENDERSTATETEXTUREADDRESSU, RwTextureGetAddressingU(tex));
	//RenderStateSet(rwRENDERSTATETEXTUREADDRESSV, RwTextureGetAddressingV(tex));
	auto raster = RwTextureGetRaster(tex);
	if (raster) {
		RwD3D1XRaster* d3dRaster = GetD3D1XRaster(raster);
		if (d3dRaster) {
			if (d3dRaster->resourse && !d3dRaster->resourse->isRendering()) {
				/*auto srv = d3dRaster->resourse->GetSRV().p;
				m_pRenderer->getContext()->PSSetShaderResources(Stage, 1, &srv);*/
				g_pStateMgr->SetRaster(raster);
				g_pStateMgr->SetTextureEnable(true);
			}
		}
		else
		{
			g_pStateMgr->SetRaster(nullptr);
			g_pStateMgr->SetTextureEnable(false);
		}
	}
}

bool CRwD3D1XEngine::SkinAllInOneNode(RxPipelineNode * self, const RxPipelineNodeParam * params)
{
	RpAtomic* atomic = (RpAtomic*)params->dataParam;
	RpGeometry* geom = atomic->geometry;
	RxInstanceData* entryData = nullptr;
	RpSkinPipeCB* callbacks = (RpSkinPipeCB*)self->privateData;
	if (geom->numVertices <= 0)
		return true;
	RpD3DMeshHeader* mesh = (RpD3DMeshHeader*)geom->mesh;
	if (!mesh->numMeshes)
		return true;

	if (geom->numMorphTargets == 1)
		entryData = (RxInstanceData*)geom->repEntry;
	else
		entryData = (RxInstanceData*)atomic->repEntry;

	if (entryData == nullptr || entryData->header.serialNumber != mesh->serialNum) {
		if (entryData != nullptr)
			_RwResourcesFreeResEntry(entryData);
		if (geom->numMorphTargets == 1)
			entryData = m_D3DSkinInstance(atomic, geom, &geom->repEntry, mesh);
		else
			entryData = m_D3DSkinInstance(atomic, atomic, &atomic->repEntry, mesh);
		if (entryData == nullptr)
			return false;
		geom->lockedSinceLastInst = 0;
	}
	else 
	{
		if (geom->lockedSinceLastInst)
		{
			m_pSkinPipe->Instance(atomic, &entryData->header, true);
			geom->lockedSinceLastInst = 0;
		}
		atomic->interpolator.flags &= ~1u;
		if (entryData->link.next)
		{
			rwLinkListRemoveLLLink(&entryData->link);
			RwModuleInfo ResModule = RpResModule;
			UINT engineOffset = (UINT)*(RwGlobals**)RwEngineInstance;
			rwResourcesGlobals* globalPtr = (rwResourcesGlobals*)(engineOffset + ResModule.globalsOffset);
			rwLinkListAddLLLink(globalPtr->res.usedEntries, &entryData->link);
		}
	}
	{
		RwMatrix *ltm = RwFrameGetLTM((RwFrame*)atomic->object.object.parent);
		g_pRenderBuffersMgr->UpdateWorldMatrix(ltm);

		g_pRenderBuffersMgr->SetMatrixBuffer();
		if (callbacks->render)
			m_pSkinPipe->Render(entryData, atomic, 1, geom->flags);
	}
	return true;
}

RwTexture * CRwD3D1XEngine::CopyTexture(RwTexture * tex)
{
	RwRaster* raster = tex->raster;
	RwRaster* rasterTex = RwRasterCreate(raster->width, raster->height, raster->depth, (raster->cFormat & 0x6F) << 8 | 4);
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(raster);
	RwD3D1XRaster* d3dRaster2 = GetD3D1XRaster(rasterTex);
	m_pRenderer->getContext()->CopySubresourceRegion(d3dRaster2->resourse->GetTexture(),0, 0, 0, 0, d3dRaster->resourse->GetTexture(), 0, nullptr);
	return RwTextureCreate(rasterTex);
}

void CRwD3D1XEngine::SetRenderTargets(RwRaster ** rasters, RwRaster* zBuffer, RwUInt32 rasterCount)
{
	
	auto context = m_pRenderer->getContext();
	std::vector<ID3D11RenderTargetView*> pRTVs;
	for (RwUInt32 i = 0; i < rasterCount; i++)
	{
		RwD3D1XRaster* pd3dRaster = GetD3D1XRaster(rasters[i]);
		if(rasters[i] !=nullptr)
			pRTVs.push_back(pd3dRaster->resourse->GetRTRV());
		else
			pRTVs.push_back(nullptr);
	}
	D3D11_VIEWPORT vp;
	vp.Width = rasters[0]->width;
	vp.Height = rasters[0]->height;
	vp.MaxDepth = 1;
	vp.MinDepth = 0;
	vp.TopLeftX = 0;vp.TopLeftY = 0;
	g_pStateMgr->SetViewport(vp);
	if (zBuffer) {
		RwD3D1XRaster* pd3dDepthRaster = GetD3D1XRaster(zBuffer);
		g_pStateMgr->SetRenderTargets(pRTVs.size(), pRTVs.data(), pd3dDepthRaster->resourse->GetDSRV());
	}else
		g_pStateMgr->SetRenderTargets(pRTVs.size(), pRTVs.data(), nullptr);
	if (rasterCount >= 1) {
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (auto rtv: pRTVs)
			context->ClearRenderTargetView(rtv, clearColor);
	}
}

void CRwD3D1XEngine::SetRenderTargetsAndUAVs(RwRaster ** rasters, RwRaster ** uavs, RwRaster* zBuffer, RwUInt32 rasterCount, RwUInt32 uavCount, RwUInt32 uavStart)
{
	RwD3D1XRaster* pd3dDepthRaster = GetD3D1XRaster(zBuffer);
	std::vector<ID3D11RenderTargetView*> pRTVs;
	std::vector<ID3D11UnorderedAccessView*> pUAVs;
	for (RwUInt32 i = 0; i < rasterCount; ++i)
	{
		RwD3D1XRaster* pd3dRaster = GetD3D1XRaster(rasters[i]);
		pRTVs.push_back(pd3dRaster->resourse->GetRTRV());
	}
	D3D11_VIEWPORT vp;
	vp.Width	= static_cast<FLOAT>(rasters[0]->width);
	vp.Height	= static_cast<FLOAT>(rasters[0]->height);
	vp.MaxDepth = 1;
	vp.MinDepth = 0;
	vp.TopLeftX = 0;vp.TopLeftY = 0;
	g_pStateMgr->SetViewport(vp);
	if (uavs[0] == nullptr) {
		ID3D11UnorderedAccessView* puav = nullptr;
		m_pRenderer->getContext()->OMSetRenderTargetsAndUnorderedAccessViews(pRTVs.size(), pRTVs.data(), pd3dDepthRaster->resourse->GetDSRV(),
			uavStart, 1, &puav, nullptr);
		return;
	}
	for (RwUInt32 i = 0; i < uavCount; i++)
	{
		RwD3D1XRaster* pd3dRaster = GetD3D1XRaster(uavs[i]);
		pUAVs.push_back(pd3dRaster->resourse->GetUAV());
	}
	
	m_pRenderer->getContext()->OMSetRenderTargetsAndUnorderedAccessViews(pRTVs.size(), pRTVs.data(), pd3dDepthRaster->resourse->GetDSRV(),
		uavStart, pUAVs.size(), pUAVs.data(),nullptr);
	if (rasterCount > 1) {
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (auto rtv : pRTVs)
			m_pRenderer->getContext()->ClearRenderTargetView(rtv, clearColor);
	}
}

void CRwD3D1XEngine::ReloadTextures()
{
	if (m_pRastersToReload.size() > 0) {
		for (auto raster: m_pRastersToReload)
		{
			if (raster) {
				int flags = raster->cType | raster->cFlags | (raster->cFormat << 8);
				GetD3D1XRaster(raster)->resourse->Reload();
			}
		}
		m_pRastersToReload.clear();
	}
}
