//
// Created by peter on 27.06.2020.
//
#pragma once

namespace rh::engine
{
class IBuffer;
class IDescriptorSetLayout;
class IDescriptorSet;
class IDescriptorSetAllocator;
} // namespace rh::engine

namespace rh::rw::engine
{
struct FrameInfo;
class CameraDescription
{
  public:
    CameraDescription();
    virtual ~CameraDescription();

    void Update( FrameInfo *frame );

    rh::engine::IDescriptorSetLayout *GetSetLayout()
    {
        return mCameraSetLayout;
    }
    rh::engine::IDescriptorSet *GetDescSet() { return mCameraSet; }

  private:
    rh::engine::IDescriptorSetAllocator *mDescSetAlloc;
    rh::engine::IDescriptorSetLayout *   mCameraSetLayout;
    rh::engine::IDescriptorSet *         mCameraSet;
    rh::engine::IBuffer *                mCameraBuffer;
};
} // namespace rh::rw::engine