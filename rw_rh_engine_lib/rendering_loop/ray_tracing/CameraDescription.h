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
class IDeviceState;
} // namespace rh::engine

namespace rh::rw::engine
{
struct CameraState;
class CameraDescription
{
  public:
    CameraDescription( rh::engine::IDeviceState &device );
    virtual ~CameraDescription();

    void Update( const CameraState &state );

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