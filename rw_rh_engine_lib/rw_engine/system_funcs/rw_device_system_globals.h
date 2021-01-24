#pragma once
#include "../../common_headers.h"
#include <atomic>
#include <cstdint>
#include <ipc/shared_memory_queue_client.h>
#include <memory>
#include <render_client/render_client.h>
#include <render_driver/render_driver.h>
#include <thread>

using RwSystemFunc = int32_t ( * )( int32_t nOption, void *pOut, void *pInOut,
                                    int32_t nIn );

// TODO: MOVE from here
enum RwCoreDeviceSystemFn
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
    rwDEVICESYSTEMDD = 0x1000
};

enum RwRenderState : int32_t;
using RwRenderStateSetFunction = int32_t ( * )( RwRenderState nState,
                                                void *        pParam );
using RwRenderStateGetFunction = int32_t ( * )( RwRenderState nState,
                                                void *        pParam );
using RwIm2DRenderLineFunction = int32_t ( * )( RwIm2DVertex *vertices,
                                                int32_t       numVertices,
                                                int32_t vert1, int32_t vert2 );

using RwIm2DRenderTriangleFunction = int32_t ( * )( RwIm2DVertex *vertices,
                                                    int32_t       numVertices,
                                                    int32_t       vert1,
                                                    int32_t       vert2,
                                                    int32_t       vert3 );

using RwIm2DRenderPrimitiveFunction = int32_t ( * )( RwPrimitiveType primType,
                                                     RwIm2DVertex *  vertices,
                                                     int32_t numVertices );

using RwIm2DRenderIndexedPrimitiveFunction =
    int32_t ( * )( RwPrimitiveType primType, RwIm2DVertex *vertices,
                   int32_t numVertices, int16_t *indices, int32_t numIndices );

using RwIm3DRenderLineFunction = int32_t ( * )( int32_t vert1, int32_t vert2 );

using RwIm3DRenderTriangleFunction = int32_t ( * )( int32_t vert1,
                                                    int32_t vert2,
                                                    int32_t vert3 );

using RwIm3DRenderPrimitiveFunction = int32_t ( * )( RwPrimitiveType primType );

using RwIm3DRenderIndexedPrimitiveFunction = int32_t ( * )(
    RwPrimitiveType primtype, int16_t *indices, int32_t numIndices );

struct RwDevice
{
    float        gammaCorrection; /* Gamma correction  */
    RwSystemFunc fpSystem;        /* System handler */
    float        zBufferNear;     /* Near Z buffer value */
    float        zBufferFar;      /* Far Z buffer value */

    /* Immediate mode functions */
    RwRenderStateSetFunction fpRenderStateSet; /* Internal Use */
    RwRenderStateGetFunction fpRenderStateGet; /* Internal Use */

    /* Render functions */
    RwIm2DRenderLineFunction      fpIm2DRenderLine;      /* Internal Use */
    RwIm2DRenderTriangleFunction  fpIm2DRenderTriangle;  /* Internal Use */
    RwIm2DRenderPrimitiveFunction fpIm2DRenderPrimitive; /* Internal Use */
    RwIm2DRenderIndexedPrimitiveFunction
        fpIm2DRenderIndexedPrimitive; /* Internal Use */

    RwIm3DRenderLineFunction      fpIm3DRenderLine;      /* Internal Use */
    RwIm3DRenderTriangleFunction  fpIm3DRenderTriangle;  /* Internal Use */
    RwIm3DRenderPrimitiveFunction fpIm3DRenderPrimitive; /* Internal Use */
    RwIm3DRenderIndexedPrimitiveFunction
        fpIm3DRenderIndexedPrimitive; /* Internal Use */
};

struct RwCamera;
struct RwMemoryFunctions;
struct RwRwDeviceGlobals
{
    /* Current camera */
    RwCamera *curCamera;

    /* Memory allocators */
    RwMemoryFunctions *memFuncs;
};

enum RwVideoModeFlag : uint32_t
{
    rwVIDEOMODEEXCLUSIVE   = 0x0001, /**<Exclusive (i.e. full-screen) */
    rwVIDEOMODEINTERLACE   = 0x0002, /**<Interlaced                   */
    rwVIDEOMODEFFINTERLACE = 0x0004, /**<Flicker Free Interlaced */

    /* Platform specific video mode flags. */

    rwVIDEOMODE_PS2_FSAASHRINKBLIT = 0x0100,
    /**< \if sky2
     *   Full-screen antialiasing mode 0
     *   \endif
     */
    rwVIDEOMODE_PS2_FSAAREADCIRCUIT = 0x0200,
    /**< \if sky2
     *   Full-screen antialiasing mode 1
     *   \endif
     */

    rwVIDEOMODE_XBOX_WIDESCREEN = 0x0100,
    /**< \if xbox
     *   Wide screen.
     *   \endif
     */
    rwVIDEOMODE_XBOX_PROGRESSIVE = 0x0200,
    /**< \if xbox
     *   Progressive.
     *   \endif
     */
    rwVIDEOMODE_XBOX_FIELD = 0x0400,
    /**< \if xbox
     *   Field rendering.
     *   \endif
     */
    rwVIDEOMODE_XBOX_10X11PIXELASPECT = 0x0800,
    /**< \if xbox
     *   The frame buffer is centered on the display.
     *   On a TV that is 704 pixels across, this would leave 32 pixels
     * of black border on the left and 32 pixels of black border on the
     * right. \endif
     */
};

enum RwDeviceStandardFn
{
    rwSTANDARDNASTANDARD,
    rwSTANDARDCAMERABEGINUPDATE,     /* Start 3d camera update */
    rwSTANDARDRGBTOPIXEL,            /* For an RGB value return a pixel value */
    rwSTANDARDPIXELTORGB,            /* For a pixel value returns a RGB value */
    rwSTANDARDRASTERCREATE,          /* Create an raster */
    rwSTANDARDRASTERDESTROY,         /* Raster destroy */
    rwSTANDARDIMAGEGETRASTER,        /* Get image from a raster */
    rwSTANDARDRASTERSETIMAGE,        /* Set raster from an image */
    rwSTANDARDTEXTURESETRASTER,      /* Set texture's raster */
    rwSTANDARDIMAGEFINDRASTERFORMAT, /* Find a suitable raster format for an
                                        image */
    rwSTANDARDCAMERAENDUPDATE,       /* End 3d camera update */
    rwSTANDARDSETRASTERCONTEXT,      /* Start destination of 2d operations */
    rwSTANDARDRASTERSUBRASTER,       /* Make a raster inside another raster */
    rwSTANDARDRASTERCLEARRECT, /* Clear a rectangle of the current dest raster
                                */
    rwSTANDARDRASTERCLEAR,     /* Clear the current dest raster */
    rwSTANDARDRASTERLOCK,      /* Lock a raster to get it's pixels */
    rwSTANDARDRASTERUNLOCK,    /* Unlock a raster to return it's pixels */
    rwSTANDARDRASTERRENDER,    /* Render a raster (not scaled, but masked) */
    rwSTANDARDRASTERRENDERSCALED, /* Render a raster (scaled and masked) */
    rwSTANDARDRASTERRENDERFAST,   /* Render a raster (not scaled or masked) */
    rwSTANDARDRASTERSHOWRASTER,   /* Show a camera raster */
    rwSTANDARDCAMERACLEAR,        /* Clear a camera's raster and/or Z raster */
    rwSTANDARDHINTRENDERF2B, /* Set hint for rendering direction in the world */
    rwSTANDARDRASTERLOCKPALETTE,    /* Lock a raster to get it's palette */
    rwSTANDARDRASTERUNLOCKPALETTE,  /* Unlock a raster to return it's palette */
    rwSTANDARDNATIVETEXTUREGETSIZE, /* Get size of native texture when written
                                       to a stream */
    rwSTANDARDNATIVETEXTUREREAD,    /* Read native texture from the stream */
    rwSTANDARDNATIVETEXTUREWRITE,   /* Write native texture to the stream */
    rwSTANDARDRASTERGETMIPLEVELS, /* Get the number of mip levels in a raster */
    rwSTANDARDNUMOFSTANDARD,

};

struct RwVideoMode
{
    int32_t  width;   /**< Width  */
    int32_t  height;  /**< Height */
    int32_t  depth;   /**< Depth  */
    uint32_t flags;   /**< Flags  */
    int32_t  refRate; /**< Approximate refresh rate */
    int32_t  format;  /**< Raster format
                       * \see RwRasterFormat
                       */
};

using RwStandardFunc = int32_t ( * )( void *pOut, void *pInOut, int32_t nI );
namespace rh::engine
{
class IDeviceState;
class IWindow;
class IImageBuffer;
class IImageView;
} // namespace rh::engine

using RwPluginObjectConstructor = void *(*)( void *  object,
                                             int32_t offsetInObject,
                                             int32_t sizeInObject );
using RwPluginObjectDestructor  = void *(*)( void *  object,
                                            int32_t offsetInObject,
                                            int32_t sizeInObject );
using RwPluginObjectCopy = void *(*)( void *dstObject, const void *srcObject,
                                      int32_t offsetInObject,
                                      int32_t sizeInObject );
using RegisterPluginCall = int32_t ( * )( int32_t size, uint32_t pluginID,
                                          RwPluginObjectConstructor constructCB,
                                          RwPluginObjectDestructor  destructCB,
                                          RwPluginObjectCopy        copyCB );

using RwPluginDataChunkAlwaysCallBack = int32_t ( * )( void *  object,
                                                       int32_t offsetInObject,
                                                       int32_t sizeInObject );
using SetStreamAlwaysCallBack         = int32_t ( * )(
    uint32_t pluginID, RwPluginDataChunkAlwaysCallBack alwaysCB );

using RwResourcesAllocateResEntry = RwResEntry *(*)( void *, RwResEntry **,
                                                     int32_t,
                                                     RwResEntryDestroyNotify );
using RwResourcesFreeResEntry     = int32_t ( * )( RwResEntry *entry );

using RpSkinAtomicGetHAnimHierarchyFP =
    RpHAnimHierarchy *(*)( const RpAtomic *atomic );

using RpSkinGeometryGetSkinFP       = RpSkin *(*)( const RpGeometry *atomic );
using RpSkinGetSkinToBoneMatricesFP = const RwMatrix *(*)( RpSkin *skin );
using RpSkinGetVertexBoneWeightsFP = const RwMatrixWeights *(*)( RpSkin *skin );
using RpSkinGetVertexBoneIndicesFP = const uint32_t *(*)( RpSkin *skin );
namespace rh::rw::engine
{
int32_t SystemHandler( int32_t nOption, void *pOut, void *pInOut, int32_t nIn );

struct PluginPtrTable
{
    RegisterPluginCall      MaterialRegisterPlugin;
    SetStreamAlwaysCallBack MaterialSetStreamAlwaysCallBack;
    RegisterPluginCall      RasterRegisterPlugin;
    RegisterPluginCall      CameraRegisterPlugin;
};

struct SkinPtrTable
{
    RpSkinAtomicGetHAnimHierarchyFP AtomicGetHAnimHierarchy;
    RpSkinGeometryGetSkinFP         GeometryGetSkin;
    RpSkinGetSkinToBoneMatricesFP   GetSkinToBoneMatrices;
    RpSkinGetVertexBoneWeightsFP    GetVertexBoneWeights =
        []( RpSkin *skin ) -> const RwMatrixWeights * {
        return skin->vertexBoneWeights;
    };
    RpSkinGetVertexBoneIndicesFP GetVertexBoneIndices =
        []( RpSkin *skin ) -> const uint32_t * {
        return skin->vertexBoneIndices;
    };
};

struct ResourcePtrTable
{
    RwResourcesAllocateResEntry AllocateResourceEntry =
        []( void *o, RwResEntry **, int32_t s,
            RwResEntryDestroyNotify destroyNotify ) {
            RwResEntry *res_entry =
                static_cast<RwResEntry *>( malloc( sizeof( RwResEntry ) + s ) );
            res_entry->owner         = o;
            res_entry->destroyNotify = destroyNotify;
            return res_entry;
        };
    RwResourcesFreeResEntry FreeResourceEntry = []( RwResEntry *entry ) {
        entry->destroyNotify( entry );
        free( entry );
        return 1;
    };
};

class DeviceGlobals
{
  public:
    RwDevice *         DevicePtr        = nullptr;
    RwSystemFunc       fpOldSystem      = nullptr;
    RwRwDeviceGlobals *DeviceGlobalsPtr = nullptr;
    RwStandardFunc *   Standards        = nullptr;
    PluginPtrTable     PluginFuncs{};
    ResourcePtrTable   ResourceFuncs{};
    SkinPtrTable       SkinFuncs{};
};

extern DeviceGlobals gRwDeviceGlobals;

extern std::unique_ptr<RenderClient> gRenderClient;
extern std::unique_ptr<RenderDriver> gRenderDriver;

} // namespace rh::rw::engine
