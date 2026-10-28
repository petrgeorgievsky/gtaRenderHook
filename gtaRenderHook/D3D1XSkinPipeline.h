#ifndef D3D1XSkinPipeline_h__
#define D3D1XSkinPipeline_h__
//#include "D3D1XPipeline.h"
#include "DeferredPipeline.h"
#include "D3D1XConstantBuffer.h"
#include "D3D1XVertexDeclaration.h"
#include <unordered_map>
struct PerSkinMatrixBuffer
{
    RwMatrix4x3 mSkinToWorldMatrices[64];
    RwMatrix4x3 mSkinToWorldMatricesPrev[64]; // the same atomic's bones in the previous frame (motion vectors)
};
/*! \class CD3D1XSkinPipeline
    \brief Default skinned mesh rendering pipeline.

    This class manages renderware skinned mesh draw calls.
*/
class CD3D1XSkinPipeline :
    public CDeferredPipeline
{
public:
    /*!
        Initializes pipeline shaders and skin vertex layout
    */
    CD3D1XSkinPipeline();
    /*!
        Releases pipeline resources
    */
    ~CD3D1XSkinPipeline();
    /*!
        Converts object verticies into GPU-compatible format, generates vertex layout and normals
    */
    bool Instance( void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance );
    /*!
        Renders resource entry
    */
    void Render( RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags );
private:
    /*!
        Generates normals for verticies
    */
    static void GenerateNormals( SimpleVertexSkin* verticles, unsigned int vertexCount, RpTriangle* triangles, unsigned int triangleCount );

private:
    CD3D1XConstantBuffer<PerSkinMatrixBuffer>*		m_pSkinningDataBuffer = nullptr;
    // bone matrices of each skinned atomic drawn, for the previous frame's pose
    struct BoneHistory
    {
        RwMatrix4x3  current[64];
        RwMatrix4x3  previous[64];
        unsigned int frame = 0;
    };
    std::unordered_map<RpAtomic *, BoneHistory> m_boneHistory;
    CD3D1XVertexDeclaration*						m_pVertexDeclaration = nullptr;
};

#endif