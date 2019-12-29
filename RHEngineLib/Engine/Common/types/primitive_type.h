#pragma once
#include <cstdint>
namespace rh::engine {

/*
    This enum contains all possible primitive types
*/
enum class PrimitiveType : uint8_t {
    Unknown = 0,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    PointList
};
} // namespace rh::engine
