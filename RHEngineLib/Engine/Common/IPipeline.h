#pragma once
#include "types/shader_stage.h"
#include <string>
#include <vector>


namespace rh::engine
{
class IRenderPass;
class IShader;
struct ShaderStageDesc
{
    ShaderStage mStage;
    IShader *   mShader;
    std::string mEntryPoint;
};
struct PipelineCreateParams
{
    IRenderPass *                mRenderPass;
    std::vector<ShaderStageDesc> mShaderStages;
};
class IPipeline
{
  public:
    virtual ~IPipeline() = default;
};
} // namespace rh::engine