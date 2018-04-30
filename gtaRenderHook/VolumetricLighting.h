#pragma once
#include "SettingsHolder.h"
#include <AntTweakBar.h>
#include "D3D1XConstantBuffer.h"
class CD3D1XShader;

struct CBVolumetricLighting
{
	float RaymarchingDistance;
	float SunlightBlendOffset;
	float SunlightIntensity;
	float padding___;
};

/*!
	\class CVolumetricLighting
	\brief Volumetric lighting rendering alghorithm.

	This class responsible for rendering volumetric effects, like GodRays, Clouds and Fog.
*/
class CVolumetricLighting
{
public:
	/*!
		Initializes volumetric lighting resources.
	*/
	static void Init();
	/*!
		Releases volumetric lighting resources
	*/
	static void Shutdown();
	/*!
		Renders all volumetric lighting and combines it with result raster.
	*/
	static void RenderVolumetricEffects(RwRaster* normalsDepth, RwRaster* cascadeShadowMap, RwRaster* result);
	static void	QueueTextureReload();
	static bool	m_bRequiresReloading;
private:
	static CD3D1XShader* m_pVolumetricSunlightPS;
	static CD3D1XShader* m_pVolumetricCombinePS;
	static RwRaster*	 m_pVolumeLightingRaster;
	static CD3D1XConstantBuffer<CBVolumetricLighting>* m_pVolumeLightingCB;
};

/*!
	Volumetric lighting settings.
*/
class VolumetricLightingSettingsBlock : public SettingsBlock {
public:
	VolumetricLightingSettingsBlock() {
		m_sName = "VolumetricLightingSettings";
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar*);
public:
	unsigned int SunlightRaymarchingSteps;
	float	RaymarchingDistance;
	float	SunlightBlendOffset;
	float	SunlightIntensity;
	float	VolumetricRenderingScale;
};
extern VolumetricLightingSettingsBlock gVolumetricLightingSettings;