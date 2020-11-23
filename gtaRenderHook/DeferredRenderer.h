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
    void	RenderToGBuffer( void( *renderCB )( ) );
    void	RenderOutput();
    void	RenderToCubemap( void( *renderCB )( ) );
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
    RwRaster*	m_aDeferredRasters[4];
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
    UINT m_uiCurrentFinalRaster = 0;

};
extern CDeferredRenderer* g_pDeferredRenderer;
extern UINT m_uiDeferredStage;
// Tonemap settings block class
class DeferredSettingsBlock : public SettingsBlock
{
public:
    DeferredSettingsBlock()
    {
        m_sName = "Deferred";
        // TODO: Separate by groups
        //m_aFields["Enabled"] = new ToggleSField("Enabled", false, false, false, m_sName, true);
        m_aFields["BlurShadows"] = new ToggleSField(
            "BlurShadows", false, false, true, m_sName, true );
        m_aFields["UseDitheringForAlphaObjects"] = new ToggleSField(
            "UseDitheringForAlphaObjects", false, false, false, m_sName, false );
        m_aFields["UsePCSS"] = new ToggleSField( "UsePCSS", false, false, true, m_sName, true );
        m_aFields["SampleShadows"] = new ToggleSField( "SampleShadows", false, false, true, m_sName, true );
        m_aFields["UseSSR"] = new ToggleSField( "UseSSR", false, false, true, m_sName, true );
        m_aFields["SampleCubemap"] = new ToggleSField( "SampleCubemap", false, false, true, m_sName, true );
        m_aFields["SSRStep"] = new FloatSField( "SSRStep", false, false, false, m_sName, 0.0025f, 0.0005f, 10.0f, 0.005f );
        m_aFields["SSRScale"] = new FloatSField( "SSRScale", false, false, false, m_sName, 0.75f, 0.1f, 1.0f, 0.05f );
        m_aFields["MaxShadowBlur"] = new FloatSField( "MaxShadowBlur", false, false, false, m_sName, 4.0f, 0.0f, 10.0f, 0.005f );
        m_aFields["MinShadowBlur"] = new FloatSField( "MinShadowBlur", false, false, false, m_sName, 0.3f, 0.0f, 10.0f, 0.005f );

        m_aFields["SSRMaxIterations"] = new UIntSField( "SSRMaxIterations", false, false, true, m_sName, 24, 2, 128, 1 );
        m_aFields["CubemapSize"] = new UIntSField( "CubemapSize", true, false, true, m_sName, 128, 8, 4096, 1 );
        m_aFields["ShadowsBlurKernelSize"] = new UIntSField( "ShadowsBlurKernelSize", false, false, true, m_sName, 2, 1, 10, 1 );
        Reset();
    }
    void Load( const tinyxml2::XMLDocument &doc );
    void InitGUI( TwBar* );
};
extern DeferredSettingsBlock gDeferredSettings;