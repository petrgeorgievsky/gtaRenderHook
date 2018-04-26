#pragma once

class CD3D1XShader;
class CShadowRenderer;
class CHDRTonemapping;
class CCubemapReflectionRenderer;
class SettingsBlock;
#include "D3D1XConstantBuffer.h"
#include "SettingsHolder.h"
#include <AntTweakBar.h>
struct CBDeferredRendering
{
	unsigned int SSRMaxIterations;
	float SSRStep;
	float MaxShadowBlur;
	float MinShadowBlur;
};
class CDeferredRenderer
{
public:
	CDeferredRenderer();
	~CDeferredRenderer();
	void	RenderToGBuffer(void(*renderCB)());
	void	RenderOutput();
	void	RenderToCubemap(void(*renderCB)());
	void	RenderTonemappedOutput();
	void	SetNormalDepthRaster();
	void	SetPreviousFinalRaster();
	void	SetPreviousNonTonemappedFinalRaster();
	void	QueueTextureReload();
	CShadowRenderer* m_pShadowRenderer;
	CCubemapReflectionRenderer* m_pReflRenderer;
	CHDRTonemapping* m_pTonemapping;
	bool	m_bRequiresReloading = false;
private:
	RwRaster*	m_aDeferredRasters[3];
	RwRaster*	m_pLightingRaster;
	RwRaster*	m_pFinalRasters[4];
	RwRaster*	m_pReflectionRaster;

	CD3D1XShader* m_pSunLightingPS;
	CD3D1XShader* m_pPointLightingPS;
	CD3D1XShader* m_pFinalPassPS;
	CD3D1XShader* m_pAtmospherePassPS;
	CD3D1XShader* m_pReflectionPassPS;
	CD3D1XShader* m_pBlitPassPS;
	CD3D1XConstantBuffer<CBDeferredRendering>* m_pDeferredBuffer;
	UINT m_uiCurrentFinalRaster=0;
	
};
extern CDeferredRenderer* g_pDeferredRenderer;
extern UINT m_uiDeferredStage;
// Tonemap settings block class
class DeferredSettingsBlock : public SettingsBlock {
public:
	DeferredSettingsBlock() {
		m_sName = "DeferredSettings";
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar*);
public:
	unsigned int SSRMaxIterations;
	unsigned int CubemapSize;
	unsigned int ShadowsBlurKernelSize;
	float SSRStep;
	float SSRScale;
	float MaxShadowBlur;
	float MinShadowBlur;
	bool BlurShadows;
	bool UsePCSS;
	bool SampleShadows;
	bool UseSSR;
	bool SampleCubemap;
};
extern DeferredSettingsBlock gDeferredSettings;