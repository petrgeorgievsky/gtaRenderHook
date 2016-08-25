#pragma once
#include "RwRenderEngine.h"
class CD3DRenderer;
class CD3D1XIm2DPipeline;
class CD3D1XIm3DPipeline;
class CD3D1XDefaultPipeline;
class CD3D1XSkinPipeline;

class CRwD3D1XEngine :
	public CIRwRenderEngine
{
public:
	CRwD3D1XEngine(CDebug* d);

private:// D3D11 Instance methods.
	rxInstanceData* m_D3DInstance		(void* object,void* instanceObject, RwUInt8 type, RwResEntry ** repEntry, rpD3DMeshHeader* mesh, RxD3D9AllInOneInstanceCallBack instance, int bNativeInstance);

protected:
	rxInstanceData* m_D3DSkinInstance	(void* object, void* instanceObject, RwResEntry ** repEntry, rpD3DMeshHeader* mesh);

private:// D3D11 API objects
	CD3DRenderer*			m_pRenderer				= nullptr;
	CD3D1XIm2DPipeline*		m_pIm2DPipe				= nullptr;
	CD3D1XIm3DPipeline*		m_pIm3DPipe				= nullptr;
	CD3D1XDefaultPipeline*	m_pDefaultPipe			= nullptr;
	CD3D1XSkinPipeline*		m_pSkinPipe				= nullptr;

private://Virtual methods.
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
	virtual int GetMaxMultiSamplingLevels() override;
	virtual void SetMultiSamplingLevels(int) override;
	virtual bool RenderStateSet(RwRenderState, UINT) override;
	virtual bool RenderStateGet(RwRenderState, UINT&) override;
	virtual bool Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices) override;
	virtual bool Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices) override;
	virtual bool RasterCreate(RwRaster *raster, UINT flags) override;
	virtual bool RasterDestroy(RwRaster *raster) override;
	virtual bool NativeTextureRead(RwStream *stream, RwTexture** tex) override;
	virtual bool RasterLock(RwRaster *raster, UINT flags, void** data) override;
	virtual bool RasterUnlock(RwRaster *raster) override;
	virtual bool CameraClear(RwCamera *camera, RwRGBA *color, RwInt32 flags) override;
	virtual bool CameraBeginUpdate(RwCamera *camera) override;
	virtual bool CameraEndUpdate(RwCamera *camera) override;
	virtual bool RasterShowRaster(RwRaster *raster, UINT flags) override;
	virtual bool AtomicAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params) override;
	virtual void DefaultRenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags) override;
	virtual RwBool DefaultInstanceCallback(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance) override;
	virtual RwBool Im3DSubmitNode() override;
	virtual void SetTexture(RwTexture * tex, int Stage) override;
	virtual bool SkinAllInOneNode(RxPipelineNode * self, const RxPipelineNodeParam * params) override;
	virtual RwTexture * CopyTexture(RwTexture * tex) override;
};

