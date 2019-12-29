#pragma once
#include "SettingsHolder.h"
#include <AntTweakBar.h>
#include "D3D1XConstantBuffer.h"
class CD3D1XShader;
/*!
    \class CBlurAlgorithms
    \brief Image blur algorithms.

    This class responsible for blurring images using different blur filters.
*/
class CBlurAlgorithms
{
public:
    /*!
        Initializes resources
    */
    static void Init();
    /*!
        Releases resources
    */
    static void Shutdown();
    /*!
        Blurs image with gaussian filter with static kernel
    */
    static void Gaussian( int kernel, RwRaster* tex, bool halfSize );
    /*!
        Blurs image with space-aware filter(bilateral) with static kernel.
    */
    static void Spatial( int kernel, RwRaster* tex, RwRaster* depthNormals, bool halfSize );
    /*!
        Blurs image with directional filter using direction map
    */
    static void Directional( int sampleCount, RwRaster* tex, RwRaster* directionMap, bool halfSize );
    /*!
        Blurs image with directional filter using custom direction
    */
    static void Directional( int sampleCount, RwRaster* tex, const RwV3d &direction, bool halfSize );
    /*!
        Blurs image with radial filter around some point in world space
    */
    static void Radial( int sampleCount, RwRaster* tex, const RwV3d &point, bool halfSize );

private:
    static CD3D1XShader* m_pGaussianXPS;
    static CD3D1XShader* m_pGaussianYPS;
    static CD3D1XShader* m_pSpatialXPS;
    static CD3D1XShader* m_pSpatialYPS;
    static CD3D1XShader* m_pDirectionalVecPS;
    static CD3D1XShader* m_pDirectionalTexPS;
    static CD3D1XShader* m_pRadialPS;

    static RwRaster*	 m_pHalfSizeRasterH;
    static RwRaster*	 m_pHalfSizeRasterV;
    static RwRaster*	 m_pFullSizeRasterH;
    static RwRaster*	 m_pFullSizeRasterV;
};

