#pragma once
#include <basetsd.h>
#include <cstdint>

#include "rw_engine/system_funcs/rw_device_system_globals.h"

namespace rh::rw::engine
{

struct RwGlobalPointerTable
{
    [[maybe_unused]] RwSystemFunc fpDefaultSystem;
    uint32_t *                    opInternalRasterExtOffset;
    // RwResourcesAllocateResEntry_FN fpResourcesAllocateResEntry;
};

struct RwPointerTable
{
    // Main render state-machine system
    INT_PTR mRwDevicePtr          = 0;
    INT_PTR mRwRwDeviceGlobalsPtr = 0;

    INT_PTR mRasterRegisterPluginPtr   = 0;
    INT_PTR mMaterialRegisterPluginPtr = 0;
    INT_PTR mCameraRegisterPluginPtr   = 0;
    INT_PTR mAllocateResourceEntry     = 0;
    INT_PTR mFreeResourceEntry         = 0;
    INT_PTR mAtomicGetHAnimHierarchy   = 0;
    INT_PTR mGeometryGetSkin           = 0;
    INT_PTR mGetSkinToBoneMatrices     = 0;
    INT_PTR mGetVertexBoneWeights      = 0;
    INT_PTR mGetVertexBoneIndices      = 0;

    INT_PTR m_fpSetRefreshRate = 0;
    INT_PTR m_fpSetVideoMode   = 0;
    // Immediate 3D pointers
    INT_PTR mIm3DOpen                   = 0;
    INT_PTR mIm3DClose                  = 0;
    INT_PTR mIm3DTransform              = 0;
    INT_PTR mIm3DRenderIndexedPrimitive = 0;
    INT_PTR mIm3DRenderPrimitive        = 0;
    INT_PTR mIm3DRenderLine             = 0;
    INT_PTR mIm3DRenderTriangle         = 0;
    INT_PTR mIm3DEnd                    = 0;

    // Default pipelines nodes and callbacks
    [[maybe_unused]] INT_PTR m_fpAtomicAllInOneNode      = 0;
    INT_PTR                  m_fpSkinAllInOneNode        = 0;
    INT_PTR                  m_fpDefaultInstanceCallback = 0;
    // Renderware init/shutdown
    // INT_PTR m_fpInitRW;
    // INT_PTR m_fpShutdownRW;
    // Feature support checks
    INT_PTR m_fpCheckNativeTextureSupport = 0;
    INT_PTR m_fpCheckEnviromentMapSupport = 0;
    // Game specific func?
    INT_PTR m_fpInitGamma = 0;
};

class RwGameHooks
{
  public:
    static void Patch( const RwPointerTable &pointerTable );

    static int32_t RenderSystem( int32_t request, void *out, void *inOut,
                                 int32_t in );
    static int32_t SetRenderState( RwRenderState nState, void *pParam );
    static int32_t GetRenderState( [[maybe_unused]] RwRenderState nState,
                                   void *                         pParam );
    static void    SetRefreshRate( uint32_t refreshRate );
    static void    SetVideoMode( uint32_t videomode );

    [[maybe_unused]] static int32_t
    Im2DRenderPrim( [[maybe_unused]] RwPrimitiveType primType,
                    [[maybe_unused]] RwIm2DVertex *  vertices,
                    [[maybe_unused]] int32_t         numVertices );
    static int32_t
    Im2DRenderIndexedPrim( [[maybe_unused]] RwPrimitiveType primType,
                           [[maybe_unused]] RwIm2DVertex *  vertices,
                           [[maybe_unused]] int32_t         numVertices,
                           [[maybe_unused]] int16_t *       indices,
                           [[maybe_unused]] int32_t         numIndices );
    static int32_t Im2DRenderLine( void *vertices, int32_t numVertices,
                                   int32_t vert1, int32_t vert2 );

    static int32_t               Im3DSubmit();
    [[maybe_unused]] static void Im3DOpen();

    static void    CheckNativeTextureSupport();
    static int32_t CheckEnviromentMapSupport();

    static int32_t AtomicAllInOneNode( void *self, const void *params );
    static int32_t SkinAllInOneNode( void *self, const void *params );

    static int32_t DefaultInstanceCallback( void *object, void *resEntryHeader,
                                            int32_t reinstance );

    // static void InitRW();
    // static void ShutdownRW();
    static void InitGamma( void *p );
};

} // namespace rh::rw::engine
