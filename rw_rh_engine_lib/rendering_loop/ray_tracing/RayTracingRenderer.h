//
// Created by peter on 04.05.2020.
//

#pragma once
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/ScopedPtr.h>
#include <Engine/VulkanImpl/VulkanComputePipeline.h>
#include <Engine/VulkanImpl/VulkanImGUI.h>
#include <array>
#include <rendering_loop/frame_renderer.h>
#include <rw_engine/rh_backend/Im2DRenderer.h>
#include <rw_engine/rh_backend/im2d_backend.h>
#include <rw_engine/rh_backend/im3d_renderer.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rh_backend/skinned_mesh_backend.h>

namespace rh::engine
{
class IFrameBuffer;
class IBuffer;
struct WindowParams;
struct SwapchainFrame;
class VulkanRayTracingPipeline;
class VulkanCommandBuffer;
class VulkanTopLevelAccelerationStructure;
class VulkanBottomLevelAccelerationStructure;
} // namespace rh::engine
namespace rh::rw::engine
{
class GPUTexturePool;
class GPUModelBuffersPool;
class SkinAnimationPipeline;

constexpr auto gFramebufferCacheSize = 4;
class RayTracingTestPipe;
class RTBlasBuildPass;
class RTTlasBuildPass;
class RTSceneDescription;
class RTPrimaryRaysPass;
class CameraDescription;
class RTAOPass;
class RTShadowsPass;
class DeferredCompositionPass;
class VarAwareTempAccumFilterPipe;
class VarAwareTempAccumColorFilterPipe;
class TiledLightCulling;
class RTReflectionRaysPass;
class DebugPipeline;
class BilateralFilterPipeline;

using rh::engine::ScopedPointer;
class RayTracingRenderer : public IFrameRenderer
{
  public:
    RayTracingRenderer();
    ~RayTracingRenderer() override;

    void OnResize( const rh::engine::WindowParams &window ) override;
    std::vector<rh::engine::CommandBufferSubmitInfo>
         Render( SceneInfo *scene, rh::engine::ICommandBuffer *dest,
                 const rh::engine::SwapchainFrame &frame ) override;
    void DrawGUI();

    rh::engine::IRenderPass *GetForwardPass();
    rh::engine::VulkanImGUI *GetImGui( rh::engine::IRenderPass *pass );
    Im2DRenderer *           GetIm2DRenderer( rh::engine::IRenderPass *pass );
    Im3DRenderer *           GetIm3DRenderer( rh::engine::IRenderPass *pass );
    rh::engine::IFrameBuffer *
    GetFrameBuffer( const rh::engine::SwapchainFrame &frame,
                    rh::engine::IRenderPass *         pass );

  private:
    ScopedPointer<Im2DRenderer> im2DRendererGlobals;
    ScopedPointer<Im3DRenderer> im3DRenderer;
    rh::engine::IFrameBuffer *  mFramebufferCache[gFramebufferCacheSize]{};
    ScopedPointer<rh::engine::IRenderPass>  mForwardPass;
    ScopedPointer<rh::engine::IImageBuffer> mDepthBuffer;
    ScopedPointer<rh::engine::IImageView>   mDepthBufferView;

    ScopedPointer<VarAwareTempAccumFilterPipe> mVarTempAcummFilterPipe;
    ScopedPointer<VarAwareTempAccumColorFilterPipe>
                                           mVarTempAccumColorFilterPipe;
    ScopedPointer<BilateralFilterPipeline> mBilPipe;
    ScopedPointer<rh::engine::VulkanTopLevelAccelerationStructure> mTLAS;
    std::vector<DrawCallInfo>              mSkinDrawCallList;
    ScopedPointer<DebugPipeline>           mDebugPipeline;
    ScopedPointer<SkinAnimationPipeline>   mSkinAnimationPipe;
    ScopedPointer<RTSceneDescription>      mSceneDescription;
    ScopedPointer<CameraDescription>       mCameraDescription;
    ScopedPointer<RTBlasBuildPass>         mBlasBuildPass;
    ScopedPointer<RTTlasBuildPass>         mTlasBuildPass;
    ScopedPointer<RTPrimaryRaysPass>       mPrimaryRaysPass;
    ScopedPointer<RTAOPass>                mRTAOPass;
    ScopedPointer<RTShadowsPass>           mRTShadowsPass;
    ScopedPointer<RTReflectionRaysPass>    mRTReflectionPass;
    ScopedPointer<DeferredCompositionPass> mDeferredComposePass;
    ScopedPointer<TiledLightCulling>       mTiledLightCulling;
    ScopedPointer<rh::engine::VulkanImGUI> mImGUI;
    float                                  mCPURecordTime    = 0;
    uint64_t                               mGameViewRasterId = 0;
    uint64_t                               mBlasBuilt        = 0;
    uint32_t                               mFrameWidth       = 0;
    uint32_t                               mFrameHeight      = 0;

    bool RenderPrimaryRays( void *scene_data, void *skin_mesh_block );
    void ProcessDynamicGeometry( SceneInfo *scene );
    friend class RayTracingTestPipe;
    std::vector<rh::engine::CommandBufferSubmitInfo> mRenderDispatchList;
    std::vector<float>                               mFrameTimeGraph;
};

// TODO: Move, refactor, etc.

struct MaterialUpdDesc
{
    uint64_t tex_id;
    uint32_t color;
};

} // namespace rh::rw::engine