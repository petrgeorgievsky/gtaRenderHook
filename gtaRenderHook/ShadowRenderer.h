#pragma once
#include "RwVectorMath.h"
#include "SettingsHolder.h"
#include "AntTweakBar.h"
#include "D3D1XConstantBuffer.h"
struct CBShadows
{
	RwMatrix ViewProj[4];
	float	FadeDistances[5];
	int		ShadowSize;
	int		CascadeCount;
	float	pad;
	float	ShadowBias[4];
};

/*!
	\class CShadowRenderer
	\brief Cascaded shadow map renderer.

	This class responsible for rendering shadow maps.
*/
class CShadowRenderer
{
public:
	CShadowRenderer();
	~CShadowRenderer();

	void DirectionalLightTransform(RwCamera* mainCam, const RW::V3d & lightDir, int shadowCascade);
	RW::V3d CalculateCameraPos(RwCamera* mainCam, const RW::V3d & lightDir, int shadowCascade);
	void CalculateFrustumPoints(RwReal fNear, RwReal fFar, RwCamera* camera, RW::V3d* points);
	void RenderShadowToBuffer(int cascade, void(*render)(int cascade));
	void SetShadowBuffer() const;
	void CalculateShadowDistances(RwReal fNear, RwReal fFar);
	void QueueTextureReload();
	RwCamera* m_pShadowCamera = nullptr;
	float	m_fShadowDistances[5];
	bool	m_bShadowsRendered = false;
	bool	m_bRequiresReloading = false;
	/*!
		Light aligned bounding box for each shadow cascade
	*/
	static RW::BBox m_LightBBox[4];
	static RwV3d m_LightPos[4];
	static RW::Matrix m_LightSpaceMatrix[4];
	static RW::Matrix m_InvLightSpaceMatrix[4];
private:
	RwMatrix* m_pLightViewProj;
	CD3D1XConstantBuffer<CBShadows>* m_pLightCB;
	std::list<void*> m_aShadowObjectCacheList;
};

class ShadowSettingsBlock : public SettingsBlock {
public:
	ShadowSettingsBlock() {
		m_sName = "ShadowSettings";
		Reset();
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar*);
public:
	bool Enable;
	UINT Size;
	float DistanceCoefficients[3];
	float BiasCoefficients[4];
	float MaxDrawDistance;
	float MinOffscreenShadowCasterSize;
	float LodShadowsMinDistance;
	int MaxSectorsAroundPlayer;
	int ShadowCascadeCount;
	bool CullPerCascade;
	bool ScanShadowsBehindPlayer;
};
extern ShadowSettingsBlock gShadowSettings;
