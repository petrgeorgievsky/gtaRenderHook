#include "RwGameHooks.h"
#include "Engine/RwRenderEngine.h"
#include "DebugUtils/DebugLogger.h"
#include "MemoryInjectionUtils/InjectorHelpers.h"

void rh::engine::RwGameHooks::Patch( const RwPointerTable & pointerTable )
{
    if( pointerTable.m_fpRenderSystem )
        SetPointer( pointerTable.m_fpRenderSystem, RenderSystem );

    if( pointerTable.m_fpIm3DOpen )
        RedirectJump( pointerTable.m_fpIm3DOpen, Im3DOpen );

    if( pointerTable.m_fpCheckEnviromentMapSupport )
        RedirectJump( pointerTable.m_fpCheckEnviromentMapSupport, CheckEnviromentMapSupport );

    if( pointerTable.m_fpCheckNativeTextureSupport )
        RedirectJump( pointerTable.m_fpCheckNativeTextureSupport, CheckNativeTextureSupport );

    if( pointerTable.m_fpIm2DRenderPrim )
        SetPointer( pointerTable.m_fpIm2DRenderPrim, Im2DRenderPrim );

    if( pointerTable.m_fpIm2DRenderIndexedPrim )
        SetPointer( pointerTable.m_fpIm2DRenderIndexedPrim, Im2DRenderIndexedPrim );

    if( pointerTable.m_fpIm2DRenderLine )
        SetPointer( pointerTable.m_fpIm2DRenderLine, Im2DRenderLine );

    if( pointerTable.m_fpSetRenderState )
        SetPointer( pointerTable.m_fpSetRenderState, SetRenderState );

    if( pointerTable.m_fpGetRenderState )
        SetPointer( pointerTable.m_fpGetRenderState, GetRenderState );
}

RwBool rh::engine::RwGameHooks::RenderSystem( RwInt32 request, void * out, void * inOut, RwInt32 in )
{
    return g_pRWRenderEngine->EventHandlingSystem( static_cast<RwRenderSystemRequest> ( request ), (int*)out, inOut, in );
}

RwBool rh::engine::RwGameHooks::SetRenderState( RwRenderState state, UINT param )
{
    /*RHDebug::DebugLogger::Log( TEXT( "SetRenderState:" ) + 
                               std::to_string( static_cast<UINT>( state ) ) +
                               TEXT( " value:" ) +
                               std::to_string( static_cast<UINT>( param ) )
    );
    */
    return g_pRWRenderEngine->RenderStateSet( state, param );
}

RwBool rh::engine::RwGameHooks::GetRenderState( RwRenderState state, UINT * param )
{
    *param = 0;
    return true;
}

void rh::engine::RwGameHooks::Im3DOpen()
{
}

RwBool rh::engine::RwGameHooks::CheckNativeTextureSupport()
{
    return true;
}

RwBool rh::engine::RwGameHooks::CheckEnviromentMapSupport()
{
    return true;
}

RwBool rh::engine::RwGameHooks::Im2DRenderPrim( RwPrimitiveType primType, RwIm2DVertex * vertices, RwInt32 numVertices )
{
    return g_pRWRenderEngine->Im2DRenderPrimitive( primType, vertices, numVertices );
}

RwBool rh::engine::RwGameHooks::Im2DRenderIndexedPrim( RwPrimitiveType primType, RwIm2DVertex * vertices, RwInt32 numVertices, RwImVertexIndex * indices, RwInt32 numIndices )
{
    return true;
}

RwBool rh::engine::RwGameHooks::Im2DRenderLine( RwIm2DVertex * vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2 )
{
    return true;
}
