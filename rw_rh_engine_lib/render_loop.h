
#include "common_headers.h"
#include <cstdint>
#include <rendering_loop/frame_renderer.h>
#include <rw_engine/rh_backend/im2d_backend.h>
#include <rw_engine/rh_backend/im3d_backend.h>
#include <rw_engine/rh_backend/mesh_rendering_backend.h>
#include <rw_engine/rh_backend/skinned_mesh_backend.h>
#include <string>

struct RwCamera;

namespace rh
{
namespace engine
{
class IWindow;
class ISyncPrimitive;
class ICommandBuffer;
class IFrameBuffer;
class IRenderPass;
class ISwapchain;
class IImageView;
struct WindowParams;
} // namespace engine

namespace rw::engine
{

constexpr auto gFrameResourceCacheSize = 1; // 4;

struct PerFrameResources
{
    rh::engine::ISyncPrimitive *mImageAquire{};
    rh::engine::ISyncPrimitive *mRenderExecute{};
    rh::engine::ICommandBuffer *mCmdBuffer{};
    bool                        mBufferIsRecorded = false;
};

class CameraState
{
  public:
    CameraState( rh::engine::IDeviceState &device );
    ~CameraState();
    PerFrameResources &CurrentFrameResources();
    void               NextFrame();

  private:
    // Resource caches
    PerFrameResources mFrameResourceCache[gFrameResourceCacheSize];
    int32_t           mFrameId{};
};

class EngineClient
{
  public:
    static Im2DClientGlobals     gIm2DGlobals;
    static Im3DClient            gIm3DGlobals;
    static BackendRendererClient gRendererGlobals;
    static SkinRendererClient    gSkinRendererGlobals;
};

class EngineState
{
  public:
    // Rendering stuff
    static std::unique_ptr<CameraState>    gCameraState;
    static std::shared_ptr<IFrameRenderer> gFrameRenderer;
};

void InitRenderEvents();

void ExecuteRender();

} // namespace rw::engine
} // namespace rh