#pragma once
#include "RwRenderEngine.h"
class CD3DRenderer;
class CD3D1XIm2DPipeline;
class CD3D1XIm3DPipeline;
class CD3D1XDefaultPipeline;
class CD3D1XSkinPipeline;
class CD3D1XVertexDeclarationManager;
/*!
    \brief Graphics matrix(in homogenous space)

    RenderWare treated all matrices the same, but often in graphics
    we need to use 4th column-vector so this structure is needed to
    remap renderware matrices to 4x4 matrices.
*/
struct RwGraphicsMatrix
{
    RwV4d m[4];
};
class CRwD3D1XEngine :
    public CIRwRenderEngine
{
public:
    CRwD3D1XEngine( CDebug* d );
    CD3DRenderer*			getRenderer() { return m_pRenderer; }

private:// D3D11 Instance methods.
    RxInstanceData* m_D3DInstance( void* object, void* instanceObject, RwUInt8 type, RwResEntry ** repEntry, RpD3DMeshHeader* mesh, RxD3D9AllInOneInstanceCallBack instance, int bNativeInstance );

protected:
    RxInstanceData* m_D3DSkinInstance( void* object, void* instanceObject, RwResEntry ** repEntry, RpD3DMeshHeader* mesh );

private:// D3D11 API objects
    CD3DRenderer*			m_pRenderer = nullptr;
    CD3D1XIm2DPipeline*		m_pIm2DPipe = nullptr;
    CD3D1XIm3DPipeline*		m_pIm3DPipe = nullptr;
    CD3D1XDefaultPipeline*	m_pDefaultPipe = nullptr;
    CD3D1XSkinPipeline*		m_pSkinPipe = nullptr;

private://Virtual methods.
    virtual bool Open( HWND ) override;
    virtual bool Close() override;
    virtual bool Start() override;
    virtual bool Stop() override;

    virtual bool GetNumModes( int& ) override;
    virtual bool GetModeInfo( RwVideoMode&, int ) override;
    virtual bool UseMode( int ) override;
    virtual bool GetMode( int& ) override;

    virtual bool Focus( bool ) override;
    virtual bool Standards( int*, int ) override;


    virtual bool GetNumSubSystems( int& ) override;
    virtual bool GetSubSystemInfo( RwSubSystemInfo&, int ) override;
    virtual bool GetCurrentSubSystem( int& ) override;
    virtual bool SetSubSystem( int ) override;

    virtual bool GetTexMemSize( int& ) override;
    virtual bool GetMaxTextureSize( int& ) override;
    virtual int  GetMaxMultiSamplingLevels() override;
    virtual void SetMultiSamplingLevels( int ) override;

    virtual bool BaseEventHandler( int State, int* a2, void* a3, int a4 ) override;

    virtual bool RenderStateSet( RwRenderState, UINT ) override;
    virtual bool RenderStateGet( RwRenderState, UINT& ) override;

    virtual bool Im2DRenderPrimitive( RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices ) override;
    virtual bool Im2DRenderIndexedPrimitive( RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices ) override;

    virtual bool RasterCreate( RwRaster *raster, UINT flags ) override;
    virtual bool RasterDestroy( RwRaster *raster ) override;

    virtual bool NativeTextureRead( RwStream *stream, RwTexture** tex ) override;

    virtual bool RasterLock( RwRaster *raster, UINT flags, void** data ) override;
    virtual bool RasterUnlock( RwRaster *raster ) override;

    virtual bool CameraClear( RwCamera *camera, RwRGBA *color, RwInt32 flags ) override;
    virtual bool CameraBeginUpdate( RwCamera *camera ) override;
    virtual bool CameraEndUpdate( RwCamera *camera ) override;

    virtual bool RasterShowRaster( RwRaster *raster, UINT flags ) override;

    virtual bool AtomicAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params ) override;
    virtual bool SkinAllInOneNode( RxPipelineNode * self, const RxPipelineNodeParam * params ) override;

    virtual void DefaultRenderCallback( RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags ) override;
    virtual RwBool DefaultInstanceCallback( void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance ) override;

    virtual RwBool Im3DSubmitNode() override;

    virtual void SetTexture( RwTexture * tex, int Stage ) override;

    // Inherited via CIRwRenderEngine
    virtual void SetRenderTargets( RwRaster ** rasters, RwRaster *zBuffer, RwUInt32 rasterCount ) override;

public:
    void SetRenderTargetsAndUAVs( RwRaster ** rasters, RwRaster ** uavs, RwRaster *zBuffer, RwUInt32 rasterCount, RwUInt32 uavCount, RwUInt32 uavStart );
    void ReloadTextures();
    RwRaster * CreateD3D9Raster( RwInt32 width, RwInt32 height, D3DFORMAT format, RwInt32 flags );
    std::list<RwRaster*> m_pRastersToReload{};
    bool m_bScreenSizeChanged = false;
    bool m_bAntTweakBarAvaliable = false;
};

