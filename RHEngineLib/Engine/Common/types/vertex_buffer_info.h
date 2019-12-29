#pragma once
namespace rh::engine {
/*
    Vertex buffer info used for GPU memory allocation
*/
struct VertexBufferInfo
{
    bool isDynamic;
    unsigned int vertexSize;
    unsigned int vertexCount;
    void *initialData;
};
} // namespace rh::engine
