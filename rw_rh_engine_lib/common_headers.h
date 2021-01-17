#pragma once

//#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include <cstdint>
#include <d3d9.h>
//#include <game_sa\RenderWare.h>

struct RwObject
{
    uint8_t type;         /**< Internal Use */
    uint8_t subType;      /**< Internal Use */
    uint8_t flags;        /**< Internal Use */
    uint8_t privateFlags; /**< Internal Use */
    void *  parent;       /**< Internal Use */
    /* Often a Frame  */
};

struct RwLLLink
{
    RwLLLink *next;
    RwLLLink *prev;
};

struct RwLinkList
{
    RwLLLink link;
};

struct RwTexDictionary
{
    RwObject   object;         /* Homogeneous type */
    RwLinkList texturesInDict; /* List of textures in dictionary */
    RwLLLink   lInInstance;    /* Link list of all dicts in system */
};
constexpr auto rwTEXTUREBASENAMELENGTH = 32;

struct RwRaster
{
    RwRaster *parent;               /* Top level raster if a sub raster */
    uint8_t * cpPixels;             /* Pixel pointer when locked */
    uint8_t * palette;              /* Raster palette */
    int32_t   width, height, depth; /* Dimensions of raster */
    int32_t   stride;               /* Lines bytes of raster */
    int16_t   nOffsetX, nOffsetY;   /* Sub raster offset */
    uint8_t   cType;                /* Type of raster */
    uint8_t   cFlags;               /* Raster flags */
    uint8_t   privateFlags;         /* Raster private flags */
    uint8_t   cFormat;              /* Raster format */

    uint8_t *originalPixels;
    int32_t  originalWidth;
    int32_t  originalHeight;
    int32_t  originalStride;
};
struct RwTexture
{
    RwRaster *       raster;        /** pointer to RwRaster with data */
    RwTexDictionary *dict;          /* Dictionary this texture is in */
    RwLLLink         lInDictionary; /* List of textures in this dictionary */

    char name[rwTEXTUREBASENAMELENGTH]; /* Name of the texture */
    char mask[rwTEXTUREBASENAMELENGTH]; /* Name of the textures mask */

    /* 31 [xxxxxxxx xxxxxxxx vvvvuuuu ffffffff] 0 */
    uint32_t filterAddressing; /* Filtering & addressing mode flags */

    int32_t refCount; /* Reference count, surprisingly enough */
};

struct RwRGBA
{
    uint8_t red;   /**< red component */
    uint8_t green; /**< green component */
    uint8_t blue;  /**< blue component */
    uint8_t alpha; /**< alpha component */
};

struct RxPipeline;
struct RwSurfaceProperties
{
    float ambient;  /**< ambient reflection coefficient */
    float specular; /**< specular reflection coefficient */
    float diffuse;  /**< reflection coefficient */
};

struct RpMaterial
{
    RwTexture *         texture;      /**< texture */
    RwRGBA              color;        /**< color */
    RxPipeline *        pipeline;     /**< pipeline */
    RwSurfaceProperties surfaceProps; /**< surfaceProps */
    int16_t             refCount; /* C.f. rwsdk/world/bageomet.h:RpGeometry */
    int16_t             pad;
};

struct RwObjectHasFrame;
using RwObjectHasFrameSyncFunction =
    RwObjectHasFrame *(*)( RwObjectHasFrame *object );

struct RwObjectHasFrame
{
    RwObject                     object;
    RwLLLink                     lFrame;
    RwObjectHasFrameSyncFunction sync;
};

struct RwV2d
{
    float x; /**< X value*/
    float y; /**< Y value */
};

struct RwV3d
{
    float x; /**< X value */
    float y; /**< Y value */
    float z; /**< Z value */
};

struct RwV4d
{
    float x; /**< X value */
    float y; /**< Y value */
    float z; /**< Z value */
    float w; /**< W value */
};

struct RwMatrixTag
{
    /* These are padded to be 16 byte quantities per line */
    RwV3d    right;
    uint32_t flags;
    RwV3d    up;
    uint32_t pad1;
    RwV3d    at;
    uint32_t pad2;
    RwV3d    pos;
    uint32_t pad3;
};

using RwMatrix = RwMatrixTag;

struct RwFrame
{
    RwObject object;

    RwLLLink inDirtyListLink;

    /* Put embedded matrices here to ensure they remain 16-byte aligned */
    RwMatrix modelling;
    RwMatrix ltm;

    RwLinkList objectList; /* List of objects connected to a frame */

    struct RwFrame *child;
    struct RwFrame *next;
    struct RwFrame *root; /* Root of the tree */
};

struct RwChunkHeaderInfo
{
    uint32_t type;     /**< chunk ID - see \ref RwStreamFindChunk */
    uint32_t length;   /**< length of the chunk data in bytes */
    uint32_t version;  /**< version of the chunk data.
                        *   See \ref RwEngineGetVersion. */
    uint32_t buildNum; /**< build number of the RenderWare libraries
                        *   previously used to stream out the data */
    int32_t isComplex; /**< Internal Use */
    /* RWPUBLICEND */
    /* WAS: "TRUE if the chunk contains sub-chunks."
     * NOTE: The "isComplex" field is only ever used by
     * the stream viewer app. Do NOT add Doxygen comments for it.
     * It should not be in RenderWare at all and ought to be deleted.
     */
    /* RWPUBLIC */
};

enum RwCameraProjection
{
    rwNACAMERAPROJECTION = 0, /**<Invalid projection */
    rwPERSPECTIVE        = 1, /**<Perspective projection */
    rwPARALLEL           = 2, /**<Parallel projection */
};

struct RwCamera;
using RwCameraBeginUpdateFunc = RwCamera *(*)( RwCamera *camera );
using RwCameraEndUpdateFunc   = RwCamera *(*)( RwCamera *camera );

struct RwPlane
{
    RwV3d normal;   /**< Normal to the plane */
    float distance; /**< Distance to plane from origin in normal direction*/
};

struct RwFrustumPlane
{
    RwPlane plane;
    uint8_t closestX;
    uint8_t closestY;
    uint8_t closestZ;
    uint8_t pad;
};

struct RwBBox
{
    /* Must be in this order */
    RwV3d sup; /**< Supremum vertex. */
    RwV3d inf; /**< Infimum vertex. */
};

struct RwCamera
{
    RwObjectHasFrame object;

    /* Parallel or perspective projection */
    RwCameraProjection projectionType;

    /* Start/end update functions */
    RwCameraBeginUpdateFunc beginUpdate;
    RwCameraEndUpdateFunc   endUpdate;

    /* The view matrix */
    RwMatrix viewMatrix;

    /* The cameras image buffer */
    RwRaster *frameBuffer;

    /* The Z buffer */
    RwRaster *zBuffer;

    /* Cameras mathmatical characteristics */
    RwV2d viewWindow;
    RwV2d recipViewWindow;
    RwV2d viewOffset;
    float nearPlane;
    float farPlane;
    float fogPlane;

    /* Transformation to turn camera z or 1/z into a Z buffer z */
    float zScale, zShift;

    /* The clip-planes making up the viewing frustum */
    RwFrustumPlane frustumPlanes[6];
    RwBBox         frustumBoundBox;

    /* Points on the tips of the view frustum */
    RwV3d frustumCorners[8];
};

enum RwRasterType : uint32_t
{
    rwRASTERTYPENORMAL        = 0x00, /**<Normal */
    rwRASTERTYPEZBUFFER       = 0x01, /**<Z Buffer */
    rwRASTERTYPECAMERA        = 0x02, /**<Camera */
    rwRASTERTYPETEXTURE       = 0x04, /**<Texture */
    rwRASTERTYPECAMERATEXTURE = 0x05, /**<Camera texture */
    rwRASTERTYPEMASK          = 0x07, /**<Mask for finding type */

    rwRASTERPALETTEVOLATILE =
        0x40, /**<If set, hints that the palette will change often */
    rwRASTERDONTALLOCATE = 0x80 /**<If set the raster is not allocated */
};

enum RwRasterFormat : uint32_t
{
    rwRASTERFORMATDEFAULT = 0x0000, /* Whatever the hardware likes best */

    rwRASTERFORMAT1555 =
        0x0100, /**<16 bits - 1 bit alpha, 5 bits red, green and blue */
    rwRASTERFORMAT565 =
        0x0200, /**<16 bits - 5 bits red and blue, 6 bits green */
    rwRASTERFORMAT4444 = 0x0300, /**<16 bits - 4 bits per component */
    rwRASTERFORMATLUM8 = 0x0400, /**<Gray scale */
    rwRASTERFORMAT8888 = 0x0500, /**<32 bits - 8 bits per component */
    rwRASTERFORMAT888  = 0x0600, /**<24 bits - 8 bits per component */
    rwRASTERFORMAT16 =
        0x0700, /**<16 bits - undefined: useful for things like Z buffers */
    rwRASTERFORMAT24 =
        0x0800, /**<24 bits - undefined: useful for things like Z buffers */
    rwRASTERFORMAT32 =
        0x0900, /**<32 bits - undefined: useful for things like Z buffers */
    rwRASTERFORMAT555 = 0x0a00, /**<16 bits - 5 bits red, green and blue */

    rwRASTERFORMATAUTOMIPMAP =
        0x1000, /**<RenderWare generated the mip levels */

    rwRASTERFORMATPAL8 = 0x2000, /**<8 bit palettised */
    rwRASTERFORMATPAL4 = 0x4000, /**<4 bit palettised */

    rwRASTERFORMATMIPMAP = 0x8000, /**<Mip mapping on */

    rwRASTERFORMATPIXELFORMATMASK = 0x0f00, /**<The pixel color format
                                             *  (excluding palettised bits) */
    rwRASTERFORMATMASK = 0xff00             /**<The whole format */
};

enum RwRasterLockMode : uint32_t
{
    rwRASTERLOCKWRITE   = 0x01, /**<Lock for writing */
    rwRASTERLOCKREAD    = 0x02, /**<Lock for reading */
    rwRASTERLOCKNOFETCH = 0x04,
    rwRASTERLOCKRAW     = 0x08, /**<When used in combination with
                                    rwRASTERLOCKWRITE or rwRASTERLOCKREAD
                                    allows access to the raw platform specific
                                    pixel format */
};

enum RwPluginVendor : uint32_t
{
    rwVENDORID_CORE           = 0x000000L,
    rwVENDORID_CRITERIONTK    = 0x000001L,
    rwVENDORID_REDLINERACER   = 0x000002L,
    rwVENDORID_CSLRD          = 0x000003L,
    rwVENDORID_CRITERIONINT   = 0x000004L,
    rwVENDORID_CRITERIONWORLD = 0x000005L,
    rwVENDORID_BETA           = 0x000006L,
    rwVENDORID_CRITERIONRM    = 0x000007L,
    rwVENDORID_CRITERIONRWA   = 0x000008L, /* RenderWare Audio */
    rwVENDORID_CRITERIONRWP   = 0x000009L  /* RenderWare Physics */
};

#define MAKECHUNKID( vendorID, chunkID )                                       \
    ( ( ( (vendorID)&0xFFFFFFu ) << 8u ) | ( (chunkID)&0xFFu ) )

enum RwCorePluginID : uint32_t
{
    rwID_NAOBJECT  = MAKECHUNKID( rwVENDORID_CORE, 0x00u ),
    rwID_STRUCT    = MAKECHUNKID( rwVENDORID_CORE, 0x01u ),
    rwID_STRING    = MAKECHUNKID( rwVENDORID_CORE, 0x02u ),
    rwID_EXTENSION = MAKECHUNKID( rwVENDORID_CORE, 0x03u ),

    rwID_CAMERA = MAKECHUNKID( rwVENDORID_CORE, 0x05u ),
    /**< RwCamera chunk. See \ref RwCameraStreamRead */

    rwID_TEXTURE = MAKECHUNKID( rwVENDORID_CORE, 0x06u ),
    /**< RwTexture chunk. See \ref RwTextureStreamRead */

    rwID_MATERIAL = MAKECHUNKID( rwVENDORID_CORE, 0x07u ),
    /**< RpMaterial chunk. See \ref RpMaterialStreamRead. */

    rwID_MATLIST    = MAKECHUNKID( rwVENDORID_CORE, 0x08u ),
    rwID_ATOMICSECT = MAKECHUNKID( rwVENDORID_CORE, 0x09u ),
    rwID_PLANESECT  = MAKECHUNKID( rwVENDORID_CORE, 0x0Au ),

    rwID_WORLD = MAKECHUNKID( rwVENDORID_CORE, 0x0Bu ),
    /**< RpWorld chunk. See \ref RpWorldStreamRead. */

    rwID_SPLINE = MAKECHUNKID( rwVENDORID_CORE, 0x0Cu ),
    /**< RpSpline chunk. See \ref RpSplineStreamRead */

    rwID_MATRIX = MAKECHUNKID( rwVENDORID_CORE, 0x0Du ),
    /**< RwMatrix chunk. See \ref RwMatrixStreamRead */

    rwID_FRAMELIST = MAKECHUNKID( rwVENDORID_CORE, 0x0Eu ),

    rwID_GEOMETRY = MAKECHUNKID( rwVENDORID_CORE, 0x0Fu ),
    /**< RpGeometry chunk. See \ref RpGeometryStreamRead. */

    rwID_CLUMP = MAKECHUNKID( rwVENDORID_CORE, 0x10u ),
    /**< RpClump chunk. See \ref RpClumpStreamRead. */

    rwID_LIGHT = MAKECHUNKID( rwVENDORID_CORE, 0x12u ),
    /**< RpLight chunk. See \ref RpLightStreamRead. */

    rwID_UNICODESTRING = MAKECHUNKID( rwVENDORID_CORE, 0x13u ),

    rwID_ATOMIC = MAKECHUNKID( rwVENDORID_CORE, 0x14u ),
    /**< RpAtomic chunk. See \ref RpAtomicStreamRead */

    rwID_TEXTURENATIVE = MAKECHUNKID( rwVENDORID_CORE, 0x15u ),

    rwID_TEXDICTIONARY = MAKECHUNKID( rwVENDORID_CORE, 0x16u ),
    /**< RwTexDictionary - platform specific texture dictionary.
     * See \ref RwTexDictionaryStreamRead. */

    rwID_ANIMDATABASE = MAKECHUNKID( rwVENDORID_CORE, 0x17u ),

    rwID_IMAGE = MAKECHUNKID( rwVENDORID_CORE, 0x18u ),
    /**< RwImage chunk. See \ref RwImageStreamRead */

    rwID_SKINANIMATION = MAKECHUNKID( rwVENDORID_CORE, 0x19u ),
    rwID_GEOMETRYLIST  = MAKECHUNKID( rwVENDORID_CORE, 0x1Au ),

    rwID_ANIMANIMATION = MAKECHUNKID( rwVENDORID_CORE, 0x1Bu ),
    /**< RtAnimAnimation chunk. See \ref RtAnimAnimationStreamRead. */
    rwID_HANIMANIMATION = MAKECHUNKID( rwVENDORID_CORE, 0x1Bu ),
    /**< RtAnimAnimation chunk. For backwards compatibility. See \ref
       rwID_ANIMANIMATION. */

    rwID_TEAM = MAKECHUNKID( rwVENDORID_CORE, 0x1Cu ),
    /**< \ref RpTeam chunk. See \ref RpTeamStreamRead */
    rwID_CROWD = MAKECHUNKID( rwVENDORID_CORE, 0x1Du ),

    rwID_DMORPHANIMATION = MAKECHUNKID( rwVENDORID_CORE, 0x1Eu ),
    /**< RpDMorphAnimation - delta morph animation chunk. See \ref
       RpDMorphAnimationStreamRead */

    rwID_RIGHTTORENDER = MAKECHUNKID( rwVENDORID_CORE, 0x1fu ),

    rwID_MTEFFECTNATIVE = MAKECHUNKID( rwVENDORID_CORE, 0x20u ),
    /**< \if xbox RpMTEffect - multi-texture effect chunk. See \ref
       RpMTEffectStreamRead \endif */
    /**< \if gcn  RpMTEffect - multi-texture effect chunk. See \ref
       RpMTEffectStreamRead \endif */

    rwID_MTEFFECTDICT = MAKECHUNKID( rwVENDORID_CORE, 0x21u ),
    /**< \if xbox RpMTEffectDict - multi-texture effect dictionary chunk. See
       \ref RpMTEffectDictStreamRead \endif */
    /**< \if gcn  RpMTEffectDict - multi-texture effect dictionary chunk. See
       \ref RpMTEffectDictStreamRead \endif */

    rwID_TEAMDICTIONARY = MAKECHUNKID( rwVENDORID_CORE, 0x22u ),
    /**< \ref RpTeamDictionary chunk. See \ref RpTeamDictionaryStreamRead */

    rwID_PITEXDICTIONARY = MAKECHUNKID( rwVENDORID_CORE, 0x23u ),
    /**< RwTexDictionary - platform independent texture dictionary. See \ref
       RtPITexDictionaryStreamRead. */

    rwID_TOC = MAKECHUNKID( rwVENDORID_CORE, 0x24u ),
    /**< RtTOC chunk. See \ref RtTOCStreamRead */

    rwID_PRTSTDGLOBALDATA = MAKECHUNKID( rwVENDORID_CORE, 0x25u ),
    /**< RpPrtStdEmitterClass, RpPrtStdParticleClass and RpPrtStdPropertyTable
     * chunks. See \ref RpPrtStdEClassStreamRead, \ref RpPrtStdPClassStreamRead
     *   \ref RpPrtStdPropTabStreamRead and \ref RpPrtStdGlobalDataStreamRead */

    rwID_ALTPIPE   = MAKECHUNKID( rwVENDORID_CORE, 0x26u ),
    rwID_PIPEDS    = MAKECHUNKID( rwVENDORID_CORE, 0x27u ),
    rwID_PATCHMESH = MAKECHUNKID( rwVENDORID_CORE, 0x28u ),
    /**< RpPatchMesh chunk. See \ref RpPatchMeshStreamRead */

    rwID_CHUNKGROUPSTART = MAKECHUNKID( rwVENDORID_CORE, 0x29u ),
    rwID_CHUNKGROUPEND   = MAKECHUNKID( rwVENDORID_CORE, 0x2Au ),

    rwID_UVANIMDICT = MAKECHUNKID( rwVENDORID_CORE, 0x2Bu ),
    /**< UV anim dictionary chunk. See \ref RpUVAnimGetDictSchema */

    rwID_COLLTREE = MAKECHUNKID( rwVENDORID_CORE, 0x2Cu ),

    rwID_ENVIRONMENT = MAKECHUNKID( rwVENDORID_CORE, 0x2Du ),
    /**< RpEnvironment chunk is used internally to convey art package background
   < settings to the Visualizer.*/

    /* Insert before MAX and increment MAX */
    rwID_COREPLUGINIDMAX = MAKECHUNKID( rwVENDORID_CORE, 0x2Eu )
};

enum RwCriterionCoreID : uint32_t
{
    /* Guard value that should not be used. */
    rwID_NACOREID = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x00u ),

    /* The valid plugin IDs */
    /** Vector module pluginID. See \ref rwv2d and \ref rwv3d */
    rwID_VECTORMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x01u ),
    /** Matrix module pluginID. See \ref rwmatrix */
    rwID_MATRIXMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x02u ),
    /** Frame module pluginID. See \ref rwframe */
    rwID_FRAMEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x03u ),
    /** Stream module pluginID. See \ref rwstream */
    rwID_STREAMMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x04u ),
    /** Camera module pluginID. See \ref rwcamera */
    rwID_CAMERAMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x05u ),
    /** Image module pluginID. See \ref rwimage */
    rwID_IMAGEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x06u ),
    /** Raster module pluginID. See \ref rwraster */
    rwID_RASTERMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x07u ),
    /** Texture module pluginID. See \ref rwtexture */
    rwID_TEXTUREMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x08u ),
    /** Pipeline module pluginID. See \ref RxPipeline */
    rwID_PIPEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x09u ),
    /** Immediate module pluginID. See \ref rwim3d */
    rwID_IMMEDIATEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x0Au ),
    /** Resource module pluginID. See \ref rwresources */
    rwID_RESOURCESMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x0Bu ),
    /** Device module pluginID */
    rwID_DEVICEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x0Cu ),
    /** Color module pluginID. See \ref rwrgba */
    rwID_COLORMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x0Du ),
    /* Not used */
    rwID_POLYPIPEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x0Eu ),
    /** Error module pluginID. See \ref rwerror */
    rwID_ERRORMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x0Fu ),
    /** Metrics module pluginID. See \ref RwMetrics */
    rwID_METRICSMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x10u ),
    /** Driver module pluginID */
    rwID_DRIVERMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x11u ),
    /** Chunk group module pluginID. See \ref rwchunkgroup */
    rwID_CHUNKGROUPMODULE = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x12u ),
    //
    rwID_CAMERA_BACKEND = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x13u ),
    //
    rwID_RASTER_BACKEND = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x14u ),
    //
    rwID_MATERIAL_BACKEND = MAKECHUNKID( rwVENDORID_CRITERIONINT, 0x15u )
};

enum RwPlatformID
{
    rwID_PCD3D7 = 1,
    rwID_PCOGL,
    rwID_MAC,
    rwID_PS2,
    rwID_XBOX,
    rwID_GAMECUBE,
    rwID_SOFTRAS,
    rwID_PCD3D8,
    rwID_PCD3D9,
    rwID_PSP
};

enum RwTextureAddressMode
{
    rwTEXTUREADDRESSNATEXTUREADDRESS = 0,
    rwTEXTUREADDRESSWRAP,   /**<UV wraps (tiles) */
    rwTEXTUREADDRESSMIRROR, /**<Alternate UV is flipped */
    rwTEXTUREADDRESSCLAMP,  /**<UV is clamped to 0-1 */
    rwTEXTUREADDRESSBORDER, /**<Border color takes effect outside of 0-1 */
};

enum RwBlendFunction
{
    rwBLENDNABLEND = 0,
    rwBLENDZERO,         /**<(0,    0,    0,    0   ) */
    rwBLENDONE,          /**<(1,    1,    1,    1   ) */
    rwBLENDSRCCOLOR,     /**<(Rs,   Gs,   Bs,   As  ) */
    rwBLENDINVSRCCOLOR,  /**<(1-Rs, 1-Gs, 1-Bs, 1-As) */
    rwBLENDSRCALPHA,     /**<(As,   As,   As,   As  ) */
    rwBLENDINVSRCALPHA,  /**<(1-As, 1-As, 1-As, 1-As) */
    rwBLENDDESTALPHA,    /**<(Ad,   Ad,   Ad,   Ad  ) */
    rwBLENDINVDESTALPHA, /**<(1-Ad, 1-Ad, 1-Ad, 1-Ad) */
    rwBLENDDESTCOLOR,    /**<(Rd,   Gd,   Bd,   Ad  ) */
    rwBLENDINVDESTCOLOR, /**<(1-Rd, 1-Gd, 1-Bd, 1-Ad) */
    rwBLENDSRCALPHASAT,  /**<(f,    f,    f,    1   )  f = min (As, 1-Ad) */
};

enum RwCullMode
{
    rwCULLMODENACULLMODE = 0,

    rwCULLMODECULLNONE,
    /**<Both front and back-facing triangles are drawn. */
    rwCULLMODECULLBACK,
    /**<Only front-facing triangles are drawn */
    rwCULLMODECULLFRONT,
    /**<Only back-facing triangles are drawn */

};

struct RwIm2DVertex
{
    float x;   /**< Screen X */
    float y;   /**< Screen Y */
    float z;   /**< Screen Z */
    float rhw; /**< Reciprocal of homogeneous W */

    uint32_t emissiveColor; /**< Vertex color */

    float u; /**< Texture coordinate U */
    float v; /**< Texture coordinate V */
};

struct RwIm3DVertex
{
    RwV3d    objVertex; /**< position */
    RwV3d    objNormal; /**< normal */
    uint32_t color;     /**< emissive color*/
    float    u;         /**< u */
    float    v;         /**< v */
};

// TODO: MOVE from here
enum RwPrimitiveType
{
    rwPRIMTYPENAPRIMTYPE = 0, /**<Invalid primative type */
    rwPRIMTYPELINELIST =
        1, /**<Unconnected line segments, each line is specified by
            * both its start and end index, independently of other lines
            * (for example, 3 segments specified as 0-1, 2-3, 4-5) */
    rwPRIMTYPEPOLYLINE = 2, /**<Connected line segments, each line's start index
                             * (except the first) is specified by the index of
                             * the end of the previous segment (for example, 3
                             * segments specified as 0-1, 1-2, 2-3) */
    rwPRIMTYPETRILIST =
        3, /**<Unconnected triangles: each triangle is specified by
            * three indices, independently of other triangles (for example,
            * 3 triangles specified as 0-1-2, 3-4-5, 6-7-8) */
    rwPRIMTYPETRISTRIP = 4, /**<Connected triangles sharing an edge with, at
                             * most, one other forming a series (for example, 3
                             * triangles specified as 0-2-1, 1-2-3-, 2-4-3) */
    rwPRIMTYPETRIFAN = 5, /**<Connected triangles sharing an edge with, at most,
                           * two others forming a fan (for example, 3 triangles
                           * specified as 0-2-1, 0-3-2, 0-4-3) */
    rwPRIMTYPEPOINTLIST = 6, /**<Points 1, 2, 3, etc. This is not
                              * supported by the default RenderWare
                              * immediate or retained-mode pipelines
                              * (except on PlayStation 2), it is intended
                              * for use by user-created pipelines */
};

struct RwSubSystemInfo
{
    char name[80]; /**< Sub system string */
};

enum RwCameraClearMode
{
    rwCAMERACLEARIMAGE   = 0x1, /**<Clear the frame buffer */
    rwCAMERACLEARZ       = 0x2, /**<Clear the Z buffer */
    rwCAMERACLEARSTENCIL = 0x4, /**<\if xbox   Clear the stencil buffer \endif
                                 * \if d3d8   Clear the stencil buffer \endif
                                 * \if d3d9   Clear the stencil buffer \endif
                                 * \if opengl Clear the stencil buffer \endif
                                 */
};

enum RwTextureFilterMode
{
    rwFILTERNAFILTERMODE = 0,
    rwFILTERNEAREST,          /**<Point sampled */
    rwFILTERLINEAR,           /**<Bilinear */
    rwFILTERMIPNEAREST,       /**<Point sampled per pixel mip map */
    rwFILTERMIPLINEAR,        /**<Bilinear per pixel mipmap */
    rwFILTERLINEARMIPNEAREST, /**<MipMap interp point sampled */
    rwFILTERLINEARMIPLINEAR,  /**<Trilinear */
};
enum RwRenderState : int32_t
{
    rwRENDERSTATENARENDERSTATE = 0,

    rwRENDERSTATETEXTURERASTER,
    /**<Raster used for texturing (normally used in immediate mode).
     *  The value is a pointer to an \ref RwRaster.
     * Default: NULL.
     */
    rwRENDERSTATETEXTUREADDRESS,
    /**<\ref RwTextureAddressMode: wrap, clamp, mirror or border.
     * Default: rwTEXTUREADDRESSWRAP.
     */
    rwRENDERSTATETEXTUREADDRESSU,
    /**<\ref RwTextureAddressMode in u only.
     * Default: rwTEXTUREADDRESSWRAP.
     */
    rwRENDERSTATETEXTUREADDRESSV,
    /**<\ref RwTextureAddressMode in v only.
     * Default: rwTEXTUREADDRESSWRAP.
     */
    rwRENDERSTATETEXTUREPERSPECTIVE,
    /**<Perspective correction on/off (always enabled on many platforms).
     */
    rwRENDERSTATEZTESTENABLE,
    /**<Z-buffer test on/off.
     * Default: TRUE.
     */
    rwRENDERSTATESHADEMODE,
    /**<\ref RwShadeMode: flat or gouraud shading.
     * Default: rwSHADEMODEGOURAUD.
     */
    rwRENDERSTATEZWRITEENABLE,
    /**<Z-buffer write on/off.
     * Default: TRUE.
     */
    rwRENDERSTATETEXTUREFILTER,
    /**<\ref RwTextureFilterMode: point sample, bilinear, trilinear, etc.
     * Default: rwFILTERLINEAR.
     */
    rwRENDERSTATESRCBLEND,
    /**<\ref RwBlendFunction used to modulate the source pixel color
     *  when blending to the frame buffer.
     * Default: rwBLENDSRCALPHA.
     */
    rwRENDERSTATEDESTBLEND,
    /**<\ref RwBlendFunction used to modulate the destination pixel
     *  color in the frame buffer when blending. The resulting pixel
     *  color is given by the formula
     *  (SRCBLEND * srcColor + DESTBLEND * destColor) for each RGB
     *  component. For a particular platform, not all combinations
     *  of blend function are allowed (see platform specific
     *  restrictions).
     * Default: rwBLENDINVSRCALPHA.
     */
    rwRENDERSTATEVERTEXALPHAENABLE,
    /**<Alpha blending on/off (always enabled on some platforms).
     *  This is normally used in immediate mode to enable alpha blending
     *  when vertex colors or texture rasters have transparency. Retained
     *  mode pipelines will usually set this state based on material colors
     *  and textures.
     * Default: FALSE.
     */
    rwRENDERSTATEBORDERCOLOR,
    /**<Border color for \ref RwTextureAddressMode
     *  \ref rwTEXTUREADDRESSBORDER. The value should be a packed
     *  RwUInt32 in a platform specific format. The macro
     *  RWRGBALONG(r, g, b, a) may be used to construct this using
     *  8-bit color components.
     * Default: RWRGBALONG(0, 0, 0, 0).
     */
    rwRENDERSTATEFOGENABLE,
    /**<Fogging on/off (all polygons will be fogged).
     * Default: FALSE.
     */
    rwRENDERSTATEFOGCOLOR,
    /**<Color used for fogging. The value should be a packed RwUInt32
     *  in a platform specific format. The macro RWRGBALONG(r, g, b, a)
     *  may be used to construct this using 8-bit color components.
     * Default: RWRGBALONG(0, 0, 0, 0).
     */
    rwRENDERSTATEFOGTYPE,
    /**<\ref RwFogType, the type of fogging to use.
     * Default: rwFOGTYPELINEAR.
     */
    rwRENDERSTATEFOGDENSITY,
    /**<Fog density for \ref RwFogType of
     *  \ref rwFOGTYPEEXPONENTIAL or \ref rwFOGTYPEEXPONENTIAL2.
     *  The value should be a pointer to an RwReal in the
     *  range 0 to 1.
     * Default: 1.
     */
    rwRENDERSTATECULLMODE = 20,
    /**<\ref RwCullMode, for selecting front/back face culling, or
     *  no culling.
     * Default: rwCULLMODECULLBACK.
     */
    rwRENDERSTATESTENCILENABLE,
    /**<Stenciling on/off.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: FALSE.
     */
    rwRENDERSTATESTENCILFAIL,
    /**<\ref RwStencilOperation used when the stencil test passes.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: rwSTENCILOPERATIONKEEP.
     */
    rwRENDERSTATESTENCILZFAIL,
    /**<\ref RwStencilOperation used when the stencil test passes and
     *  the depth test (z-test) fails.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: rwSTENCILOPERATIONKEEP.
     */
    rwRENDERSTATESTENCILPASS,
    /**<\ref RwStencilOperation used when both the stencil and the depth
     *  (z) tests pass.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: rwSTENCILOPERATIONKEEP.
     */
    rwRENDERSTATESTENCILFUNCTION,
    /**<\ref RwStencilFunction for the stencil test.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: rwSTENCILFUNCTIONALWAYS.
     */
    rwRENDERSTATESTENCILFUNCTIONREF,
    /**<Integer reference value for the stencil test.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: 0.
     */
    rwRENDERSTATESTENCILFUNCTIONMASK,
    /**<Mask applied to the reference value and each stencil buffer
     *  entry to determine the significant bits for the stencil test.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: 0xffffffff.
     */
    rwRENDERSTATESTENCILFUNCTIONWRITEMASK,
    /**<Write mask applied to values written into the stencil buffer.
     *  <i> Supported on Xbox, D3D8, D3D9, and OpenGL only. </i>
     * Default: 0xffffffff.
     */
    rwRENDERSTATEALPHATESTFUNCTION,
    /**<\ref RwAlphaTestFunction for the alpha test. When a pixel fails,
     * neither the frame buffer nor the Z-buffer are updated.
     * Default: rwALPHATESTFUNCTIONGREATER (GameCube, Xbox, D3D8, D3D9
     * and OpenGL). The default PS2 behaviour is to always update the
     * frame buffer and update the Z-buffer only if a greater than or
     * equal test passes.
     */
    rwRENDERSTATEALPHATESTFUNCTIONREF,
    /**<Integer reference value for the alpha test.
     *  <i> Range is 0 to 255, mapped to the platform's actual range </i>
     * Default: 128 (PS2) 0 (GameCube, Xbox, D3D8, D3D9 and OpenGL).
     */

};

struct RpInterpolator
{
    int32_t flags;            /**< flags */
    int16_t startMorphTarget; /**< startMorphTarget */
    int16_t endMorphTarget;   /**< endMorphTarget */
    float   time;             /**< time */
    float   recipTime;        /**< recipTime */
    float   position;         /**< position */
};

struct RwResEntry;

using RwResEntryDestroyNotify = void ( * )( RwResEntry *resEntry );

struct RwResEntry
{
    RwLLLink     link;     /* Node in the list of resource elements */
    int32_t      size;     /* Size of this node */
    void *       owner;    /* Owner of this node */
    RwResEntry **ownerRef; /* Pointer to pointer to this (enables de-alloc) */
    RwResEntryDestroyNotify
        destroyNotify; /* This is called right before destruction */
};

struct RpMaterialList
{
    RpMaterial **materials;
    int32_t      numMaterials;
    int32_t      space;
};

struct RpTriangle
{
    uint16_t vertIndex[3]; /**< vertex indices */
    uint16_t matIndex;     /**< Index into material list */
};

struct RwTexCoords
{
    float u; /**< U value */
    float v; /**< V value */
};

#define rwMAXTEXTURECOORDS 8

struct RpMeshHeader
{
    uint32_t flags;              /**< \see RpMeshHeaderFlags */
    uint16_t numMeshes;          /**< Number of meshes in object */
    uint16_t serialNum;          /**< Determine if mesh has changed
                                  * since last instance */
    uint32_t totalIndicesInMesh; /**< Total triangle index
                                  * count in all meshes
                                  */
    uint32_t firstMeshOffset;    /**< Offset in bytes from end this
                                  * structure RpMeshHeader
                                  * to the first mesh
                                  */
};
struct RpGeometry;
struct RwSphere
{
    RwV3d center; /**< Sphere center */
    float radius; /**< Sphere radius */
};
struct RpMorphTarget
{
    RpGeometry *parentGeom;
    RwSphere    boundingSphere;
    RwV3d *     verts;
    RwV3d *     normals;
};
struct RpGeometry
{
    RwObject object; /* Generic type */

    uint32_t flags; /* Geometry flags */

    uint16_t lockedSinceLastInst; /* What has been locked since we last
                                     instanced - for re-instancing */
    int16_t refCount; /* Reference count (for keeping track of atomics
                         referencing geometry) */

    int32_t numTriangles; /* Quantity of various things (polys, verts and morph
                             targets) */
    int32_t numVertices;
    int32_t numMorphTargets;
    int32_t numTexCoordSets;

    RpMaterialList matList;

    RpTriangle *triangles; /* The triangles */

    RwRGBA *preLitLum; /* The pre-lighting values */

    RwTexCoords *texCoords[rwMAXTEXTURECOORDS]; /* Texture coordinates */

    RpMeshHeader *mesh; /* The mesh - groups polys of the same material */

    RwResEntry *repEntry; /* Information for an instance */

    RpMorphTarget *morphTarget; /* The Morph Target */
};

struct RpClump;
typedef RpClump *( *RpClumpCallBack )( RpClump *clump, void *data );

struct RpClump
{
    RwObject object;

    /* Information about all the Atomics */
    RwLinkList atomicList;

    /* Lists of lights and cameras */
    RwLinkList lightList;
    RwLinkList cameraList;

    /* The clump in a world */
    RwLLLink inWorldLink;

    /* Clump frustum callback */
    RpClumpCallBack callback;
};
struct RpAtomic;
typedef RpAtomic *( *RpAtomicCallBackRender )( RpAtomic *atomic );

struct RpAtomic
{
    RwObjectHasFrame object;

    /* Information for an instance */
    RwResEntry *repEntry;

    /* Triangles making the object */
    RpGeometry *geometry;

    /* Interpolated bounding sphere (in object space and world space) */
    RwSphere boundingSphere;
    RwSphere worldBoundingSphere;

    /* Connections to other atomics */
    RpClump *clump;
    RwLLLink inClumpLink;

    /* callbacks */
    RpAtomicCallBackRender renderCallBack;

    /* Interpolation animation pointer */
    RpInterpolator interpolator;

    /* Counter for checks of "render has occurred already" */
    uint16_t renderFrame;
    uint16_t pad;

    /* Connections to sectors */
    RwLinkList llWorldSectorsInAtomic;

    /* The Atomic object pipeline for this Atomic */
    RxPipeline *pipeline;
};

struct rwFrameList
{
    RwFrame **frames;
    int32_t   numFrames;
};

enum RwCriterionWorldID
{
    /* Guard value that should not be used. */
    rwID_NAWORLDID = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x00u ),

    /* The valid plugin IDs */
    /**< RpMaterial pluginID */
    rwID_MATERIALMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x01u ),
    /**< RpMesh pluginID */
    rwID_MESHMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x02u ),
    /**< RpGeometry pluginID */
    rwID_GEOMETRYMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x03u ),
    /**< RpClump pluginID */
    rwID_CLUMPMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x04u ),
    /**< RpLight pluginID */
    rwID_LIGHTMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x05u ),
    /* Not used */
    rwID_COLLISIONMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x06u ),
    /**< RpWorld pluginID */
    rwID_WORLDMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x07u ),
    /* Not used */
    rwID_RANDOMMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x08u ),
    /**< PluginID for RpWorld's objects */
    rwID_WORLDOBJMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x09u ),
    /**< RpWorldSector pluginID */
    rwID_SECTORMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x0Au ),
    /**< Binary RpWorld pluginID */
    rwID_BINWORLDMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x0Bu ),
    /**< RpWorld pipeline pluginID */
    rwID_WORLDPIPEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x0Du ),
    /**< Binary RpMesh pluginID */
    rwID_BINMESHPLUGIN = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x0Eu ),
    /**< RpWorld device pluginID */
    rwID_RXWORLDDEVICEMODULE = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x0Fu ),
    /**< PluginID for platform specific serialization data */
    rwID_NATIVEDATAPLUGIN = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x10u ),
    /**< \if xbox Vertex format pluginID \endif */
    /**< \if gcn  Vertex format pluginID \endif */
    rwID_VERTEXFMTPLUGIN = MAKECHUNKID( rwVENDORID_CRITERIONWORLD, 0x11u )
};
enum RpGeometryFlag : uint32_t
{
    rpGEOMETRYTRISTRIP = 0x00000001,  /**<This geometry's meshes can be
                                          rendered as strips.
                                          \ref RpMeshSetTriStripMethod is
                                          used to change this method.*/
    rpGEOMETRYPOSITIONS = 0x00000002, /**<This geometry has positions */
    rpGEOMETRYTEXTURED  = 0x00000004, /**<This geometry has only one set of
                                          texture coordinates. Texture
                                          coordinates are specified on a per
                                          vertex basis */
    rpGEOMETRYPRELIT  = 0x00000008,   /**<This geometry has pre-light colors */
    rpGEOMETRYNORMALS = 0x00000010,   /**<This geometry has vertex normals */
    rpGEOMETRYLIGHT   = 0x00000020,   /**<This geometry will be lit */
    rpGEOMETRYMODULATEMATERIALCOLOR = 0x00000040, /**<Modulate material color
                                                      with vertex colors
                                                      (pre-lit + lit) */

    rpGEOMETRYTEXTURED2 = 0x00000080, /**<This geometry has at least 2 sets of
                                          texture coordinates. */

    /*
     * These above flags were stored in the flags field in an RwObject, they
     * are now stored in the flags file of the RpGeometry.
     */

    rpGEOMETRYNATIVE         = 0x01000000,
    rpGEOMETRYNATIVEINSTANCE = 0x02000000,

    rpGEOMETRYFLAGSMASK       = 0x000000FF,
    rpGEOMETRYNATIVEFLAGSMASK = 0x0F000000
};

struct RpMesh
{
    uint16_t *  indices;    /**< vertex indices defining the mesh */
    uint32_t    numIndices; /**< number of vertices in mesh */
    RpMaterial *material;   /**< pointer to material used to
                             *   render the mesh. */
};

enum RpMeshHeaderFlags : uint32_t
{
    /* NOTE: trilists are denoted by absence of any other
     *       primtype flags, so be careful that you test:
     *        (triListFlag == (flags&triListFlag))
     *       or:
     *        (0 == (flags&rpMESHHEADERPRIMMASK))
     *       and not:
     *        (flags&triListFlag)
     */
    rpMESHHEADERTRISTRIP = 0x0001u, /**< Render as tristrips */
    rpMESHHEADERTRIFAN =
        0x0002u, /**< On PlayStation 2 these will be converted to trilists */
    rpMESHHEADERLINELIST = 0x0004u, /**< Render as linelists */
    rpMESHHEADERPOLYLINE =
        0x0008u, /**< On PlayStation 2 these will be converted to linelists */
    rpMESHHEADERPOINTLIST =
        0x0010u, /**< Pointlists are supported only if rendered by
                  *   custom pipelines; there is no default RenderWare
                  *   way to render pointlists. */

    rpMESHHEADERPRIMMASK =
        0x00FFu, /**< All bits reserved for specifying primitive type */
    rpMESHHEADERUNINDEXED =
        0x0100u, /**< Topology is defined implicitly by vertex
                  *   order, ergo the mesh contains no indices */
};

struct RwImage
{
    int32_t flags;

    int32_t width;  /* Device may have different ideas */
    int32_t height; /* internally !! */

    int32_t depth; /* Of referenced image */
    int32_t stride;

    uint8_t *cpPixels;
    RwRGBA * palette;
};

typedef void *( *RpWorldSectorCallBackRender )( void *worldSector );

struct RpWorld
{
    RwObject object;

    uint32_t flags;

    uint32_t renderOrder;

    /* Materials */
    RpMaterialList matList;

    /* The world stored as a BSP tree */
    void *rootSector; // RpSector

    /* The number of texture coordinate sets in each sector */
    int32_t numTexCoordSets;

    /* Render frame used when last rendered */
    int32_t   numClumpsInWorld;
    RwLLLink *currentClumpLink;

    /* All the clumps in the world */
    RwLinkList clumpList;

    /* All of the lights in the world */
    RwLinkList lightList;

    /* Directional lights in the world */
    RwLinkList directionalLightList;

    /* The worlds origin offset */
    RwV3d worldOrigin;

    /* Bounding box around the whole world */
    RwBBox boundingBox;

    /* The callbacks functions */
    RpWorldSectorCallBackRender renderCallBack;

    RxPipeline *pipeline;
};
struct RwMatrixWeights
{
    float w0; /**< The first matrix weight.  */
    float w1; /**< The second matrix weight. */
    float w2; /**< The third matrix weight.  */
    float w3; /**< The fourth matrix weight. */
};
struct RpSkin
{
    uint32_t         numBones;
    uint32_t         numBoneIds;
    uint8_t *        boneIds;
    RwMatrix *       skinToBoneMatrices;
    uint32_t         maxNumWeightsForVertex;
    uint32_t *       vertexBoneIndices;
    RwMatrixWeights *vertexBoneWeights;
    uint32_t         boneCount;
    uint32_t         useVS;
    uint32_t         boneLimit;
    uint32_t         numMeshes;
    uint32_t         numRLE;
    uint8_t *        meshBoneRemapIndices;
    uint32_t         meshBoneRLECount;
    void *           meshBoneRLE;
    void *           field_3C;
};

struct RpHAnimNodeInfo
{
    int32_t  nodeID;    /**< User defined ID for this node  */
    int32_t  nodeIndex; /**< Array index of node  */
    int32_t  flags;     /**< Matrix push/pop flags  */
    RwFrame *pFrame;    /**< Pointer to an attached RwFrame (see \ref
                           RpHAnimHierarchyAttach) */
};

struct RpHAnimHierarchy
{
    int32_t flags;    /**< Flags for the hierarchy  */
    int32_t numNodes; /**< Number of nodes in the hierarchy  */

    RwMatrix *pMatrixArray;      /**< Pointer to node matrices*/
    void *pMatrixArrayUnaligned; /**< Pointer to memory used for node matrices
                                  * from which the aligned pMatrixArray is
                                  * allocated */
    RpHAnimNodeInfo
        *    pNodeInfo;   /**< Array of node information (push/pop flags etc) */
    RwFrame *parentFrame; /**< Pointer to the Root RwFrame of the hierarchy this
                           * RpHAnimHierarchy represents */
    RpHAnimHierarchy *parentHierarchy;  /**< Internal use */
    int32_t           rootParentOffset; /**< Internal use */

    void *currentAnim; /**< Internal use RtAnimInterpolator */
};

struct RxPipelineNodeParam
{
    void *                 dataParam;
    [[maybe_unused]] void *heap;
};