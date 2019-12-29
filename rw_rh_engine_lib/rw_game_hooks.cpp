#include "rw_game_hooks.h"
#include "RwRenderEngine.h"
#include <DebugUtils/DebugLogger.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>

void rh::rw::engine::RwGameHooks::Patch( const RwPointerTable &pointerTable )
{
    if ( pointerTable.m_fpRenderSystem )
        SetPointer( pointerTable.m_fpRenderSystem, reinterpret_cast<void *>( RenderSystem ) );

    if ( pointerTable.m_fpIm3DOpen )
        RedirectJump( pointerTable.m_fpIm3DOpen, reinterpret_cast<void *>( Im3DOpen ) );

    if ( pointerTable.m_fpCheckEnviromentMapSupport )
        RedirectJump( pointerTable.m_fpCheckEnviromentMapSupport,
                      reinterpret_cast<void *>( CheckEnviromentMapSupport ) );

    if ( pointerTable.m_fpCheckNativeTextureSupport )
        RedirectJump( pointerTable.m_fpCheckNativeTextureSupport,
                      reinterpret_cast<void *>( CheckNativeTextureSupport ) );

    if ( pointerTable.m_fpSetRefreshRate )
        RedirectJump( pointerTable.m_fpSetRefreshRate, reinterpret_cast<void *>( SetRefreshRate ) );

    if ( pointerTable.m_fpSetVideoMode )
        RedirectJump( pointerTable.m_fpSetVideoMode, reinterpret_cast<void *>( SetVideoMode ) );

    if ( pointerTable.m_fpIm2DRenderPrim )
        SetPointer( pointerTable.m_fpIm2DRenderPrim, reinterpret_cast<void *>( Im2DRenderPrim ) );

    if ( pointerTable.m_fpIm2DRenderIndexedPrim )
        SetPointer( pointerTable.m_fpIm2DRenderIndexedPrim,
                    reinterpret_cast<void *>( Im2DRenderIndexedPrim ) );

    if ( pointerTable.m_fpIm2DRenderLine )
        SetPointer( pointerTable.m_fpIm2DRenderLine, reinterpret_cast<void *>( Im2DRenderLine ) );

    if ( pointerTable.m_fpSetRenderState )
        SetPointer( pointerTable.m_fpSetRenderState, reinterpret_cast<void *>( SetRenderState ) );

    if ( pointerTable.m_fpGetRenderState )
        SetPointer( pointerTable.m_fpGetRenderState, reinterpret_cast<void *>( GetRenderState ) );
}

RwBool rh::rw::engine::RwGameHooks::RenderSystem( RwInt32 request, void *out, void *inOut, RwInt32 in )
{
    return g_pRwRenderEngine->EventHandlingSystem( static_cast<RwRenderSystemRequest>( request ),
                                                   static_cast<int *>( out ),
                                                   inOut,
                                                   in );
}

RwBool rh::rw::engine::RwGameHooks::SetRenderState( RwRenderState state, UINT param )
{
    /*RHDebug::DebugLogger::Log( TEXT( "SetRenderState:" ) +
             std::to_string( static_cast<UINT>( state ) ) +
             TEXT( " value:" ) +
             std::to_string( static_cast<UINT>( param ) )
);
*/
    return g_pRwRenderEngine->RenderStateSet( state, param );
}

RwBool rh::rw::engine::RwGameHooks::GetRenderState( RwRenderState /*state*/, UINT *param )
{
    *param = 0;
    return true;
}

void rh::rw::engine::RwGameHooks::SetRefreshRate( RwUInt32 /*refreshRate*/ )
{
    debug::DebugLogger::Log( "test" );
}

void rh::rw::engine::RwGameHooks::SetVideoMode( RwUInt32 videomode )
{
    g_pRwRenderEngine->UseMode( videomode );
}

void rh::rw::engine::RwGameHooks::Im3DOpen() {}

RwBool rh::rw::engine::RwGameHooks::CheckNativeTextureSupport()
{
    return true;
}

RwBool rh::rw::engine::RwGameHooks::CheckEnviromentMapSupport()
{
    return true;
}

RwBool rh::rw::engine::RwGameHooks::Im2DRenderPrim( RwPrimitiveType primType,
                                                  RwIm2DVertex *vertices,
                                                  RwInt32 numVertices )
{
    return g_pRwRenderEngine->Im2DRenderPrimitive( primType,
                                                   vertices,
                                                   static_cast<RwUInt32>( numVertices ) );
}

RwBool rh::rw::engine::RwGameHooks::Im2DRenderIndexedPrim( RwPrimitiveType primType,
                                                         RwIm2DVertex *vertices,
                                                         RwInt32 numVertices,
                                                         RwImVertexIndex *indices,
                                                         RwInt32 numIndices )
{
    return g_pRwRenderEngine->Im2DRenderIndexedPrimitive( primType,
                                                          vertices,
                                                          static_cast<RwUInt32>( numVertices ),
                                                          indices,
                                                          numIndices );
}

RwBool rh::rw::engine::RwGameHooks::Im2DRenderLine( RwIm2DVertex * /*vertices*/,
                                                  RwInt32 /*numVertices*/,
                                                  RwInt32 /*vert1*/,
                                                  RwInt32 /*vert2*/ )
{
    return true;
}
