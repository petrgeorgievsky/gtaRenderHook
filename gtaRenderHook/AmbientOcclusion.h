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
    static void RenderAO( RwRaster* normalsDepth );
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
class AmbientOcclusionSettingsBlock : public SettingsBlock
{
public:
    AmbientOcclusionSettingsBlock()
    {
        m_sName = "AmbientOcclusion";
        m_aFields["EnableAO"] = new ToggleSField( "EnableAO", false, false, false, m_sName, true );
        m_aFields["SampleCount"] = new UIntSField( "SampleCount", false, false, true, m_sName, 8, 0, 128, 1 );
        m_aFields["Radius"] = new FloatSField( "Radius", false, false, false, m_sName, 0.5f, 0.005f, 5.0, 0.001f );
        m_aFields["Intesity"] = new FloatSField( "Intesity", false, false, false, m_sName, 1.0f, 0.001f, 5.0, 0.001f );
        m_aFields["Curve"] = new FloatSField( "Curve", false, false, false, m_sName, 1.5f, 0.01f, 10.0f, 0.005f );
        m_aFields["Scale"] = new FloatSField( "Scale", false, false, false, m_sName, 0.75f, 0.1f, 10.0f, 0.05f );

        Reset();
    }
    void Load( const tinyxml2::XMLDocument &doc );
    void InitGUI( TwBar* );
};
extern AmbientOcclusionSettingsBlock gAmbientOcclusionSettings;