#pragma once
#include "string_typedefs.h"
namespace rh::engine {
enum ShaderStage : unsigned char;
struct ShaderInfo
{
    String filePath, entrypoint;
    ShaderStage shaderType;
};
} // namespace rh::engine
