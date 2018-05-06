#pragma once
class CD3D1XShader;
#include "SettingsHolder.h"
#include <AntTweakBar.h>
#include "D3D1XConstantBuffer.h"
struct CBAmbientOcclusion
{
	float AOIntensity;
	float AORadius;
	float AOCurve;
	float padding___;
};

/*!
	\class CAmbientOcclusion
	\brief Ambient occlusion rendering alghorithm.

	This class responsible for rendering ambient occlusion.
*/
class CAmbientOcclusion
{
public:
	/*!
		Initializes ambient occlusion resources.
	*/
	static void Init();
	/*!
		Releases ambient occlusion resources.
	*/
	static void Shutdown();
	/*!
		Renders ambient occlusion.
	*/
	static void RenderAO(RwRaster* normalsDepth);
	/*!
		Returns ambient occlusion raster.
	*/
	static RwRaster* GetAORaster();
	/*!
		Used to reload textures.
	*/
	static void	QueueTextureReload();
	static bool	m_bRequiresReloading;
private:
	static RwRaster* m_pAORaser;
	static CD3D1XShader* m_pAmbientOcclusionPS;
	static CD3D1XConstantBuffer<CBAmbientOcclusion>* m_pAmbientOcclusionCB;
};

/*!
	Ambient occlusion settings.
*/
class AmbientOcclusionSettingsBlock : public SettingsBlock {
public:
	AmbientOcclusionSettingsBlock() {
		m_sName = "AmbientOcclusionSettings";
		Reset();
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar*);
public:
	unsigned int AOSamples;
	float	AORadius;
	float	AOIntesity;
	float	AOCurve;
	float	AOScale;
};
extern AmbientOcclusionSettingsBlock gAmbientOcclusionSettings;