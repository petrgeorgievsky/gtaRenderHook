//
// Created by peter on 04.08.2021.
//
#pragma once
#include <Engine/Common/ScopedPtr.h>
#include <vector>
namespace rh::engine
{
class VulkanComputePipeline;
class IDescriptorSetAllocator;
class IPipelineLayout;
class IDescriptorSetLayout;
class IDescriptorSet;
class IBuffer;
class IShader;
class ICommandBuffer;
class IImageView;
class IImageBuffer;
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

struct SpatialReusePassParams
{
    int32_t  NeighbourCount = 5;
    float    SpatialRadius  = 20.0f;
    uint32_t ScreenWidth;
    uint32_t ScreenHeight;
    uint32_t Timestamp     = 0;
    uint32_t LightsCount   = 0;
    uint32_t TriLightCount = 0;
    uint32_t padd;
};

struct SpatialReusePassBase
{
    rh::engine::IDeviceState &Device;
    uint32_t                  mWidth;
    uint32_t                  mHeight;
    rh::engine::IBuffer      *mReservoirBuffer;
    rh::engine::IImageView   *mNormalsView;
    rh::engine::IBuffer      *mLightBuffer;
    rh::engine::IBuffer      *mSkyCfg;
    CameraDescription        *mCamera;
    RTSceneDescription       *Scene; //
    rh::engine::IBuffer      *TempReservoirBuffer;
    rh::engine::IBuffer      *TriLights;
};

//
class SpatialReusePass : public SpatialReusePassBase
{
  public:
    explicit SpatialReusePass( SpatialReusePassBase &&base );

    void Execute( uint32_t                    light_count,
                  rh::engine::ICommandBuffer *cmd_buffer );

    rh::engine::IBuffer *GetResult();

    void UpdateUI();

  private:
    SpatialReusePassParams mPassParams{};

    // pipe stuff
    ScopedPointer<rh::engine::IDescriptorSetAllocator> mDescSetAlloc;
    ScopedPointer<rh::engine::IDescriptorSetLayout>    mDescSetLayout;
    ScopedPointer<rh::engine::IDescriptorSet>          mDescSet;
    ScopedPointer<rh::engine::IPipelineLayout>         mPipeLayout;
    ScopedPointer<rh::engine::VulkanComputePipeline>   mPipeline;
    ScopedPointer<rh::engine::IShader>                 mShader;

    ScopedPointer<rh::engine::IBuffer> mParamsBuffer;
};
} // namespace restir
} // namespace rh::rw::engine