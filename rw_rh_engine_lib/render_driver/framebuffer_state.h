//
// Created by peter on 16.02.2021.
//
#pragma once

#include <cstdint>
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
class IDeviceState;
struct WindowParams;
} // namespace engine

namespace rw::engine
{

struct PerFrameResources
{
    rh::engine::ISyncPrimitive *mImageAquire{};
    rh::engine::ISyncPrimitive *mRenderExecute{};
    rh::engine::ICommandBuffer *mCmdBuffer{};
    bool                        mBufferIsRecorded = false;
};

class FramebufferState
{
    static constexpr auto CacheSize = 1; // 4;
  public:
    FramebufferState( rh::engine::IDeviceState &device );
    ~FramebufferState();
    PerFrameResources &CurrentFrameResources();
    void               NextFrame();

  private:
    int32_t Id{};
    // Resource caches
    PerFrameResources ResourceCache[CacheSize];
};
} // namespace rw::engine
} // namespace rh