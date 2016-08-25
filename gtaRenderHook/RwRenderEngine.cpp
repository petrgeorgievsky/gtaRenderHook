#include "stdafx.h"
#include "RwRenderEngine.h"

bool CIRwRenderEngine::EventHandlingSystem(RwRenderSystemState nOption, int * pOut, void * pInOut, int nIn)
{
	std::string s = "Event handled: "; s += std::to_string(static_cast<int>(nOption));
	//m_pDebug->printMsg(s.c_str());
	switch (nOption)
	{
	case RwRenderSystemState::rwDEVICESYSTEMOPEN:
		return Open(*static_cast<HWND*>(pInOut));
	case RwRenderSystemState::rwDEVICESYSTEMCLOSE:
		return Close();
	case RwRenderSystemState::rwDEVICESYSTEMSTART:
		return Start();
	case RwRenderSystemState::rwDEVICESYSTEMSTOP:
		return Stop();
	case RwRenderSystemState::rwDEVICESYSTEMGETNUMMODES:
		return GetNumModes(*pOut);
	case RwRenderSystemState::rwDEVICESYSTEMGETMODEINFO:
		return GetModeInfo(*(RwVideoMode*)(pOut), nIn);
	case RwRenderSystemState::rwDEVICESYSTEMUSEMODE:
		return UseMode(nIn);
	case RwRenderSystemState::rwDEVICESYSTEMFOCUS:
		return Focus(nIn!=0);
	case RwRenderSystemState::rwDEVICESYSTEMGETMODE:
		return GetMode(*pOut);
	case RwRenderSystemState::rwDEVICESYSTEMSTANDARDS:
		return Standards((int*)pOut, nIn);
	case RwRenderSystemState::rwDEVICESYSTEMGETNUMSUBSYSTEMS:
		return GetNumSubSystems(*(int*)pOut);
	case RwRenderSystemState::rwDEVICESYSTEMGETSUBSYSTEMINFO:
		return GetSubSystemInfo(*(RwSubSystemInfo*)pOut, nIn);
	case RwRenderSystemState::rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
		return GetCurrentSubSystem(*pOut);
	case RwRenderSystemState::rwDEVICESYSTEMSETSUBSYSTEM:
		return SetSubSystem(nIn);
	case RwRenderSystemState::rwDEVICESYSTEMGETTEXMEMSIZE:
		return GetTexMemSize(*pOut);
	case RwRenderSystemState::rwDEVICESYSTEMREGISTER:
	case RwRenderSystemState::rwDEVICESYSTEMINITPIPELINE:
	case RwRenderSystemState::rwDEVICESYSTEMFINALIZESTART:
	case RwRenderSystemState::rwDEVICESYSTEMINITIATESTOP:
	case RwRenderSystemState::rwDEVICESYSTEMRXPIPELINEREQUESTPIPE:
	case RwRenderSystemState::rwDEVICESYSTEMGETMETRICBLOCK:
		break;
	case RwRenderSystemState::rwDEVICESYSTEMGETMAXTEXTURESIZE:
		return GetMaxTextureSize(*pOut);
	case RwRenderSystemState::rwDEVICESYSTEMGETID:
		*pOut = 2;
		return true;
	default:
		break;
	}
	return BaseEventHandler(static_cast<int>(nOption),pOut,pInOut,nIn);
}

bool mDefStd(void *, void *, int)
{
	return false;
}

bool mRasterCreate(void * a, void * b, int c)
{
	UNREFERENCED_PARAMETER(a);
	return g_pRwCustomEngine->RasterCreate(static_cast<RwRaster*>(b), (UINT)c);
}

bool mNativeTextureRead(void * a, void * b, int c) {
	UNREFERENCED_PARAMETER(c);
	return g_pRwCustomEngine->NativeTextureRead(static_cast<RwStream*>(a), static_cast<RwTexture**>(b));
}

bool mRasterLock(void * a, void * b, int c) {
	return g_pRwCustomEngine->RasterLock(static_cast<RwRaster*>(b), static_cast<UINT>(c), static_cast<void**>(a));
}
bool mRasterUnlock(void * a, void * b, int c) {
	UNREFERENCED_PARAMETER(a);
	UNREFERENCED_PARAMETER(c);
	return g_pRwCustomEngine->RasterUnlock(static_cast<RwRaster*>(b));
}
bool mCamClear(void * a, void * b, int c) {
	return g_pRwCustomEngine->CameraClear(static_cast<RwCamera*>(a), static_cast<RwRGBA*>(b), c);
}

bool mCamBU(void * a, void * b, int c) {
	UNREFERENCED_PARAMETER(a);
	UNREFERENCED_PARAMETER(c);
	return g_pRwCustomEngine->CameraBeginUpdate(static_cast<RwCamera*>(b));
}

bool mCameraEndUpdate(void * a, void * b, int c) {
	UNREFERENCED_PARAMETER(a);
	UNREFERENCED_PARAMETER(c);
	return g_pRwCustomEngine->CameraEndUpdate(static_cast<RwCamera*>(b));
}
bool mRasterShowRaster(void * a, void * b, int c) {
	UNREFERENCED_PARAMETER(b);
	return g_pRwCustomEngine->RasterShowRaster(static_cast<RwRaster*>(a), (UINT)c);
}
bool mRasterDestroy(void * a , void * b, int c) {
	UNREFERENCED_PARAMETER(a);
	UNREFERENCED_PARAMETER(c);
	return g_pRwCustomEngine->RasterDestroy(static_cast<RwRaster*>(b));
}