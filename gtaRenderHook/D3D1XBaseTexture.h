#pragma once
enum class eTextureDimension {
	TT_Unk,
	TT_1D,
	TT_2D,
	TT_3D
};
class CD3D1XBaseTexture
{
public:
	CD3D1XBaseTexture(RwRaster* parent, eTextureDimension dimension, const std::string& resourceTypeName);
	virtual ~CD3D1XBaseTexture();
	virtual void SetDebugName(const std::string& name);
	ID3D11Resource* GetResource() const { return m_pTextureResource; }
protected:
	ID3D11Resource*		m_pTextureResource	= nullptr;
	RwRaster*			m_pParent			= nullptr;
	eTextureDimension	m_dimension			= eTextureDimension::TT_Unk;
	std::string			m_resourceTypeName  = "BaseTexture";
};

