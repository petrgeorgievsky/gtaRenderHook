// dllmain.cpp : Defines the entry point for the DLL application.

#include "CDebug.h"
//#include "RwRenderEngine.h"
//#include "RwVulkanEngine.h"
#include "RwD3D1XEngine.h"
CDebug* g_pDebug;
CIRwRenderEngine* g_pRwCustomEngine;

bool D3DSystemHook(int State, int* a2, void* a3, int a4) {
	return g_pRwCustomEngine->EventHandlingSystem((RwRenderSystemState)State, a2, a3, a4);
}
bool SetRenderStateHook(RwRenderState nState, UINT pParam) {
	return g_pRwCustomEngine->RenderStateSet(nState, pParam);
}
UINT rsData;
bool GetRenderStateHook(RwRenderState nState, UINT* pParam) {
	g_pRwCustomEngine->RenderStateGet(nState, rsData);
	*pParam = rsData;
	return true;
}
void SetRR(RwUInt32 refreshRate) {
	UNREFERENCED_PARAMETER(refreshRate);
}
void SetVM(RwUInt32 videomode) {
	g_pRwCustomEngine->UseMode(videomode);
}

bool psNativeTextureSupport() {
	return true;
}
RwBool im2dDrawPrim(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices) {
	return g_pRwCustomEngine->Im2DRenderPrimitive(primType, vertices, numVertices);
}
RwBool im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices,	RwImVertexIndex *indices, RwInt32 numIndices) {
	return g_pRwCustomEngine->Im2DRenderIndexedPrimitive(primType, vertices, numVertices, indices, numIndices);
}
RwBool im2DRenderLine(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2) {
	return true;
}
void im3dOpen() {
	//static_cast<CRwVulkanEngine*>(g_pRwCustomEngine)->Im3DRenderOpen();
}
void CGamma__init(void *p) {
	UNREFERENCED_PARAMETER(p);
}
bool envMapSupport() {
	return true;
}
RwBool im3dSubmit() {
	return g_pRwCustomEngine->Im3DSubmitNode();
}
bool _D3D9AtomicAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params) {
	return g_pRwCustomEngine->AtomicAllInOneNode(self,params);
}
bool _D3D9SkinAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params) {
	return g_pRwCustomEngine->SkinAllInOneNode(self, params);
}
void _rxD3D9DefaultRenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags) {
	g_pRwCustomEngine->DefaultRenderCallback(repEntry, object, type, flags);
}
void _rxD3D9SkinRenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags) {
	//g_pRwCustomEngine->SkinRenderCallback(repEntry, object, type, flags);
}
RwBool _rxD3D9DefaultInstanceCallback(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance) {
	return g_pRwCustomEngine->DefaultInstanceCallback(object, resEntryHeader, reinstance);
}
RwBool  _rwD3D9RWSetRasterStage(RwRaster* raster,int stage) {
	if (!stage) {
		return g_pRwCustomEngine->RenderStateSet(rwRENDERSTATETEXTURERASTER, (UINT)raster);
	}
	return true;
}
RwTexture* CopyTex(RwTexture* tex) {
	return g_pRwCustomEngine->CopyTexture(tex);
}
RxPipeline * RxPipelineExecute(RxPipeline *pipeline, void *data, RwBool heapReset) {
	return pipeline;
} 
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	UNREFERENCED_PARAMETER(lpReserved);
	UNREFERENCED_PARAMETER(hModule);
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_pDebug = new CDebug("debug.log");
		g_pRwCustomEngine = new CRwD3D1XEngine(g_pDebug);
		
		SetPointer(0x8E249C, D3DSystemHook);
		SetPointer(0x8E24A8, SetRenderStateHook);
		SetPointer(0x8E24AC, GetRenderStateHook);
		
		SetPointer(0x8E24B8, im2dDrawPrim);
		SetPointer(0x8E24BC, im2DRenderIndexedPrimitive);
		SetPointer(0x8E24B0, im2DRenderLine);
		//SetPointer(0x8E291C, envMapSupport);//submitnode
		SetPointer(0x8E297C, im3dSubmit);//submitnodeNoLight
		SetPointer(0x8D633C, _D3D9AtomicAllInOneNode);//submitnode
		SetPointer(0x8DED0C, _D3D9SkinAllInOneNode);//skinallinone
		SetPointer(0x7578AE, _rxD3D9DefaultRenderCallback);
		SetPointer(0x7CB24B, _rxD3D9DefaultRenderCallback);//skinrender

		SetPointer(0x757899, _rxD3D9DefaultInstanceCallback);

		RedirectCall(0x74631E, SetRR);
		RedirectCall(0x745C75, SetVM);
		RedirectCall(0x53EA0D, im3dOpen);//shadowrenderUpdate
		
		RedirectCall(0x619D40, psNativeTextureSupport);
		
		RedirectJump(0x80A225, envMapSupport);//im3dopen
		
		RedirectCall(0x748A30, CGamma__init);
		RedirectCall(0x5DA044, envMapSupport);
		SetInt(0x7488DB, 5);
		//CRwD3DEngine::PatchGame();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete g_pRwCustomEngine;
		break;
	}
	return TRUE;
}

