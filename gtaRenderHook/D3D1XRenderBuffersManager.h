#pragma once
#include "D3D1XConstantBuffer.h"
#include <unordered_map>
/*!
    Matrix buffer structure, represents common transformation matrices used for rendering.
*/
struct PerFrameMatrixBuffer
{
    RwMatrix mView;
    RwMatrix mProjection;
    RwMatrix mViewProjection;
    RwMatrix mInvView;
    RwMatrix mInvViewProj;
    // Object motion vectors (main camera only, see CTemporalAA::JitterProjMatrix): last frame's view-projection
    // without jitter, and this frame's jitter (xy, NDC, as added to the projection _31/_32); z = 1 when valid
    RwMatrix mPrevViewProj;
    float    vMotionJitter[4];
};
struct PerObjectMatrixBuffer
{
    RwMatrix mWorld;
    RwMatrix mWorldInv;
    RwMatrix mWorldPrev; // world matrix of the same frame (RwMatrix) in the previous rendered frame
};
/*!

*/
struct MaterialBuffer
{
    RwRGBAReal	diffuseColor;
    float		diffuseIntensity;
    float		specularIntensity;
    float		glossiness;
    float		metallness;
    float       alphaCutoff;    // dual-pass alpha objects: alpha test value, 0 = off
    float       alphaBlendPass; // 1 in the blended pass of a dual-pass mesh
    int         hasNormalTex;
    int			hasSpecTex;
};
struct RwGraphicsMatrix;
class CD3DRenderer;
/*! \class CD3D1XRenderBuffersManager
    \brief Render buffers manager class.

    This class manages render buffers for current frame or current draw call. Responsible for matrix and material state update.
*/
class CD3D1XRenderBuffersManager
{
public:
    /*!
        Initializes per-frame, per-object and per-material constant buffers
    */
    CD3D1XRenderBuffersManager();
    /*!
        Releases allocated resources
    */
    ~CD3D1XRenderBuffersManager();

    /*!
        Updates current view and projection matrices
    */
    void UpdateViewProjMatricles( RwMatrix &view, RwMatrix &proj );
    /*!
        Updates current view matrix
    */
    void UpdateViewMatrix( RwMatrix &view );
    /*!
        Sets the camera part of object motion vectors for the current camera: last frame's view-projection without
        jitter and this frame's jitter. UpdateViewProjMatricles resets it (no motion for other cameras).
    */
    void SetMotionCamera( const RwMatrix &prevViewProj, float jitterX, float jitterY );
    /*!
        Starts a new rendered frame for the world matrix history (previous world matrices)
    */
    void NextFrame();
    /*!
        Updates current world matrix
    */
    void UpdateWorldMatrix( RwMatrix *ltm );
    /*!
        Returns current world matrix
    */
    RwMatrix * GetCurrentWorldMatrix()
    {
        return m_pCurrentWorldMatrix;
    }
    /*!
        Multiplies 2 matrices in homogenous space(using 4th vector in matrix)
    */
    void Multipy4x4Matrices( RwGraphicsMatrix* res, RwGraphicsMatrix* a, RwGraphicsMatrix* b );
    /*!
        Inverts 4x4 matrix
    */
    void Inverse4x4Matrix( RwGraphicsMatrix* a, RwGraphicsMatrix* b );
    /*!
        Transposes 4x4 matrix
    */
    void Transpose4x4Matrix( RwGraphicsMatrix* a, RwGraphicsMatrix* b );
    /*!
        Updates current world space matrix buffer
    */
    void SetMatrixBuffer();
    /*!
        Updates current diffuse color for material
    */
    void UpdateMaterialDiffuseColor( const RwRGBA &color );
    /*!
        Updates current emmissive color for material
    */
    void UpdateMaterialEmmissiveColor( const RwRGBA &color );
    /*!
        Updates current specular intensity for material
    */
    void UpdateMaterialSpecularInt( const float &intensity );
    /*!
        Updates current glossiness for material
    */
    void UpdateMaterialGlossiness( const float &intensity );
    /*!
        Updates current metallness for material
    */
    void UpdateMaterialMetalness( const float &intensity );
    /*!
        Updates current value of specular texture avaliablity
    */
    void UpdateHasSpecTex( const int &hastex );
    /*!
        Updates current value of normal texture avaliablity
    */
    void UpdateHasNormalTex( const int &hastex );
    /*!
        Updates dual-pass alpha parameters: alpha test value for the G-buffer and shadow passes (0 = off), and
        whether this is the blended pass, which then draws only the pixels below the cutoff
    */
    void UpdateAlphaPass( float cutoff, bool blendPass );
    /*!
        Flushes all material buffer changes if there was any
    */
    void FlushMaterialBuffer();
private:
    CD3D1XConstantBuffer<PerFrameMatrixBuffer> *	m_pPerFrameMatrixBuffer = nullptr;
    CD3D1XConstantBuffer<PerObjectMatrixBuffer> *	m_pPerObjectMatrixBuffer = nullptr;
    CD3D1XConstantBuffer<MaterialBuffer> *			m_pPerMaterialBuffer = nullptr;
    bool m_bMaterialBufferRequiresUpdate = false;
    RwMatrix *		m_pCurrentWorldMatrix = nullptr;
    RwMatrix *		m_pOldWorldMatrix = nullptr;
    // world matrix as last uploaded, see SetMatrixBuffer
    RwMatrix		m_mUploadedWorld{};
    RwMatrix		m_mUploadedWorldPrev{};
    // previous world matrix of each frame matrix (RwFrame LTM) drawn, for object motion vectors
    struct WorldHistory
    {
        RwMatrix current;
        RwMatrix previous;
        unsigned int frame;
    };
    std::unordered_map<RwMatrix *, WorldHistory> m_worldHistory;
    unsigned int m_nFrame = 2; // new history entries have frame 0, which must not count as "last frame"
};
extern CD3D1XRenderBuffersManager* g_pRenderBuffersMgr;
