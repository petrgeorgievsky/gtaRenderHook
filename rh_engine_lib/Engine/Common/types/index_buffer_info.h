#pragma once
namespace rh::engine {

struct IndexBufferInfo
{
    bool isDynamic;
    unsigned int indexCount;
    const void *initialData;
};
}
