#pragma once
#include "color_types.h"
#include <cstdint>
namespace rh::engine {
enum class SamplerFilter : uint8_t;
enum class SamplerAddressing : uint8_t;
enum class ComparisonFunc : uint8_t;
struct Sampler
{
    SamplerFilter filtering;
    SamplerAddressing adressU, adressV, adressW;
    ComparisonFunc comparison;
    RGBA borderColor;
};
} // namespace rh::engine
