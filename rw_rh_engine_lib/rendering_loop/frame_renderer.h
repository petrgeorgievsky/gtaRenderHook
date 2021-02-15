//
// Created by peter on 04.05.2020.
//
#pragma once

#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IWindow.h>
#include <scene_graph.h>

namespace rh::engine
{
class ICommandBuffer;
class IImageView;
struct SwapchainFrame;
} // namespace rh::engine
namespace rh::rw::engine
{

struct SceneInfo
{
    FrameInfo *mFrameInfo;
    void *     mIm2DRenderBlock;
    void *     mIm3DRenderBlock;
    void *     mSkinMeshRenderBlock;
    void *     mSceneMeshRenderBlock;
};

class IFrameRenderer
{
  public:
    virtual ~IFrameRenderer()                                       = default;
    virtual void OnResize( const rh::engine::WindowParams &window ) = 0;
    virtual std::vector<rh::engine::CommandBufferSubmitInfo>
    Render( const SceneInfo *scene, rh::engine::ICommandBuffer *dest,
            const rh::engine::SwapchainFrame &frame ) = 0;
};

} // namespace rh::rw::engine