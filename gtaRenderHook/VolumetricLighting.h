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
    static void RenderVolumetricEffects( RwRaster* normalsDepth, RwRaster* cascadeShadowMap, RwRaster* from, RwRaster* to );
    static void	QueueTextureReload();
    static bool	m_bRequiresReloading;
private:
    static CD3D1XShader *m_pVolumetricSunlightPS;
    static CD3D1XShader *m_pVolumetricCombinePS;
    static RwRaster *m_pVolumeLightingRaster;
    static CD3D1XConstantBuffer<CBVolumetricLighting> *m_pVolumeLightingCB;
};

/*!
    Volumetric lighting settings.
*/
class VolumetricLightingSettingsBlock : public SettingsBlock
{
public:
    VolumetricLightingSettingsBlock()
    {
        m_sName = "VolumetricLighting";
        m_aFields["EnableVolumetricLighting"] = new ToggleSField( "EnableVolumetricLighting", false, false, false, m_sName, true );
        m_aFields["RaymarchingDistance"] = new FloatSField( "RaymarchingDistance", false, false, false, m_sName, 40.0f, 1.0f, 1000.0f, 0.5f );
        m_aFields["SunlightBlendOffset"] = new FloatSField( "SunlightBlendOffset", false, false, false, m_sName, 0.25f, -1.0f, 1.0f, 0.01f );
        m_aFields["SunlightIntensity"] = new FloatSField( "SunlightIntensity", false, false, false, m_sName, 0.25f, 0.001f, 5.0f, 0.001f );
        m_aFields["VolumetricRenderingScale"] = new FloatSField( "VolumetricRenderingScale", false, false, false, m_sName, 0.95f, 0.25f, 1.0f, 0.01f );
        m_aFields["SunlightRaymarchingSteps"] = new UIntSField( "SunlightRaymarchingSteps", false, false, false, m_sName, 16, 2, 256 );
        Reset();
    }
    void Load( const tinyxml2::XMLDocument &doc );
    void InitGUI( TwBar* );
};
extern VolumetricLightingSettingsBlock gVolumetricLightingSettings;