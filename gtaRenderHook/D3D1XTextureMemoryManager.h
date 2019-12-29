#pragma once
class CD3D1XBaseTexture;
/*!
    \class CD3D1XTextureMemoryManager
    \brief Texture management class

    This class manages textures lifecycle
*/
class CD3D1XTextureMemoryManager
{
public:
    // todo: replace with faster structure, pool for example
    static std::list<CD3D1XBaseTexture*> textureList;
    /*!
        Adds texture reference to texture list.
    */
    static void AddNew( CD3D1XBaseTexture* &texture );
    /*!
        Removes texture reference from texture list.
    */
    static void Remove( CD3D1XBaseTexture* &texture );
    /*
        Cleans up all texture references.
    */
    static void Shutdown();
};

