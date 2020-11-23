#pragma once
#include "D3D1XShader.h"
#include "D3D1XConstantBuffer.h"
#include "RwD3D1XEngine.h"
struct CBTemporalAA
{
    RwGraphicsMatrix PrevView;
    RwV3d	MovementVec;
    float	BlendFactor;

    float	MBMaxSpeed;
    float	MBMinPixelDistance;
    float	MBEdgeScale;
    float	MBCenterScale;
};
class CTemporalAA
{
public:
    static void Init();
    static void Shutdown();
    static void JitterProjMatrix();
    static void SaveViewMatrix();
    static void Render( RwRaster *input, RwRaster *output, RwRaster *gBuff1,
                        RwRaster *gBuff2 );
    static void QueueTextureReload();

  private:
    static CD3D1XShader * m_pTemporalAA;
    static RwRaster* m_pAccumRasters[2];
    static int m_nCurrentAccRaster;
    static int m_nCurrentJitterSample;
    static bool m_bFirstTimeRender;
    static RwV3d m_vPrevCamPos;
    static RwGraphicsMatrix m_mPrevView;
    static CD3D1XConstantBuffer<CBTemporalAA>* m_pTAABuffer;
};

/*!
    Temporal antialiasing settings.
*/
class TAASettingsBlock : public SettingsBlock
{
public:
    TAASettingsBlock()
    {
        m_sName = "TAA";
        m_aFields["EnableTAA"] = new ToggleSField( "EnableTAA", false, false, false, m_sName, true );
        m_aFields["MotionBlurFallback"] = new ToggleSField( "MotionBlurFallback", false, false, true, m_sName, true );
        // count of frames participating in temporal anti-aliasing
        m_aFields["FrameCount"] = new IntSField( "FrameCount", false, false, false, m_sName, 4, 2, 480, 1 );
        m_aFields["SubPixelScale"] = new FloatSField( "SubPixelScale", false, false, false, m_sName, 1.0f, 0.1f, 2.0f, 0.1f );
        m_aFields["SubPixelCount"] = new UIntSField( "SubPixelCount", false, false, false, m_sName, 8, 2, 8, 1 );
        m_aFields["MBMaxSpeed"] = new FloatSField( "MBMaxSpeed", false, false, false, m_sName, 64.0f, 0.1f, 1000.0f, 0.1f );
        m_aFields["MBMinPixelDistance"] = new FloatSField( "MBMinPixelDistance", false, false, false, m_sName, 2.0f, 0.1f, 32.0f, 0.1f );
        m_aFields["MBEdgeScale"] = new FloatSField( "MBEdgeScale", false, false, false, m_sName, 2.0f, 0.001f, 128.0f, 0.001f );
        m_aFields["MBCenterScale"] = new FloatSField( "MBCenterScale", false, false, false, m_sName, 0.005f, 0.001f, 128.0f, 0.001f );
        Reset();
    }
    void Load( const tinyxml2::XMLDocument &doc );
    void InitGUI( TwBar* );
};
extern TAASettingsBlock gTAASettingsBlock;