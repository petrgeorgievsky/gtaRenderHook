#pragma once
struct CRwPointerTable
{
    // Main render state-machine system
    INT_PTR m_fpRenderSystem;
    // Render state set/get funcs
    INT_PTR m_fpSetRenderState;
    INT_PTR m_fpGetRenderState;
    INT_PTR m_fpSetRefreshRate;
    INT_PTR m_fpSetVideoMode;
    // Immediate 2D pointers
    INT_PTR m_fpIm2DRenderPrim;
    INT_PTR m_fpIm2DRenderIndexedPrim;
    INT_PTR m_fpIm2DRenderLine;
    // Immediate 3D pointers
    INT_PTR m_fpIm3DOpen;
    INT_PTR m_fpIm3DSubmit;
    // Default pipelines nodes and callbacks
    INT_PTR m_fpAtomicAllInOneNode;
    INT_PTR m_fpSkinAllInOneNode;
    INT_PTR m_fpDefaultInstanceCallback;
    // Renderware init/shutdown
    //INT_PTR m_fpInitRW;
    //INT_PTR m_fpShutdownRW;
    // Feature support checks
    INT_PTR m_fpCheckNativeTextureSupport;
    INT_PTR m_fpCheckEnviromentMapSupport;
    // Game specific func?
    INT_PTR m_fpInitGamma;
};

class CRwGameHooks
{
public:
    static void Patch( const CRwPointerTable& pointerTable );

    static RwBool RenderSystem( RwInt32 request, void *out, void *inOut, RwInt32 in );
    static RwBool SetRenderState( RwRenderState state, UINT param );
    static RwBool GetRenderState( RwRenderState state, UINT* param );
    static void SetRefreshRate( RwUInt32 refreshRate );
    static void SetVideoMode( RwUInt32 videomode );

    static RwBool Im2DRenderPrim( RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices );
    static RwBool Im2DRenderIndexedPrim( RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices );
    static RwBool Im2DRenderLine( RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2 );

    static RwBool Im3DSubmit();
    static void Im3DOpen();

    static RwBool CheckNativeTextureSupport();
    static RwBool CheckEnviromentMapSupport();

    static RwBool AtomicAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params );
    static RwBool SkinAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params );

    static RwBool DefaultInstanceCallback( void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance );

    //static void InitRW();
    //static void ShutdownRW();
    static void InitGamma( void *p );
    static CRwPointerTable ms_rwPointerTableSA;
};

