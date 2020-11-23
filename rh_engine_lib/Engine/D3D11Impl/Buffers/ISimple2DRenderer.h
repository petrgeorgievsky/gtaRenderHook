#pragma once
namespace rh::engine {
typedef unsigned short VertexIndex;
enum class PrimitiveType;
struct Simple2DVertex
{
    float x, y, z, w;
    float u1, v1;
    float u2, v2;
};

class ISimple2DRenderer
{
    virtual ~ISimple2DRenderer() = default;
    /*!
  Draws primitive
*/
    virtual void Draw( void *impl,
                       PrimitiveType prim,
                       Simple2DVertex *verticles,
                       unsigned int vertexCount )
        = 0;

    /*!
  Draws indexed primitive
*/
    virtual void DrawIndexed( void *impl,
                              PrimitiveType prim,
                              Simple2DVertex *vertices,
                              unsigned int numVertices,
                              VertexIndex *indices,
                              unsigned int numIndices )
        = 0;
};
} // namespace rh::engine
