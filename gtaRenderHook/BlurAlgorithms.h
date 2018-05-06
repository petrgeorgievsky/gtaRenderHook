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
	static void Gaussian(int Kernel, RwRaster* tex);
	/*!
		Blurs image with space-aware filter(bilateral) with static kernel.
	*/
	static void Spatial(int Kernel, RwRaster* tex, RwRaster* depthNormals);
	/*!
		Blurs image with directional filter using direction map
	*/
	static void Directional(int SampleCount, RwRaster* tex, RwRaster* directionMap);
	/*!
		Blurs image with directional filter using custom direction
	*/
	static void Directional(int SampleCount, RwRaster* tex, const RwV3d &direction);
	/*!
		Blurs image with radial filter around some point in world space
	*/
	static void Radial(int SampleCount, RwRaster* tex, const RwV3d &point);

private:
	static CD3D1XShader* m_pGaussianXPS;
	static CD3D1XShader* m_pGaussianYPS;
	static CD3D1XShader* m_pSpatialXPS;
	static CD3D1XShader* m_pSpatialYPS;
	static CD3D1XShader* m_pDirectionalVecPS;
	static CD3D1XShader* m_pDirectionalTexPS;
	static CD3D1XShader* m_pRadialPS;
};

