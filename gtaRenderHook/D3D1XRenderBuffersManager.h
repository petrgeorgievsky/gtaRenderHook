#pragma once
#include "D3D1XConstantBuffer.h"
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
};
struct PerObjectMatrixBuffer
{
    RwMatrix mWorld;
    RwMatrix mWorldInv;
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
    float       padd__[2];
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
};
extern CD3D1XRenderBuffersManager* g_pRenderBuffersMgr;
