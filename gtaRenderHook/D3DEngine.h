#ifndef D3DENGINE_H
#define D3DENGINE_H
#include "RwRenderEngine.h"
#include <vector>
#include <list>
struct d3dRasterInfo
{
	D3DFORMAT fmt;
	byte depth;
	bool alpha;
};
enum RasterFormat8
{
	FORMATDEFAULT = 0x00,
	FORMAT1555 = 0x01,
	FORMAT565 = 0x02,
	FORMAT4444 = 0x03,
	FORMATLUM8 = 0x04,
	FORMAT8888 = 0x05,
	FORMAT888 = 0x06,
	FORMAT16 = 0x07,
	FORMAT24 = 0x08,
	FORMAT32 = 0x09,
	FORMAT555 = 0x0a,
	FORMATAUTOMIPMAP = 0x10,
	FORMATPAL8 = 0x20,
	FORMATPAL4 = 0x40,
	FORMATMIPMAP = 0x80,
	FORMATFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
struct DVB_Manager
{
	int offset;
	int size;
	IDirect3DVertexBuffer9* vertexbuffer;
};
class CRwD3DEngine : public CIRwRenderEngine
{
	struct dVB
	{
		UINT size;
		IDirect3DVertexBuffer9* vb;
	};
public:
	CRwD3DEngine(CDebug* d) :CIRwRenderEngine(d), m_enableMultithreadSafe{ false }, m_enableSoftwareVertexProcessing{ false }, 
		m_displayModes{ nullptr }, m_vertexDeclarations{ nullptr }, m_numVertexDeclarations{ 0 }, m_maxVertexDeclarations{0},
		m_currentBaseIndex{ 0 }, m_vertexDeclIm2D{ nullptr }, m_currentDVBMgr{ 0 }, m_dynamicVertexBufferList{nullptr} {};
	bool BaseEventHandler(int State, int* a2, void* a3, int a4) override;
	// ****** NOTE: Events
	bool Open(HWND*) override;
	bool Close() override;
	bool Start() override;
	bool Stop() override;
	bool GetNumModes(int&) override;
	bool GetModeInfo(RwVideoMode&, int) override;
	bool UseMode(int) override;
	bool GetMode(int&) override;
	bool Standards(int*, int) override;
	bool GetNumSubSystems(int&) override;
	bool GetSubSystemInfo(RwSubSystemInfo&, int) override;
	bool GetCurrentSubSystem(int&) override;
	bool SetSubSystem(int) override;
	bool Focus(bool) override;
	bool GetTexMemSize(int&) override;
	bool GetMaxTextureSize(int&) override;
	// ****** NOTE: Not events
	
	bool Test(void *pOut, void *pInOut, RwInt32 nI);
	bool CameraClear(RwCamera *camera, RwRGBA *color, RwInt32 flags);
	bool CameraBeginUpdate(RwCamera *camera);
	bool CameraEndUpdate(RwCamera *camera);
	
	bool RasterCreate(RwRaster *raster,UINT flags);
	bool RasterDestroy(RwRaster *raster);
	
	bool RasterLock(RwRaster *raster, UINT flags,void** data);
	bool RasterUnlock(RwRaster *raster);
	
	bool RasterShowRaster(RwRaster *raster, UINT flags);
	bool NativeTextureRead(RwStream *stream, RwTexture** tex);

	// ****** NOTE: not standard functions
	bool CreateVertexDeclaration(D3DVERTEXELEMENT9* elements,IDirect3DVertexDeclaration9** decl);
	void DeleteVertexDeclaration(IDirect3DVertexDeclaration9* decl);

	bool CreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader);

	bool DynamicVertexBufferCreate(UINT size, IDirect3DVertexBuffer9** vertexbuffer);
	bool DynamicVertexBufferLock(RwUInt32 vertexSize, RwUInt32 numVertex, IDirect3DVertexBuffer9 **vertexBufferOut, void **vertexDataOut, RwUInt32 *baseIndexOut);
	
	void SetTransform(D3DTRANSFORMSTATETYPE state, RwMatrix *matrix);

	void ReleaseVideoMemory();
	int GetMaxMultiSamplingLevels() override;
	void SetMultiSamplingLevels(int) override;

	bool RenderStateSet(RwRenderState, void *) override;
	bool RenderStateGet(RwRenderState, void *) override;

	void Im3DRenderOpen();
	static void PatchGame();

	bool Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices) override;
	bool m_enableMultithreadSafe, m_enableSoftwareVertexProcessing, m_windowed, m_systemStarted, m_insideScene, m_hasStencilBuffer,
		m_useOwnVertexBuffer;

	int m_numDisplayModes, m_numAdapterModes, m_currentModeIndex, m_depth,
		m_maxMultisamplingLevels, m_maxMultisamplingLevelsNonMask, m_selectedMultisamplingLevels, m_selectedMultisamplingLevelsNonMask,
		m_numVertexDeclarations, m_maxVertexDeclarations, m_ZBufferDepth, m_maxNumLights,
		m_currentDVBMgr;
	UINT m_adapterIndex, m_currentBaseIndex;

	RwDisplayMode* m_displayModes;
	RwD3D9VertexDecl* m_vertexDeclarations;

	HWND m_windowHandle;
	IDirect3D9* m_d3d9;
	IDirect3DDevice9* m_device;
	D3DDEVTYPE m_deviceType;
	D3DDISPLAYMODE m_displayMode;
	D3DPRESENT_PARAMETERS m_presentParams;
	D3DCAPS9 m_deviceCaps;
	IDirect3DSurface9* m_renderSurface, *m_depthStencilSurface, *m_currentRenderSurface[4], *m_currentDepthStencilSurface;
	DVB_Manager m_dynamicVertexBufferMgrs[4]{};
	IDirect3DVertexBuffer9* m_currentVertexBuffer;
	IDirect3DVertexDeclaration9* m_vertexDeclIm2D;
	list<dVB> *m_dynamicVertexBufferList;
private:
	const D3DFORMAT m_aBackBufferFormat[3]{ D3DFMT_R5G6B5, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10 };
	const d3dRasterInfo m_rasterConvertTable[11]{	{ D3DFMT_UNKNOWN, 0, false }, { D3DFMT_A1R5G5B5, 16, true }, { D3DFMT_R5G6B5, 16, false }, { D3DFMT_A4R4G4B4, 16, true },
													{ D3DFMT_L8, 8, false }, { D3DFMT_A8R8G8B8, 32, true },	{ D3DFMT_X8R8G8B8, 32, false },{ D3DFMT_D16, 16, false },
													{ D3DFMT_D24X8, 32, false },{ D3DFMT_D32, 32, false },{ D3DFMT_X1R5G5B5, 16, false } };
	const D3DPRIMITIVETYPE m_primConvTable[7]{ (D3DPRIMITIVETYPE)NULL,D3DPT_LINELIST,D3DPT_LINESTRIP,D3DPT_TRIANGLELIST,D3DPT_TRIANGLESTRIP,D3DPT_TRIANGLEFAN,D3DPT_POINTLIST };
	void m_setPresentParameters(const D3DDISPLAYMODE&);
	void m_setDepthStencilSurface(IDirect3DSurface9 *);
	void m_setRenderTarget(int, IDirect3DSurface9 *);

	void m_im2DRenderFlush();

	void m_clearCacheShaders();
	void m_clearCacheMatrix();
	void m_clearCacheLights();

	bool m_resetDevice();

	void m_createDisplayModeList();
	void m_calculateMaxMSLevels();
	bool m_DefaultStandard(void *pOut, void *pInOut, int nI) { return false; }

	void m_rasterOpen();
	void m_im2DRenderOpen();
	void m_vertexBufferManagerOpen();

	void m_rasterClose();
	void m_im2DRenderClose();
	void m_vertexBufferManagerClose();
	
	bool m_videoMemoryRasterListRestore();
	void m_dynamicVertexBufferRestore();

	void m_videoMemoryRasterListAdd(RwRaster* r);
	void m_videoMemoryRasterListRemove(RwRaster* r);

	void m_dynamicVertexBufferManagerCreate();

	bool m_checkTextureFormat(RwRaster* raster,UINT flags);

	bool m_createZBufferRaster(RwRaster *raster, RwD3D9Raster *d3dRaster);
	bool m_createTextureRaster(RwRaster *raster, RwD3D9Raster *d3dRaster);

	bool m_checkValidTextureFormat(D3DFORMAT fmt);
	bool m_checkValidZBufferFormat(D3DFORMAT fmt);
	bool m_checkValidZBufferTextureFormat(D3DFORMAT fmt);
	bool m_checkAutoMipmapGenTextureFormat(D3DFORMAT fmt);

	int m_getRasterFormat(D3DFORMAT fmt);
	int m_getDepthValue(D3DFORMAT fmt);

	
};
extern CIRwRenderEngine* g_pRwCustomEngine;

#endif