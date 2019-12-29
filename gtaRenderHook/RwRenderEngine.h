#ifndef RW_RENDER_ENGINE_H
#define RW_RENDER_ENGINE_H
#include "CDebug.h"
enum class RwRenderSystemState
{
    rwDEVICESYSTEMOPEN = 0x00,
    rwDEVICESYSTEMCLOSE,
    rwDEVICESYSTEMSTART,
    rwDEVICESYSTEMSTOP,
    rwDEVICESYSTEMREGISTER,
    rwDEVICESYSTEMGETNUMMODES,
    rwDEVICESYSTEMGETMODEINFO,
    rwDEVICESYSTEMUSEMODE,
    rwDEVICESYSTEMFOCUS,
    rwDEVICESYSTEMINITPIPELINE,
    rwDEVICESYSTEMGETMODE,
    rwDEVICESYSTEMSTANDARDS,
    rwDEVICESYSTEMGETTEXMEMSIZE,
    rwDEVICESYSTEMGETNUMSUBSYSTEMS,
    rwDEVICESYSTEMGETSUBSYSTEMINFO,
    rwDEVICESYSTEMGETCURRENTSUBSYSTEM,
    rwDEVICESYSTEMSETSUBSYSTEM,
    rwDEVICESYSTEMFINALIZESTART,
    rwDEVICESYSTEMINITIATESTOP,
    rwDEVICESYSTEMGETMAXTEXTURESIZE,
    rwDEVICESYSTEMRXPIPELINEREQUESTPIPE,
    rwDEVICESYSTEMGETMETRICBLOCK,
    rwDEVICESYSTEMGETID,
};
bool mDefStd( void *, void *, int );
bool mRasterCreate( void * a, void * b, int c );
bool mNativeTextureRead( void * a, void * b, int c );
bool mRasterLock( void * a, void * b, int c );
bool mRasterUnlock( void * a, void * b, int c );
bool mCamClear( void * a, void * b, int c );
bool mCamBU( void * a, void * b, int c );
bool mCameraEndUpdate( void * a, void * b, int c );
bool mRasterShowRaster( void * a, void * b, int c );
bool mRasterDestroy( void * a, void * b, int c );
/*typedef bool(*RwStandardFunc)(void *pOut, void *pInOut, int nI);*/
// RenderWare rendering engine interface(allows you to implement different rendering APIs like DX12, Vulkan or OpenGL).
class CIRwRenderEngine
{
public:
    // Render system events
    virtual bool Open( HWND ) = 0;
    virtual bool Close() = 0;
    virtual bool Start() = 0;
    virtual bool Stop() = 0;
    virtual bool GetNumModes( int& ) = 0;
    virtual bool GetModeInfo( RwVideoMode&, int ) = 0;
    virtual bool UseMode( int ) = 0;
    virtual bool Focus( bool ) = 0;
    virtual bool GetMode( int& ) = 0;
    virtual bool Standards( int*, int ) = 0;
    virtual bool GetTexMemSize( int& ) = 0;
    virtual bool GetNumSubSystems( int& ) = 0;
    virtual bool GetSubSystemInfo( RwSubSystemInfo&, int ) = 0;
    virtual bool GetCurrentSubSystem( int& ) = 0;
    virtual bool SetSubSystem( int ) = 0;
    virtual bool GetMaxTextureSize( int& ) = 0;

    // Base event handler to handle events that doesn't use any API-dependent methods. TODO: remove this method and add few more(implement custom renderware RwDevice handle return)
    virtual bool BaseEventHandler( int State, int* a2, void* a3, int a4 ) = 0;

    // State get-set methods.
    virtual void	SetMultiSamplingLevels( int ) = 0;
    virtual int		GetMaxMultiSamplingLevels() = 0;
    virtual bool	RenderStateSet( RwRenderState, UINT ) = 0;
    virtual bool	RenderStateGet( RwRenderState, UINT& ) = 0;

    // Raster/Texture/Camera methods
    virtual bool	RasterCreate( RwRaster *raster, UINT flags ) = 0;
    virtual bool	RasterDestroy( RwRaster *raster ) = 0;
    virtual bool	RasterLock( RwRaster *raster, UINT flags, void** data ) = 0;
    virtual bool	RasterUnlock( RwRaster *raster ) = 0;
    virtual bool	RasterShowRaster( RwRaster *raster, UINT flags ) = 0;
    virtual bool	NativeTextureRead( RwStream *stream, RwTexture** tex ) = 0;
    virtual bool	CameraClear( RwCamera *camera, RwRGBA *color, RwInt32 flags ) = 0;
    virtual bool	CameraBeginUpdate( RwCamera *camera ) = 0;
    virtual bool	CameraEndUpdate( RwCamera *camera ) = 0;
    virtual void	SetRenderTargets( RwRaster **rasters, RwRaster *zBuffer, RwUInt32 rasterCount ) = 0;

    // Immediate mode render methods.
    virtual bool	Im2DRenderPrimitive( RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices ) = 0;
    virtual bool	Im2DRenderIndexedPrimitive( RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices ) = 0;
    virtual RwBool	Im3DSubmitNode() = 0;

    virtual void		SetTexture( RwTexture* tex, int Stage ) = 0;

    virtual bool AtomicAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params ) = 0;
    virtual bool SkinAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params ) = 0;

    virtual void	DefaultRenderCallback( RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags ) = 0;
    virtual RwBool	DefaultInstanceCallback( void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance ) = 0;

    CIRwRenderEngine( CDebug *d ) { m_pDebug = d; };
    virtual ~CIRwRenderEngine() { };

    // Render engine event system.
    bool EventHandlingSystem( RwRenderSystemState State, int* a2, void* a3, int a4 );
protected:
    CDebug* m_pDebug;
};
extern CIRwRenderEngine* g_pRwCustomEngine;
#define GET_D3D_RENDERER \
	(static_cast<CRwD3D1XEngine*>(g_pRwCustomEngine)->getRenderer())
#define GET_D3D_DEVICE \
	(GET_D3D_RENDERER->getDevice())
#define GET_D3D_CONTEXT \
	(GET_D3D_RENDERER->getContext())
#define GET_D3D_FEATURE_LVL \
	(GET_D3D_RENDERER->getFeatureLevel())
#define GET_D3D_SWAP_CHAIN \
	(GET_D3D_RENDERER->getSwapChain())
#endif 

