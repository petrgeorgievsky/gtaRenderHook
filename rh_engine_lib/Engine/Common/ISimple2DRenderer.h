#pragma once

namespace rh::engine {
using VertexIndex = unsigned short;
enum class PrimitiveType : unsigned char;

struct Simple2DVertex
{
    float x, y, z, w;
    unsigned int color;
    float u, v;
};

class ISimple2DRenderer
{
public:
    virtual ~ISimple2DRenderer() = default;

    virtual void Shutdown() = 0;

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

    virtual void BindTexture( void *texture ) = 0;
};
}; // namespace rh::engine
