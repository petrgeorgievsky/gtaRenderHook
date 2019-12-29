#pragma once
#include "DebugRenderObject.h"
// Debug information rendering class.
class CD3D1XShader;
/*!
    \class DebugRendering
    \brief Debug objects rendering utility.

    This class is used to render all debug information for RenderHook.
*/
class DebugRendering
{
public:
    /*!
        Initializes resources.
    */
    static void Init();
    /*!
        Releases resources.
    */
    static void Shutdown();
    /*!
        Adds debug object to debug rendering list.
    */
    static void AddToRenderList( DebugRenderObject* );
    /*!
        Removes debug object from debug rendering list.
    */
    static void RemoveFromRenderList( DebugRenderObject* );
    /*!
        Cleans up debug rendering list
    */
    static void ResetList();
    /*!
        Renders all debug objects.
    */
    static void Render();
    /*!
        Renders fullscreen quad with specified raster.
    */
    static void RenderRaster( RwRaster* );
private:
    static std::list<DebugRenderObject*> m_aDebugObjects;
    static CD3D1XShader*		m_pRenderRasterPS;
};

