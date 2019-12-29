/**
    @file RwRenderEngine.h
    @author Peter Georgievsky
    @date 07.07.2018
    @version 1.0.0

    @brief This file conatains common interface declaration for RenderWare
   wrapper for RenderHook rendering engine.

    Render Hook engine is based mostly on RenderWare engine,
    but it doesn't follow philosophy of RW engine fully,
    so it could be used as rendering engine on it's own without conveing to old
   32bit standards of RW. This file contains the wrapper interface of common RW
   rendering functions.
*/
#pragma once
#include "Engine\Common\ISimple2DRenderer.h"
#include "Engine\IRenderer.h"
#include <Engine\Definitions.h>
#include <game_sa\RenderWare.h>

namespace rh::rw::engine {
struct RwStandard
{
    RwInt32 nStandard;
    RwStandardFunc fpStandard;
};

using RwRenderEngineGenericCB = void ( * )();
using RwRenderEngineOutputResizeCB = void ( * )( uint32_t w, uint32_t h );

/**
    This enum contains all possible requests to rendering system of RenderWare
*/
enum class RwRenderSystemRequest {
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

/**
 * @class RwRenderEngine
 * @brief A wrapper between RenderWare and graphics API-specific stuff.
 * *This wrapper contains all the needed methods to wrap around all the methods
 * of RenderWare engine
 */
class RwRenderEngine
{
public:
    /**
   * @brief Opens rendering engine
   *
   * @return true - if rendering engine opens correctly
   * @return false - if error occured
   *
   * Assigns window handle and enumerates hardware info(videocards, monitors and
   * display modes).
   */
    bool Open( HWND );

    /**
   * @brief Closes rendering engine, releasing all used resources
   * @return true if rendering engine closes correctly, false otherwise
   */
    bool Close();

    /**
   * @brief Creates logical device, binded to selected hardware
   * @return true if device is created, false otherwise
   */
    bool Start();

    /**
   * @brief Destroys logical device and all resources allocated by that device
   * @return true , false otherwise
   */
    bool Stop();

    /**
   * @brief Get the count of display modes avaliable for selected monitor
   *
   * @return true - if display mode count retrieved correctly
   * @return false - if error occured
   */
    bool GetNumModes( int & );

    /*
Returns info about display mode
*/
    bool GetModeInfo( RwVideoMode &, int );

    /*
Selects current display mode.
*/
    bool UseMode( unsigned int );

    /*
Pocus
*/
    bool Focus( bool );

    /*
Returns currently selected display mode.
*/
    bool GetMode( int & );

    bool Standards( int *, int );

    bool GetTexMemSize( int & );

    /*
Returns subsystems(fancy name for GPUs or adapters) count
*/
    bool GetNumSubSystems( unsigned int & );

    /*
Returns subsystem info(name to be more exact)
*/
    bool GetSubSystemInfo( RwSubSystemInfo &, unsigned int );

    /*
Returns currently selected subsystem
*/
    bool GetCurrentSubSystem( int & );

    /*
Selects current subsystem.
*/
    bool SetSubSystem( unsigned int );

    /*
Returns output(monitor) count
*/
    bool GetNumOutputs( unsigned int, int & );

    /*
Returns output info
*/
    bool GetOutputInfo( std::string &info, unsigned int n );

    /*
Returns currently selected output
*/
    bool GetCurrentOutput( int & );

    /*
Selects current output.
*/
    bool SetOutput( unsigned int );

    bool GetMaxTextureSize( int & );

    // Base event handler to handle events that doesn't use any API-dependent
    // methods. TODO: remove this method and add few more(implement custom
    // renderware RwDevice handle return)
    bool BaseEventHandler( int State, int *a2, void *a3, int a4 );

    // State get-set methods.
    void SetMultiSamplingLevels( int );
    int GetMaxMultiSamplingLevels();
    bool RenderStateSet( RwRenderState, UINT );
    bool RenderStateGet( RwRenderState, UINT & );

    // Raster/Texture/Camera methods
    bool RasterLock( RwRaster *raster, UINT flags, void **data );
    bool RasterUnlock( RwRaster *raster );

    // Immediate mode render methods.
    bool Im2DRenderPrimitive( RwPrimitiveType primType,
                              RwIm2DVertex *vertices,
                              RwUInt32 numVertices );
    bool Im2DRenderIndexedPrimitive( RwPrimitiveType primType,
                                     RwIm2DVertex *vertices,
                                     RwUInt32 numVertices,
                                     RwImVertexIndex *indices,
                                     RwInt32 numIndices );
    RwBool Im3DSubmitNode();

    void SetTexture( RwTexture *tex, int Stage );

    bool AtomicAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params );
    bool SkinAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params );

    void DefaultRenderCallback( RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags );
    RwBool DefaultInstanceCallback( void *object,
                                    RxD3D9ResEntryHeader *resEntryHeader,
                                    RwBool reinstance );

    RwRenderEngine( rh::engine::RenderingAPI api ) { m_renderingAPI = api; }
    virtual ~RwRenderEngine() = default;

    // Render engine event system.
    bool EventHandlingSystem( RwRenderSystemRequest request, int *pOut, void *pInOut, int nIn );

    static void RegisterPostInitCallback( RwRenderEngineGenericCB cb );
    static void RegisterPreShutdownCallback( RwRenderEngineGenericCB cb );
    static void RegisterOutputResizeCallback( RwRenderEngineOutputResizeCB cb );

    std::unique_ptr<rh::engine::ISimple2DRenderer> m_p2DRenderer = nullptr;

private:
    rh::engine::RenderingAPI m_renderingAPI;
    static std::vector<RwRenderEngineGenericCB> mPostInitCallbacks;
    static std::vector<RwRenderEngineGenericCB> mPreShutdownCallbacks;
    static std::vector<RwRenderEngineOutputResizeCB> mOutputResizeCallbacks;
};
extern std::unique_ptr<RwRenderEngine> g_pRwRenderEngine;
extern bool g_bRwSwapchainChanged;
}; // namespace rh::rw::engine
