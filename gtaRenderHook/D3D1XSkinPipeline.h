#pragma once
#include "D3D1XPipeline.h"
class CD3D1XSkinPipeline :
	public CD3D1XPipeline
{
public:
	CD3D1XSkinPipeline(CD3DRenderer* pRenderer);
	~CD3D1XSkinPipeline();

	bool Instance(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance);
	void Render(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
private:
	ID3D11Buffer*		m_pMaterialDataBuffer = nullptr;
	ID3D11Buffer*		m_pSkinningDataBuffer = nullptr;

	CD3D1XShader*		m_pHS = nullptr;
	CD3D1XShader*		m_pDS = nullptr;
};

