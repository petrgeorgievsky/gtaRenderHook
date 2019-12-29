// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "stdafx.h"
#include "CRwGameHooks.h"
#include "RwRenderEngine.h"
CRwPointerTable CRwGameHooks::ms_rwPointerTableSA = {
    0x8E249C, // RenderSystem
    // Render state set/get funcs
    0x8E24A8, // SetRenderState;
    0x8E24AC, // GetRenderState;
    0x7F8580, // SetRefreshRate;
    0x7F8640, // SetVideoMode;
    // Immediate 2D pointers
    0x8E24B8, // Im2DRenderPrim;
    0x8E24BC, // Im2DRenderIndexedPrim;
    0x8E24B0, // Im2DRenderLine;
    // Immediate 3D pointers
    0x80A225, // Im3DOpen;
    0x8E297C, // Im3DSubmit;
    // Default pipelines nodes and callbacks
    0x8D633C, // AtomicAllInOneNode;
    0x8DED0C, // SkinAllInOneNode;
    0x757899, // DefaultInstanceCallback;
    // Renderware init/shutdown
    //INT_PTR m_fpInitRW;
    //INT_PTR m_fpShutdownRW;
    // Feature support checks
    0x745530, // CheckNativeTextureSupport;
    0x5D8980, // CheckEnviromentMapSupport;
    // Game specific func?
    0x747180 // InitGamma;
};
void CRwGameHooks::Patch( const CRwPointerTable& pointerTable )
{
    SetPointer( pointerTable.m_fpRenderSystem, RenderSystem );

    SetPointer( pointerTable.m_fpIm2DRenderPrim, Im2DRenderPrim );
    SetPointer( pointerTable.m_fpIm2DRenderIndexedPrim, Im2DRenderIndexedPrim );
    SetPointer( pointerTable.m_fpIm2DRenderLine, Im2DRenderLine );

    RedirectJump( pointerTable.m_fpIm3DOpen, Im3DOpen );
    SetPointer( pointerTable.m_fpIm3DSubmit, Im3DSubmit );

    RedirectJump( pointerTable.m_fpSetRefreshRate, SetRefreshRate );
    RedirectJump( pointerTable.m_fpSetVideoMode, SetVideoMode );

    RedirectJump( pointerTable.m_fpInitGamma, InitGamma );
    RedirectJump( pointerTable.m_fpCheckEnviromentMapSupport, CheckEnviromentMapSupport );
    RedirectJump( pointerTable.m_fpCheckNativeTextureSupport, CheckNativeTextureSupport );

    SetPointer( pointerTable.m_fpSetRenderState, SetRenderState );
    SetPointer( pointerTable.m_fpGetRenderState, GetRenderState );

    SetPointer( pointerTable.m_fpAtomicAllInOneNode, AtomicAllInOneNode );
    SetPointer( pointerTable.m_fpSkinAllInOneNode, SkinAllInOneNode );

    SetPointer( pointerTable.m_fpDefaultInstanceCallback, DefaultInstanceCallback );

    //RedirectJump(pointerTable.m_fpInitRW, InitRW);
    //RedirectJump(pointerTable.m_fpShutdownRW, ShutdownRW);
}

RwBool CRwGameHooks::RenderSystem( RwInt32 request, void * out, void * inOut, RwInt32 in )
{
    return g_pRwCustomEngine->EventHandlingSystem( (RwRenderSystemState)request, (int*)out, inOut, in );
}

RwBool CRwGameHooks::SetRenderState( RwRenderState state, UINT param )
{
    return g_pRwCustomEngine->RenderStateSet( state, param );
}

RwBool CRwGameHooks::GetRenderState( RwRenderState nState, UINT* pParam )
{
    UINT rsData;
    g_pRwCustomEngine->RenderStateGet( nState, rsData );
    *pParam = rsData;
    return true;
}

void CRwGameHooks::SetRefreshRate( RwUInt32 refreshRate )
{
    UNREFERENCED_PARAMETER( refreshRate );
}

void CRwGameHooks::SetVideoMode( RwUInt32 videomode )
{
    g_pRwCustomEngine->UseMode( videomode );
}

RwBool CRwGameHooks::CheckNativeTextureSupport()
{
    // TODO: add proper check
    return true;
}

RwBool CRwGameHooks::CheckEnviromentMapSupport()
{
    // TODO: add proper check
    return true;
}

RwBool CRwGameHooks::Im2DRenderPrim( RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices )
{
    return g_pRwCustomEngine->Im2DRenderPrimitive( primType, vertices, numVertices );
}

RwBool CRwGameHooks::Im2DRenderIndexedPrim( RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices )
{
    return g_pRwCustomEngine->Im2DRenderIndexedPrimitive( primType, vertices, numVertices, indices, numIndices );
}

RwBool CRwGameHooks::Im2DRenderLine( RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2 )
{
    // TODO: add line render func
    return true;
}

void CRwGameHooks::Im3DOpen()
{
    //static_cast<CRwVulkanEngine*>(g_pRwCustomEngine)->Im3DRenderOpen();
}

void CRwGameHooks::InitGamma( void *p )
{
    UNREFERENCED_PARAMETER( p );
}

RwBool CRwGameHooks::Im3DSubmit()
{
    return g_pRwCustomEngine->Im3DSubmitNode();
}

RwBool CRwGameHooks::AtomicAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params )
{
    return g_pRwCustomEngine->AtomicAllInOneNode( self, params );
}

RwBool CRwGameHooks::SkinAllInOneNode( RxPipelineNode *self, const RxPipelineNodeParam *params )
{
    return g_pRwCustomEngine->SkinAllInOneNode( self, params );
}

RwBool CRwGameHooks::DefaultInstanceCallback( void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance )
{
    return g_pRwCustomEngine->DefaultInstanceCallback( object, resEntryHeader, reinstance );
}
