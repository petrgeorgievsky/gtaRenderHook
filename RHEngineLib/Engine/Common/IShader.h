#pragma once
#include "Engine/Common/types/shader_stage.h"
#include <common.h>

namespace rh::engine
{

struct ShaderDesc
{
    // Params
    std::string mShaderPath;
    std::string mEntryPoint;
    ShaderStage mShaderStage;
};

class IShader
{
  public:
    IShader( /* args */ ) = default;
    virtual ~IShader()    = default;
};

} // namespace rh::engine
