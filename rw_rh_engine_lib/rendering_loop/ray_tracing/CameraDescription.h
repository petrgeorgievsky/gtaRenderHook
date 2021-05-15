//
// Created by peter on 27.06.2020.
//
#pragma once
#include <render_driver/frame_renderer.h>
#include <render_driver/render_graph/RenderGraphResource.h>

namespace rh::engine
{
class IBuffer;
class IDescriptorSetLayout;
class IDescriptorSet;
class IDescriptorSetAllocator;
class IDeviceState;
} // namespace rh::engine

namespace rh::rw::engine
{
struct CameraState;
class CameraDescription : public RenderGraphResource
{
  public:
    CameraDescription( const RendererBase &renderer );
    ~CameraDescription() override;

    static uint64_t Id;
    static bool     CanCreate( const RendererBase &renderer )
    {
        return renderer.Device.GetLimits().BufferOffsetMinAlign > 0;
    }

    void Update( const FrameState &state ) override;

    rh::engine::IDescriptorSetLayout *GetSetLayout()
    {
        return mCameraSetLayout;
    }
    rh::engine::IDescriptorSet *GetDescSet() { return mCameraSet; }

  private:
    rh::engine::IDeviceState &           Device;
    rh::engine::IDescriptorSetAllocator *mDescSetAlloc;
    rh::engine::IDescriptorSetLayout *   mCameraSetLayout;
    rh::engine::IDescriptorSet *         mCameraSet;
    rh::engine::IBuffer *                mCameraBuffer;
};
} // namespace rh::rw::engine