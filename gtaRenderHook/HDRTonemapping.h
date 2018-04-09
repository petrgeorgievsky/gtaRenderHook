#pragma once
#include "PostProcessEffect.h"
#include "SettingsHolder.h"
#include "D3D1XConstantBuffer.h"
#include <AntTweakBar.h>
class CD3D1XShader;
#define NUM_TONEMAP_TEXTURES  3       // Number of stages in the 3x3 down-scaling for post-processing in PS
static const int ToneMappingTexSize = (int)pow(3.0f, NUM_TONEMAP_TEXTURES - 1);
struct CBPostProcess
{
	float	LumWhite;
	float	MiddleGray;
	float	pad[2];
};
class CHDRTonemapping :
	public CPostProcessEffect
{
public:
	CHDRTonemapping();
	~CHDRTonemapping();
	virtual void Render(RwRaster* input);
private:
	CD3D1XShader* m_pLogAvg;
	CD3D1XShader* m_pTonemap;
	CD3D1XShader* m_pDownScale2x2_Lum;
	CD3D1XShader* m_pDownScale3x3; 
	CD3D1XShader* m_pAdaptationPass;
	CD3D1XShader* m_pDownScale3x3_BrightPass;
	RwRaster*	m_pAdaptationRaster[2];
	RwRaster*	m_pBrightPassRaster;
	RwRaster*	m_pToneMapRaster[NUM_TONEMAP_TEXTURES];
	UINT m_nCurrentAdaptationRaster=0;
	CD3D1XConstantBuffer<CBPostProcess>* m_pPostFXBuffer;
};

// Tonemap settings block class
class TonemapSettingsBlock : public SettingsBlock {
public:
	TonemapSettingsBlock() {
		m_sName = "TonemapSettings";
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar*);
public:
	float LumWhiteDay;
	float LumWhiteNight;
	float MiddleGrayDay;
	float MiddleGrayNight;
	bool EnableGTAColorCorrection;
	bool EnableTonemapping;
	float GetCurrentLumWhite();
	float GetCurrentMiddleGray();
};
extern TonemapSettingsBlock gTonemapSettings;
