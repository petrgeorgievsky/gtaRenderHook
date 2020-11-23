#include "D3D11PrimitiveBatch.h"
#include "Buffers/D3D11IndexBuffer.h"
#include "Buffers/D3D11VertexBuffer.h"
using namespace rh::engine;

D3D11PrimitiveBatch::D3D11PrimitiveBatch( uint32_t batchId,
                                          void *indexBuffer,
                                          void *vertexBuffer,
                                          PrimitiveType primType,
                                          std::vector<MeshSplitData> meshSplits )
    : m_nId( batchId )
    , m_pIndexBuffer( indexBuffer )
    , m_pVertexBuffer( vertexBuffer )
    , m_primType( primType )
    , m_vMeshSplitData( std::move( meshSplits ) )
{}

D3D11PrimitiveBatch::~D3D11PrimitiveBatch()
{
    delete static_cast<D3D11IndexBuffer *>( m_pIndexBuffer );
    delete static_cast<D3D11VertexBuffer *>( m_pVertexBuffer );
}

uint32_t D3D11PrimitiveBatch::BatchId() const
{
    return m_nId;
}

PrimitiveType D3D11PrimitiveBatch::PrimType() const
{
    return m_primType;
}

void *D3D11PrimitiveBatch::IndexBuffer() const
{
    return m_pIndexBuffer;
}

void *D3D11PrimitiveBatch::VertexBuffer() const
{
    return m_pVertexBuffer;
}

std::vector<MeshSplitData> D3D11PrimitiveBatch::SplitData() const
{
    return m_vMeshSplitData;
}
