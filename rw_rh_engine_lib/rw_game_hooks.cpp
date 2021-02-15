#include "rw_game_hooks.h"
#include "render_loop.h"
#include "rw_engine/rh_backend/im2d_backend.h"
#include <DebugUtils/DebugLogger.h>
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <render_client/render_client.h>
#include <rw_engine/rw_api_injectors.h>
#include <rw_engine/rw_rh_convert_funcs.h>

namespace rh::rw::engine
{
void *_rwIm3DOpen( void *instance, [[maybe_unused]] int32_t offset,
                   [[maybe_unused]] int32_t size )
{
    return instance;
}

void *_rwIm3DClose( void *instance, [[maybe_unused]] int32_t offset,
                    [[maybe_unused]] int32_t size )
{
    return instance;
}

void RedirectJumpIfExists( INT_PTR address, void *func )
{
    if ( address == 0 )
        return;
    RedirectJump( address, func );
}

void RwGameHooks::Patch( const RwPointerTable &pointerTable )
{
    if ( pointerTable.mRwDevicePtr )
    {
        gRwDeviceGlobals.DevicePtr =
            reinterpret_cast<RwDevice *>( pointerTable.mRwDevicePtr );
        gRwDeviceGlobals.fpOldSystem = gRwDeviceGlobals.DevicePtr->fpSystem;
        gRwDeviceGlobals.DevicePtr->fpSystem = SystemHandler;
        gRwDeviceGlobals.DevicePtr->fpIm2DRenderPrimitive =
            Im2DRenderPrimitiveFunction;
        gRwDeviceGlobals.DevicePtr->fpIm2DRenderIndexedPrimitive =
            Im2DRenderIndexedPrimitiveFunction;
        gRwDeviceGlobals.DevicePtr->fpRenderStateSet = SetRenderState;
        gRwDeviceGlobals.DevicePtr->fpRenderStateGet = GetRenderState;

        /* Render functions */
        gRwDeviceGlobals.DevicePtr->fpIm2DRenderLine =
            []( RwIm2DVertex *vertices, int32_t numVertices, int32_t vert1,
                int32_t vert2 ) { return 1; };
        gRwDeviceGlobals.DevicePtr->fpIm2DRenderTriangle =
            []( RwIm2DVertex *vertices, int32_t numVertices, int32_t vert1,
                int32_t vert2, int32_t vert3 ) { return 1; };

        gRwDeviceGlobals.DevicePtr->fpIm3DRenderLine             = nullptr;
        gRwDeviceGlobals.DevicePtr->fpIm3DRenderTriangle         = nullptr;
        gRwDeviceGlobals.DevicePtr->fpIm3DRenderPrimitive        = nullptr;
        gRwDeviceGlobals.DevicePtr->fpIm3DRenderIndexedPrimitive = nullptr;
    }
    if ( pointerTable.mAllocateResourceEntry )
        gRwDeviceGlobals.ResourceFuncs.AllocateResourceEntry =
            reinterpret_cast<RwResourcesAllocateResEntry>(
                pointerTable.mAllocateResourceEntry );
    if ( pointerTable.mFreeResourceEntry )
        gRwDeviceGlobals.ResourceFuncs.FreeResourceEntry =
            reinterpret_cast<RwResourcesFreeResEntry>(
                pointerTable.mFreeResourceEntry );

    if ( pointerTable.mRasterRegisterPluginPtr )
        gRwDeviceGlobals.PluginFuncs.RasterRegisterPlugin =
            reinterpret_cast<RegisterPluginCall>(
                pointerTable.mRasterRegisterPluginPtr );

    if ( pointerTable.mMaterialRegisterPluginPtr )
        gRwDeviceGlobals.PluginFuncs.MaterialRegisterPlugin =
            reinterpret_cast<RegisterPluginCall>(
                pointerTable.mMaterialRegisterPluginPtr );

    if ( pointerTable.mMaterialRegisterPluginPtr )
        gRwDeviceGlobals.PluginFuncs.MaterialSetStreamAlwaysCallBack =
            reinterpret_cast<SetStreamAlwaysCallBack>(
                pointerTable.mMaterialSetStreamAlwaysCallbackPtr );

    if ( pointerTable.mCameraRegisterPluginPtr )
        gRwDeviceGlobals.PluginFuncs.CameraRegisterPlugin =
            reinterpret_cast<RegisterPluginCall>(
                pointerTable.mCameraRegisterPluginPtr );

    if ( pointerTable.mAtomicGetHAnimHierarchy )
        gRwDeviceGlobals.SkinFuncs.AtomicGetHAnimHierarchy =
            reinterpret_cast<RpSkinAtomicGetHAnimHierarchyFP>(
                pointerTable.mAtomicGetHAnimHierarchy );
    if ( pointerTable.mGeometryGetSkin )
        gRwDeviceGlobals.SkinFuncs.GeometryGetSkin =
            reinterpret_cast<RpSkinGeometryGetSkinFP>(
                pointerTable.mGeometryGetSkin );
    if ( pointerTable.mGetSkinToBoneMatrices )
        gRwDeviceGlobals.SkinFuncs.GetSkinToBoneMatrices =
            reinterpret_cast<RpSkinGetSkinToBoneMatricesFP>(
                pointerTable.mGetSkinToBoneMatrices );
    if ( pointerTable.mGetVertexBoneIndices )
        gRwDeviceGlobals.SkinFuncs.GetVertexBoneIndices =
            reinterpret_cast<RpSkinGetVertexBoneIndicesFP>(
                pointerTable.mGetVertexBoneIndices );
    if ( pointerTable.mGetVertexBoneWeights )
        gRwDeviceGlobals.SkinFuncs.GetVertexBoneWeights =
            reinterpret_cast<RpSkinGetVertexBoneWeightsFP>(
                pointerTable.mGetVertexBoneWeights );

    if ( pointerTable.mRwRwDeviceGlobalsPtr )
    {
        gRwDeviceGlobals.DeviceGlobalsPtr =
            reinterpret_cast<RwRwDeviceGlobals *>(
                pointerTable.mRwRwDeviceGlobalsPtr );
    }

    if ( pointerTable.mIm3DOpen )
        SetPointer( pointerTable.mIm3DOpen,
                    reinterpret_cast<void *>( _rwIm3DOpen ) );
    if ( pointerTable.mIm3DClose )
        SetPointer( pointerTable.mIm3DClose,
                    reinterpret_cast<void *>( _rwIm3DClose ) );

    RedirectJumpIfExists(
        pointerTable.m_fpCheckEnviromentMapSupport,
        reinterpret_cast<void *>( CheckEnviromentMapSupport ) );

    if ( pointerTable.m_fpCheckNativeTextureSupport )
        RedirectCall( pointerTable.m_fpCheckNativeTextureSupport,
                      reinterpret_cast<void *>( CheckNativeTextureSupport ) );

    RedirectJumpIfExists( pointerTable.m_fpSetRefreshRate,
                          reinterpret_cast<void *>( SetRefreshRate ) );

    RedirectJumpIfExists( pointerTable.m_fpSetVideoMode,
                          reinterpret_cast<void *>( SetVideoMode ) );

    RedirectJumpIfExists( pointerTable.mIm3DTransform,
                          reinterpret_cast<void *>( Im3DTransform ) );
    RedirectJumpIfExists(
        pointerTable.mIm3DRenderIndexedPrimitive,
        reinterpret_cast<void *>( Im3DRenderIndexedPrimitive ) );
    RedirectJumpIfExists( pointerTable.mIm3DRenderPrimitive,
                          reinterpret_cast<void *>( Im3DRenderPrimitive ) );
    RedirectJumpIfExists( pointerTable.mIm3DRenderLine,
                          reinterpret_cast<void *>( Im3DRenderLine ) );
    // Im3DEnd
    RedirectJumpIfExists( pointerTable.mIm3DEnd,
                          reinterpret_cast<void *>( Im3DEnd ) );
}

int32_t RwGameHooks::SetRenderState( RwRenderState nState, void *pParam )
{
    /* debug::DebugLogger::Log(
         "RWGAMEHOOKS_LOG: SetRenderState:" + std::to_string( nState ) +
         " value:" + std::to_string( (uint32_t)pParam ) );*/
    if ( nState == RwRenderState::rwRENDERSTATETEXTURERASTER )
    {
        Im2DSetRaster( static_cast<RwRaster *>( pParam ) );
    }
    else if ( nState == RwRenderState::rwRENDERSTATESRCBLEND )
    {
        auto rh_blend_op = static_cast<uint8_t>(
            RwBlendFunctionToRHBlendOp( static_cast<RwBlendFunction>(
                reinterpret_cast<uint32_t>( pParam ) ) ) );
        EngineClient::gIm2DGlobals.SetBlendSrc( rh_blend_op );
        EngineClient::gIm3DGlobals.SetBlendSrc( rh_blend_op );
    }
    else if ( nState == RwRenderState::rwRENDERSTATEDESTBLEND )
    {
        auto rh_blend_op = static_cast<uint8_t>(
            RwBlendFunctionToRHBlendOp( static_cast<RwBlendFunction>(
                reinterpret_cast<uint32_t>( pParam ) ) ) );
        EngineClient::gIm2DGlobals.SetBlendDest( rh_blend_op );
        EngineClient::gIm3DGlobals.SetBlendDest( rh_blend_op );
    }
    else if ( nState == RwRenderState::rwRENDERSTATEVERTEXALPHAENABLE )
    {
        EngineClient::gIm2DGlobals.SetBlendEnable(
            reinterpret_cast<uint32_t>( pParam ) != 0 );
        EngineClient::gIm3DGlobals.SetBlendEnable(
            reinterpret_cast<uint32_t>( pParam ) != 0 );
    }
    else if ( nState == RwRenderState::rwRENDERSTATEZTESTENABLE )
    {
        EngineClient::gIm2DGlobals.SetDepthEnable(
            reinterpret_cast<uint32_t>( pParam ) != 0 );
        EngineClient::gIm3DGlobals.SetDepthEnable(
            reinterpret_cast<uint32_t>( pParam ) != 0 );
    }
    else if ( nState == RwRenderState::rwRENDERSTATEZWRITEENABLE )
    {
        EngineClient::gIm2DGlobals.SetDepthWriteEnable(
            reinterpret_cast<uint32_t>( pParam ) != 0 );
        EngineClient::gIm3DGlobals.SetDepthWriteEnable(
            reinterpret_cast<uint32_t>( pParam ) != 0 );
    }
    return 1;
}

int32_t RwGameHooks::GetRenderState( [[maybe_unused]] RwRenderState nState,
                                     void *                         pParam )
{
    /* debug::DebugLogger::Log( "RWGAMEHOOKS_LOG: GetRenderState:" +
                              std::to_string( nState ) );*/
    *static_cast<uint32_t *>( pParam ) = 0;
    return true;
}

void RwGameHooks::SetRefreshRate( uint32_t /*refreshRate*/ )
{
    debug::DebugLogger::Log( "RWGAMEHOOKS_LOG: SetRefreshRate" );
}

void RwGameHooks::SetVideoMode( uint32_t )
{
    debug::DebugLogger::Log( "RWGAMEHOOKS_LOG: SetVideoMode" );
    // g_pRwRenderEngine->UseMode( videomode );
}

[[maybe_unused]] void RwGameHooks::Im3DOpen() {}

void RwGameHooks::CheckNativeTextureSupport() {}

int32_t RwGameHooks::CheckEnviromentMapSupport() { return true; }

[[maybe_unused]] int32_t
RwGameHooks::Im2DRenderPrim( [[maybe_unused]] RwPrimitiveType primType,
                             [[maybe_unused]] RwIm2DVertex *  vertices,
                             [[maybe_unused]] int32_t         numVertices )
{
    debug::DebugLogger::Log( "RWGAMEHOOKS_LOG: Im2DRenderPrim" );
    return 1;
}

int32_t
RwGameHooks::Im2DRenderIndexedPrim( [[maybe_unused]] RwPrimitiveType primType,
                                    [[maybe_unused]] RwIm2DVertex *  vertices,
                                    [[maybe_unused]] int32_t  numVertices,
                                    [[maybe_unused]] int16_t *indices,
                                    [[maybe_unused]] int32_t  numIndices )
{
    debug::DebugLogger::Log( "RWGAMEHOOKS_LOG: Im2DRenderIndexedPrim" );
    return 1;
}

int32_t RwGameHooks::Im2DRenderLine( void *, int32_t, int32_t, int32_t )
{
    return true;
}
} // namespace rh::rw::engine
