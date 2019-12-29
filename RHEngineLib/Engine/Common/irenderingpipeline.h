#pragma once
#include <cstdint>
#include <vector>
namespace rh::engine {
class IRenderingContext;
enum class PrimitiveType : uint8_t;
struct MeshSplitData
{
    uint32_t numIndex;

    uint32_t baseIndex;

    uint32_t numVertices;

    uint32_t startIndex;

    // !TODO: REMOVE THIS
    void *material = nullptr;
};

class IPrimitiveBatch
{
public:
    virtual ~IPrimitiveBatch() = default;

    [[nodiscard]] virtual uint32_t BatchId() const = 0;
    [[nodiscard]] virtual PrimitiveType PrimType() const = 0;
    [[nodiscard]] virtual void *IndexBuffer() const = 0;
    [[nodiscard]] virtual void *VertexBuffer() const = 0;
    [[nodiscard]] virtual std::vector<MeshSplitData> SplitData() const = 0;
};
class IRenderingPipeline
{
public:
    virtual ~IRenderingPipeline() = default;
    virtual void DrawMesh( IRenderingContext *context, IPrimitiveBatch *mesh ) = 0;
};
} // namespace rh::engine
