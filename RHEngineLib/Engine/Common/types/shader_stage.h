#pragma once
namespace rh::engine {
enum ShaderStage : unsigned char {
    Compute = 1,
    Domain = 2,
    Geometry = 4,
    Hull = 8,
    Pixel = 16,
    Vertex = 32,
};
}
