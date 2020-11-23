#pragma once
#include "string_typedefs.h"
#include <cstdint>
namespace rh::engine
{
enum ShaderStage : uint32_t;
struct ShaderInfo
{
    String      filePath, entrypoint;
    ShaderStage shaderType;
};
} // namespace rh::engine
