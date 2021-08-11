//
// Created by peter on 04.08.2021.
//
#pragma once
#include <Engine/Common/ScopedPtr.h>

#include <array>
#include <cstdint>
namespace rh::engine
{
class IImageView;
class IShader;
class IPipelineLayout;
class VulkanRayTracingPipeline;
class VulkanComputePipeline;
class IBuffer;
class IImageBuffer;
class ISampler;
class ICommandBuffer;
class IDescriptorSetAllocator;
class IDescriptorSet;
class IDescriptorSetLayout;
class IDeviceState;
} // namespace rh::engine

namespace rh::rw::engine
{
using rh::engine::ScopedPointer;
class RTSceneDescription;
class CameraDescription;
class VarAwareTempAccumColorFilterPipe;
class VATAColorFilterPass;
class BilateralFilterPipeline;
class BilateralFilterPass;

namespace restir
{
class LightSamplingPass;
class SpatialReusePass;

struct VisibilityReuseInitParams
{
    rh::engine::IDeviceState &Device;
    uint32_t                  Width;
    uint32_t                  Height;
    RTSceneDescription       *Scene;
    CameraDescription        *Camera;
    rh::engine::IBuffer      *SkyCfg;
    rh::engine::IImageView   *NormalsView;
    rh::engine::IBuffer      *LightBuffer;
    rh::engine::IBuffer      *InputReservoir;
    rh::engine::IBuffer      *OutputReservoir;
};

struct VisibilityReuseProperties
{
    uint32_t Timestamp   = 0;
    uint32_t LightsCount = 0;
    float    LightRadius = 0.3f;
    float    padd        = 0;
};

class VisibilityReusePass
{
  public:
    void Execute( void *tlas, VisibilityReuseProperties properties,
                  rh::engine::ICommandBuffer *cmd_buffer );

    VisibilityReusePass( const VisibilityReuseInitParams &params );

  private:
    rh::engine::IDeviceState &Device;
    RTSceneDescription       *mScene;
    CameraDescription        *mCamera;

    ScopedPointer<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;
    ScopedPointer<rh::engine::IDescriptorSetLayout>    mRayTraceSetLayout;
    ScopedPointer<rh::engine::IDescriptorSet>          mRayTraceSet;

    ScopedPointer<rh::engine::IPipelineLayout>          mPipeLayout;
    ScopedPointer<rh::engine::VulkanRayTracingPipeline> mPipeline;

    ScopedPointer<rh::engine::IShader> mRayGenShader;
    ScopedPointer<rh::engine::IShader> mClosestHitShader{};
    ScopedPointer<rh::engine::IShader> mAnyHitShader;
    ScopedPointer<rh::engine::IShader> mMissShader;

    ScopedPointer<rh::engine::IBuffer> mShaderBindTable;

    ScopedPointer<rh::engine::ISampler> mTextureSampler;
    uint32_t                            mWidth;
    uint32_t                            mHeight;

    ScopedPointer<rh::engine::IBuffer> mParamsBuffer;
};
} // namespace restir
} // namespace rh::rw::engine