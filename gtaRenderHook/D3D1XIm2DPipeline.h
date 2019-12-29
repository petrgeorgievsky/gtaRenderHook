#ifndef D3D1XIm2DPipeline_h__
#define D3D1XIm2DPipeline_h__
#include "D3D1XPipeline.h"
#include "D3D1XVertexDeclaration.h"
#include "D3D1XDynamicVertexBuffer.h"
#include "D3D1XDynamicIndexBuffer.h"
/*! \class CD3D1XIm2DPipeline
    \brief Immediate 2D rendering pipeline.

    This class manages 2D draw calls.
*/
class CD3D1XIm2DPipeline :public CD3D1XPipeline
{
public:
    /*!
        Initializes vertex layout, and dynamic vertex+index buffers
    */
    CD3D1XIm2DPipeline();
    /*!
        Releases allocated resources
    */
    ~CD3D1XIm2DPipeline();
    /*!
        Draws primitive
    */
    void Draw( RwPrimitiveType prim, RwIm2DVertex* verticles, RwUInt32 vertexCount );
    /*!
        Draws indexed primitive
    */
    void DrawIndexed( RwPrimitiveType prim, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwUInt32 numIndices );

private:
    CD3D1XVertexDeclaration*	m_pVertexDecl = nullptr;
    CD3D1XDynamicVertexBuffer*	m_pVertexBuffer = nullptr;
    CD3D1XDynamicIndexBuffer*	m_pIndexBuffer = nullptr;
};
#endif // D3D1XIm2DPipeline_h__

