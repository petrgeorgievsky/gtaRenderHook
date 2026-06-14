#pragma once
#include <Engine/Common/types/string_typedefs.h>

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
