//
// Created by peter on 27.06.2020.
//

#pragma once
#include "RTSceneDescription.h"
#include <Engine/Common/ScopedPtr.h>
namespace rh::engine
{
class IDescriptorSetAllocator;
class IDescriptorSetLayout;
class IDescriptorSet;
class IPipelineLayout;
class IShader;
class IBuffer;
class IImageBuffer;
class IImageView;
class ISampler;
class ICommandBuffer;
class VulkanRayTracingPipeline;
} // namespace rh::engine

namespace rh::rw::engine
{
template <typename T> using SPtr = ScopedPointer<T>;

class CameraDescription;

struct PrimaryRaysConfig
{
    // Dependencies
    rh::engine::IDeviceState &Device;
    RTSceneDescription *      mScene;
    CameraDescription *       mCamera;

    uint32_t mWidth;
    uint32_t mHeight;
};
struct SkyState;
class RTPrimaryRaysPass
{
  public:
    RTPrimaryRaysPass( const PrimaryRaysConfig &config );

    void Execute( void *tlas, rh::engine::ICommandBuffer *cmd_buffer,
                  const SkyState &frame );

    rh::engine::IImageView *GetNormalsView() { return mNormalsBufferView[0]; }
    rh::engine::IImageView *GetPrevNormalsView()
    {
        return mNormalsBufferView[1];
    }
    rh::engine::IImageView *GetAlbedoView() { return mAlbedoBufferView; }
    rh::engine::IImageView *GetMotionView() { return mMotionBufferView; }
    rh::engine::IImageView *GetMaterialsView() { return mMaterialsBufferView; }
    rh::engine::IBuffer *   GetSkyCfg() { return mSkyCfg; }
    void ConvertNormalsToShaderRO( rh::engine::ICommandBuffer *dest );

  private:
    rh::engine::IDeviceState &                Device;
    RTSceneDescription *                      mScene;
    CameraDescription *                       mCamera;
    SPtr<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;
    SPtr<rh::engine::IDescriptorSetLayout>    mRayTraceSetLayout;
    SPtr<rh::engine::IDescriptorSet>          mRayTraceSet;

    SPtr<rh::engine::IPipelineLayout>          mPipeLayout;
    SPtr<rh::engine::VulkanRayTracingPipeline> mPipeline;

    SPtr<rh::engine::IShader> mRayGenShader;
    SPtr<rh::engine::IShader> mClosestHitShader;
    SPtr<rh::engine::IShader> mAnyHitShader;
    SPtr<rh::engine::IShader> mMissShader;

    SPtr<rh::engine::IBuffer> mShaderBindTable;

    SPtr<rh::engine::IBuffer> mSkyCfg;

    SPtr<rh::engine::IImageBuffer> mAlbedoBuffer;
    SPtr<rh::engine::IImageView>   mAlbedoBufferView;
    SPtr<rh::engine::IImageBuffer> mNormalsBuffer[2];
    SPtr<rh::engine::IImageView>   mNormalsBufferView[2];
    SPtr<rh::engine::IImageBuffer> mMotionBuffer;
    SPtr<rh::engine::IImageView>   mMotionBufferView;
    SPtr<rh::engine::IImageBuffer> mMaterialsBuffer;
    SPtr<rh::engine::IImageView>   mMaterialsBufferView;
    uint32_t                       mFrame = 0;
    uint32_t                       mWidth;
    uint32_t                       mHeight;

    SPtr<rh::engine::ISampler> mTextureSampler;
};
} // namespace rh::rw::engine