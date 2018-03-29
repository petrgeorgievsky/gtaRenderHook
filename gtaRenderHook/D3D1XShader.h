#ifndef D3D1XShader_h__
#define D3D1XShader_h__
class CD3DRenderer;

// Direct3D Shader base class.
class CD3D1XShader
{
public:
	CD3D1XShader();
	~CD3D1XShader();
	// Sets shader to a D3D context.
	virtual void Set();
	// Removes shader from a D3D context.
	virtual void ReSet();
	// Returns shader byte-code blob.
	ID3DBlob* getBlob() { return m_pBlob; }
public:
	HRESULT CompileShaderFromFile(std::string szFileName, std::string szEntryPoint, std::string szShaderModel, ID3DBlob** ppBlobOut) const;
protected:
	// Shader pointer.
	ID3D11DeviceChild*	m_pShaderDC	= nullptr;
	// Blob pointer.
	ID3DBlob*			m_pBlob		= nullptr;
#ifdef DEBUG
	// Shader counter(for debug info)
	static int			 m_ShaderCount;
#endif // DEBUG
};

// Vertex Shader
class CD3D1XVertexShader : public CD3D1XShader {
public:
	CD3D1XVertexShader(std::string fileName, std::string entryPoint);
	void Set();
	void ReSet();
	// Last shader pointer.
	static CD3D1XVertexShader* m_pLastShader;
};

// Pixel Shader
class CD3D1XPixelShader : public CD3D1XShader {
public:
	CD3D1XPixelShader(std::string fileName, std::string entryPoint);
	void Set();
	void ReSet();
	// Last shader pointer.
	static CD3D1XPixelShader* m_pLastShader;
};

// Compute Shader
class CD3D1XComputeShader : public CD3D1XShader {
public:
	CD3D1XComputeShader(std::string fileName, std::string entryPoint);
	void Set();
	void ReSet();
	// Last shader pointer.
	static CD3D1XComputeShader* m_pLastShader;
};

// Geometry Shader
class CD3D1XGeometryShader : public CD3D1XShader {
public:
	CD3D1XGeometryShader(std::string fileName, std::string entryPoint);
	void Set();
	void ReSet();
	// Last shader pointer.
	static CD3D1XGeometryShader* m_pLastShader;
};

// Hull Shader
class CD3D1XHullShader : public CD3D1XShader {
public:
	CD3D1XHullShader(std::string fileName, std::string entryPoint);
	void Set();
	void ReSet();
	// Last shader pointer.
	static CD3D1XHullShader* m_pLastShader;
};

// Domain Shader
class CD3D1XDomainShader : public CD3D1XShader {
public:
	CD3D1XDomainShader(std::string fileName, std::string entryPoint);
	void Set();
	void ReSet();
	// Last shader pointer.
	static CD3D1XDomainShader* m_pLastShader;
};
#endif // D3D1XShader_h__
