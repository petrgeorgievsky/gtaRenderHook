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

struct LightPopulationPassParams
{
    uint32_t LightCount;
    int32_t  ReservoirSize = 32;
    uint32_t Timestamp     = 0;
    uint32_t FirstTime     = 1;
};

struct LightPopulationPassBase
{
    rh::engine::IDeviceState &Device;
    RTSceneDescription       *Scene;
    CameraDescription        *Camera;
    uint32_t                  Width;
    uint32_t                  Height;
    rh::engine::IImageView   *Normals;
    rh::engine::IBuffer      *Lights;
    rh::engine::IBuffer      *Sky;
    rh::engine::IBuffer      *TriLights;
    rh::engine::IImageView   *MotionVectors;
};

//
class LightSamplingPass : public LightPopulationPassBase
{
  public:
    explicit LightSamplingPass( LightPopulationPassBase &&base );

    void Execute( uint32_t                    light_count,
                  rh::engine::ICommandBuffer *cmd_buffer );

    rh::engine::IBuffer *GetResult();
    rh::engine::IBuffer *GetPrevResult();

    void UpdateUI();

  private:
    LightPopulationPassParams PassParams;

    // pipe stuff
    ScopedPointer<rh::engine::IDescriptorSetAllocator> DescSetAlloc;
    ScopedPointer<rh::engine::IDescriptorSetLayout>    DescSetLayout;
    ScopedPointer<rh::engine::IDescriptorSet>          DescSet;
    ScopedPointer<rh::engine::IPipelineLayout>         PipeLayout;
    ScopedPointer<rh::engine::VulkanComputePipeline>   Pipeline;
    ScopedPointer<rh::engine::IShader>                 Shader;

    // Stores resulting samples from
    ScopedPointer<rh::engine::IBuffer> ReservoirBuffer;
    ScopedPointer<rh::engine::IBuffer> PrevReservoirBuffer;

    ScopedPointer<rh::engine::IBuffer> ParamsBuffer;
};
} // namespace restir
} // namespace rh::rw::engine