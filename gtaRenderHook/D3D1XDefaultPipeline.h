#ifndef D3D1XDefaultPipeline_h__
#define D3D1XDefaultPipeline_h__
#include "D3D1XPipeline.h"
/*! \class CD3D1XDefaultPipeline
    \brief Default mesh rendering pipeline.

    This class manages renderware mesh draw calls.
*/
class CD3D1XDefaultPipeline : public CD3D1XPipeline
{
public:
    /*!
        Initializes pipeline shaders
    */
    CD3D1XDefaultPipeline();
    /*!
        Releases pipeline resources
    */
    ~CD3D1XDefaultPipeline();
    /*!
        Converts object verticies into GPU-compatible format, generates vertex
       layout and normals
    */
    bool Instance( void *object, RxD3D9ResEntryHeader *resEntryHeader,
                   RwBool reinstance ) const;

    
    /*!
        Converts object verticies into GPU-compatible format, generates vertex layout and normals from indicies
    */
    bool Instance( void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance, const std::vector<RxVertexIndex> &indexBuffer ) const;
    /*!
        Renders resource entry
    */
    void Render( RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags );

  private:
    /*!
        Generates normals, tangents and bitangents for verticies
    */
    static void GenerateNormals( SimpleVertex *verticles,
                                 unsigned int  vertexCount,
                                 RpTriangle *  triangles,
                                 unsigned int triangleCount, bool isTriStrip );
    /*!
        Generates tangents for verticies
    */
    static void GenerateTangentsOnly( SimpleVertex *verticles,
                                      unsigned int  vertexCount,
                                      RpTriangle *  triangles,
                                      unsigned int  triangleCount,
                                      bool          isTriStrip );
    /*!
        Generates normals for verticies using indicies
    */
    static void
    GenerateNormals( RxD3D9ResEntryHeader *resEntryHeader,
                     SimpleVertex *verticles, unsigned int vertexCount,
                     const std::vector<RxVertexIndex> &indexBuffer );
};
#endif // D3D1XDefaultPipeline_h__

