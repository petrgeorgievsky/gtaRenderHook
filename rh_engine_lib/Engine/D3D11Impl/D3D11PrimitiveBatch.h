#pragma once
#include "../Common/irenderingpipeline.h"
#include "D3D11Common.h"
#include "Engine/Common/types/primitive_type.h"
namespace rh::engine {

class D3D11PrimitiveBatch : public IPrimitiveBatch
{
public:
    D3D11PrimitiveBatch( uint32_t batchId,
                         void *indexBuffer,
                         void *vertexBuffer,
                         PrimitiveType primType,
                         std::vector<MeshSplitData> meshSplits );
    ~D3D11PrimitiveBatch() override;

    [[nodiscard]] uint32_t BatchId() const override;
    [[nodiscard]] PrimitiveType PrimType() const override;
    [[nodiscard]] void *IndexBuffer() const override;
    [[nodiscard]] void *VertexBuffer() const override;
    [[nodiscard]] std::vector<MeshSplitData> SplitData() const override;

private:
    uint32_t m_nId = 0;
    void *m_pIndexBuffer = nullptr;
    void *m_pVertexBuffer = nullptr;
    PrimitiveType m_primType = PrimitiveType::TriangleStrip;
    std::vector<MeshSplitData> m_vMeshSplitData;
};
} // namespace rh::engine
