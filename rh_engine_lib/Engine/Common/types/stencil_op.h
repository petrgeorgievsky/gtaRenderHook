#pragma once
namespace rh::engine {
enum class StencilOp : unsigned char {
    Unknown = 0,
    Keep = 1,
    Zero = 2,
    Replace = 3,
    IncrSat = 4,
    DecrSat = 5,
    Invert = 6,
    Incr = 7,
    Decr = 8
};
}
