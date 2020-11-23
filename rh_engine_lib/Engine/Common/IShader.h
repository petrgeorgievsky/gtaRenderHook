#pragma once
#include "Engine/Common/types/shader_stage.h"
#include <string>

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
