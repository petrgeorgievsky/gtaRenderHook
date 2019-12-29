
#ifndef D3D1XIm3DPipeline_h__
#define D3D1XIm3DPipeline_h__
#include "D3D1XPipeline.h"
#include "D3D1XVertexDeclaration.h"
#include "D3D1XDynamicVertexBuffer.h"
#include "D3D1XDynamicIndexBuffer.h"
struct Im3DDef
{
    float x, y, z;
    RwRGBA color;
    float u, v;
};
/*! \class CD3D1XIm3DPipeline
    \brief Immediate 3D rendering pipeline.

    This class manages 3D draw calls.
*/
class CD3D1XIm3DPipeline : public CD3D1XPipeline
{
public:
    /*!
        Initializes vertex layout, and dynamic vertex+index buffers
    */
    CD3D1XIm3DPipeline();
    /*!
        Releases allocated resources
    */
    ~CD3D1XIm3DPipeline();
    /*!
        Draws 3d mesh from rw structure
    */
    RwBool SubmitNode();
private:
    CD3D1XVertexDeclaration*	m_pVertexDeclaration = nullptr;
    CD3D1XDynamicVertexBuffer*	m_pVertexBuffer = nullptr;
    CD3D1XDynamicIndexBuffer*	m_pIndexBuffer = nullptr;
};
#endif // D3D1XIm3DPipeline_h__

