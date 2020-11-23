#pragma once
#include <vector>
namespace rh::engine {
enum class ImageBufferFormat : unsigned char;
enum class ImageBufferType : unsigned char;
struct ImageBufferRawData
{
    unsigned int stride;
    const void *data;
};
/*
    Image buffer info used for GPU memory allocation
*/
struct ImageBufferInfo
{
    unsigned int width, height, depth, mipLevels;
    std::vector<ImageBufferRawData> initialDataVec;
    ImageBufferFormat format;
    ImageBufferType type;
};
} // namespace rh::engine
