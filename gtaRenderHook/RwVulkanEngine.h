#ifndef RwVulkanEngine_h__
#define RwVulkanEngine_h__

#include "RwRenderEngine.h"
class CVulkanRenderer;
class CVulkanIm2DPipeline;

class CRwVulkanEngine :
	public CIRwRenderEngine
{
public:
	CRwVulkanEngine(CDebug* d);

	virtual bool Open(HWND) override;
	virtual bool Close() override;
	virtual bool Start() override;
	virtual bool Stop() override;
	virtual bool GetNumModes(int&) override;
	virtual bool GetModeInfo(RwVideoMode&, int) override;
	virtual bool UseMode(int) override;
	virtual bool Focus(bool) override;
	virtual bool GetMode(int&) override;
	virtual bool Standards(int*, int) override;
	virtual bool GetTexMemSize(int&) override;
	virtual bool GetNumSubSystems(int&) override;
	virtual bool GetSubSystemInfo(RwSubSystemInfo&, int) override;
	virtual bool GetCurrentSubSystem(int&) override;
	virtual bool SetSubSystem(int) override;
	virtual bool GetMaxTextureSize(int&) override;
	virtual bool BaseEventHandler(int State, int* a2, void* a3, int a4) override;
	virtual int	 GetMaxMultiSamplingLevels() override;
	virtual void SetMultiSamplingLevels(int) override;
	virtual bool RenderStateSet(RwRenderState, UINT) override;
	virtual bool RenderStateGet(RwRenderState, UINT&) override;
	virtual bool Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices) override;
	virtual bool Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices) override;
	virtual bool RasterCreate(RwRaster *raster, UINT flags) override;
	virtual bool NativeTextureRead(RwStream *stream, RwTexture** tex) override;
	virtual bool RasterLock(RwRaster *raster, UINT flags, void** data) override;
	virtual bool RasterUnlock(RwRaster *raster) override;
	virtual bool CameraClear(RwCamera *camera, RwRGBA *color, RwInt32 flags) override;
	virtual bool CameraBeginUpdate(RwCamera *camera) override;
	virtual bool CameraEndUpdate(RwCamera *camera) override;
	virtual bool RasterShowRaster(RwRaster *raster, UINT flags) override;
	virtual bool RasterDestroy(RwRaster *raster) override;
	virtual bool AtomicAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params) override;

	virtual RwBool Im3DSubmitNode() override;

	virtual void DefaultRenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags) override;

	virtual RwBool DefaultInstanceCallback(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance) override;

private:
	CVulkanRenderer* m_pRenderer;
	CVulkanIm2DPipeline* m_pIm2DPipeline;
	bool m_insideScene = false;
	RwRaster*	whiteRaster=nullptr;
};
#endif // RwVulkanEngine_h__
