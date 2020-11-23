#pragma once
#include <cstdint>
namespace rh::engine {
enum class ComparisonFunc : uint8_t {
    Unknown = 0,
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always
};
}
