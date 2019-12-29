#pragma once
#include "DebugRenderObject.h"
#include "RwVectorMath.h"
#include "D3D1XVertexBuffer.h"
#include "D3D1XIndexBuffer.h"
#include "D3D1XVertexDeclaration.h"
class CD3D1XShader;
/*!
    \class DebugBBox
    \brief Axis-Aligned bounding box rendering object.

    This class is used to debug AABB-s in game scene.
*/
class DebugBBox :
    public DebugRenderObject
{
public:
    /*!
        Basic constructor, calculates world-space transform matrix for bounding box.
    */
    DebugBBox( RW::BBox bbox );
    /*!
        Constructor with axis transform matrix, calculates world-space transform matrix for oriented bounding box.
    */
    DebugBBox( RW::BBox bbox, RW::Matrix rotationMatrix );
    /*!
        Destructor, empty. Remove?
    */
    ~DebugBBox();
    /*!
        Renders this Bounding Box.
    */
    void Render();
    /*!
        Initializes box geometry, shaders and vertex buffers.
    */
    static void Initialize();
    /*!
        Releases allocated resources.
    */
    static void Shutdown();
private:
    RwMatrix m_WorldMatrix;
    // Base pixel shader ptr.
    static CD3D1XShader*		m_pVS;
    // Base vertex shader ptr.
    static CD3D1XShader*		m_pPS;
    static RwV3d					m_aVerticles[8];
    static USHORT					m_aIndices[24];
    static CD3D1XVertexDeclaration*		m_pVertexDecl;
    static CD3D1XVertexBuffer*			m_pVertexBuffer;
    static CD3D1XIndexBuffer*			m_pIndexBuffer;
};

