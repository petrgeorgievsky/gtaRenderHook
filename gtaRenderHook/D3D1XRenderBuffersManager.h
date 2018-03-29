#pragma once
struct MatrixBuffer
{
	RwMatrix mWorld;
	RwMatrix mView;
	RwMatrix mProjection;
	RwMatrix mInvView;
	RwMatrix mInvViewProj;
};
struct MaterialBuffer
{
	RwRGBAReal diffuseColor;
	float  diffuseIntensity;
	float  specularIntensity;
	float  glossiness;
	int  hasSpecTex;
};
class CD3DRenderer;
class CD3D1XRenderBuffersManager
{
public:
	CD3D1XRenderBuffersManager();
	~CD3D1XRenderBuffersManager();

	void UpdateViewProjMatricles(RwMatrix &view, RwMatrix &proj);
	void UpdateViewMatrix(RwMatrix &view);
	void UpdateViewMatrixLookAtDX(DirectX::XMMATRIX *view);
	void UpdateViewMatrixLookAtProjDX(DirectX::XMMATRIX *view, DirectX::XMMATRIX *proj);
	void UpdateWorldMatrix(RwMatrix *ltm);
	RwMatrix * GetCurrentWorldMatrix() {
		return m_pCurrentWorldMatrix;
	}
	void SetMatrixBuffer();
	void UpdateMaterialDiffuseColor(RwRGBA &color);
	void UpdateMaterialEmmissiveColor(RwRGBA &color);
	void UpdateMaterialSpecularInt(float &intensity);
	void UpdateMaterialGlossiness(float &intensity);
	void UpdateHasSpecTex(const int &hastex);
private:
	MatrixBuffer	m_MatrixBuffer;
	MaterialBuffer	m_MaterialBuffer;
	RwMatrix *		m_pCurrentWorldMatrix = nullptr;
	RwMatrix *		m_pOldWorldMatrix=nullptr;
	ID3D11Buffer*	m_pMaterialCB	= nullptr;
	ID3D11Buffer*	m_pMatrixCB		= nullptr;
};
extern CD3D1XRenderBuffersManager* g_pRenderBuffersMgr;
