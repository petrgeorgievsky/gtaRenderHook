#pragma once
#include "D3D1XVertexBuffer.h"
/*!
    \class CD3D1XVertexBufferManager
    \brief Vertex buffer management class

    This class manages vertex buffers lifecycle
*/
class CD3D1XVertexBufferManager
{
public:
    static std::list<CD3D1XVertexBuffer*> bufferList;
    /*!
        Adds vertex buffer reference to vertex buffer list.
    */
    static void AddNew( CD3D1XVertexBuffer* &buffer );
    /*!
        Removes vertex buffer reference from vertex buffer list.
    */
    static void Remove( CD3D1XVertexBuffer* &buffer );
    /*
        Cleans up all vertex buffer references.
    */
    static void Shutdown();
};

