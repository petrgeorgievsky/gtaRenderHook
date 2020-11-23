#pragma once
#include "D3D1XConstantBuffer.h"
#include "SettingsHolder.h"

class CD3D1XShader;

struct CBCloudRendering
{
    float WindDirX;
    float WindDirY;
    float WindDirZ;
    float Time;
    float CloudsColor[4];

    float CloudStartHeight;
    float CloudEndHeight;
    float CloudCoverage;
    float CloudSpeed;
};

class CCloudRendering
{
  public:
    static void Init();
    static void Shutdown();

    static void RenderVolumetricEffects( RwRaster *normalsDepth, RwRaster *from, RwRaster *to );
    static void QueueTextureReload();
    static void GenerateCloudsTexture();

    static bool m_bRequiresReloading;

  private:
    static CD3D1XShader *m_pRenderCloudsPS;
    static CD3D1XShader *m_pVolumetricCombinePS;
    static RwRaster *    m_pCloudsRaster;
    static RwRaster *    m_pNoise3DRaster;
    static CD3D1XConstantBuffer<CBCloudRendering> *m_pCloudRenderingCB;
};

/*!
    Cloud rendering settings.
*/
class CloudRenderingSettingsBlock : public SettingsBlock
{
  public:
    CloudRenderingSettingsBlock()
    {
        m_sName = "CloudRendering";
        m_aFields["EnableCloudRendering"] =
            new ToggleSField( "EnableCloudRendering", false, false, false, m_sName, true );
        m_aFields["CloudStartHeight"] =
            new FloatSField( "CloudStartHeight", false, false, false, m_sName,
                             200.0f, 0.0f, 10000.0f, 1.f );
        m_aFields["CloudEndHeight"] =
            new FloatSField( "CloudEndHeight", false, false, false, m_sName,
                             400.0f, 0.0f, 10000.0f, 1.f );
        m_aFields["CloudSpeed"] =
            new FloatSField( "CloudSpeed", false, false, false, m_sName,
                             1.0f, 0.0f, 100.0f, 1.f );
        m_aFields["CloudsRenderingScale"] =
            new FloatSField( "CloudsRenderingScale", false, false, false,
                             m_sName, 0.95f, 0.25f, 4.0f, 0.01f );
        m_aFields["CloudsRaymarchingSteps"] =
            new UIntSField( "CloudsRaymarchingSteps", false, false, false,
                            m_sName, 16, 2, 256 );
        m_aFields["SunRaymarchingSteps"] =
            new UIntSField( "SunRaymarchingSteps", false, false, false,
                            m_sName, 2, 1, 32 );
        Reset();
    }
    void Load( const tinyxml2::XMLDocument &doc );
    void InitGUI( TwBar * );
};
extern CloudRenderingSettingsBlock gCloudRenderingSettings;