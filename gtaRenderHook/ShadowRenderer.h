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

    void DirectionalLightTransform( RwCamera* mainCam, const RW::V3d & lightDir, int shadowCascade );
    RW::V3d CalculateCameraPos( RwCamera* mainCam, const RW::V3d & lightDir, int shadowCascade );
    void CalculateFrustumPoints( RwReal fNear, RwReal fFar, RwCamera* camera, RW::V3d* points );
    void RenderShadowToBuffer( int cascade, void( *render )( int cascade ) );
    void SetShadowBuffer() const;
    void CalculateShadowDistances( RwReal fNear, RwReal fFar );
    void QueueTextureReload();
    void DrawDebug();
    /*!
        Chooses the cascades to re-render this frame (FarCascadeUpdateInterval). Every cascade is re-rendered when
        there were no shadows last frame (m_bShadowHistoryValid) or the light direction changed.
    */
    void SelectCascadesToUpdate( const RW::V3d &lightDir );
    bool IsCascadeUpdated( int cascade ) const { return m_bCascadeUpdated[cascade]; }
    bool AllCascadesUpdated() const;
    RwCamera* m_pShadowCamera = nullptr;
    float     m_fShadowDistances[5]{};
    bool	m_bShadowsRendered = false;
    // shadows were rendered last frame, so cascades kept from it are still valid
    bool	m_bShadowHistoryValid = false;
    bool	m_bRequiresReloading = false;
    /*!
        Light aligned bounding box for each shadow cascade
    */
    static RW::BBox m_LightBBox[4];
    static RwV3d m_LightPos[4];
    static RW::Matrix m_LightSpaceMatrix[4];
    static RW::Matrix m_InvLightSpaceMatrix[4];
    static RW::V3d m_vFrustumCorners[4][8];
private:
    void ClearCascade( int cascade );
    bool    m_bCascadeUpdated[4]{ true, true, true, true };
    bool    m_bForceFullUpdate = true;
    UINT    m_uiFrame = 0;
    RW::V3d m_vLastLightDir{ 0, 0, 0 };
    class CD3D1XShader* m_pDepthClearPS = nullptr;
    RwMatrix* m_pLightViewProj;
    CD3D1XConstantBuffer<CBShadows>* m_pLightCB;
};

class ShadowSettingsBlock : public SettingsBlock
{
public:
    ShadowSettingsBlock()
    {
        m_sName = "Shadows";
        m_aFields["Enable"] = new ToggleSField( "Enable", false, false, false, m_sName, true );
        m_aFields["CullPerCascade"] = new ToggleSField( "CullPerCascade", false, false, false, m_sName );
        m_aFields["ScanShadowsBehindPlayer"] = new ToggleSField( "ScanShadowsBehindPlayer", false, false, false, m_sName, true );
        m_aFields["MaxDrawDistance"] = new FloatSField( "MaxDrawDistance", false, false, false, m_sName, 500.0f, 5.0f, 30000.0f, 5.0f );
        m_aFields["LodShadowsMinDistance"] = new FloatSField( "LodShadowsMinDistance", false, false, false, m_sName, 150.0f, 2.0f, 3000.0f, 1.0f );
        m_aFields["MaxSectorsAroundPlayer"] = new UIntSField( "MaxSectorsAroundPlayer", false, false, false, m_sName, 3, 1, 10, 1 );
        // cascades from the third on are re-rendered every this many frames, staggered (1 = every frame)
        m_aFields["FarCascadeUpdateInterval"] = new UIntSField( "FarCascadeUpdateInterval", false, false, false, m_sName, 1, 1, 8, 1 );
        Reset();
    }
    tinyxml2::XMLElement* Save( tinyxml2::XMLDocument* doc );
    void Load( const tinyxml2::XMLDocument &doc );
    void Reset();
    void InitGUI( TwBar* );
public:
    UINT Size;
    float DistanceCoefficients[3];
    float BiasCoefficients[4];
    int ShadowCascadeCount;
};
extern ShadowSettingsBlock gShadowSettings;
