#pragma once
#include <cstdint>
namespace rh::engine {

/*
    This enum contains all possible image buffers supported by RH
*/
enum class InputElementType : uint8_t {
    Unknown = 0,
    Float,
    Vec2fp32,
    Vec3fp32,
    Vec4fp32,
    Vec2fp16,
    Vec4fp16,
    Vec2fp8,
    Vec4fp8
};
} // namespace rh::engine
