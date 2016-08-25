#ifndef D3D1XShader_h__
#define D3D1XShader_h__
enum class RwD3D1XShaderType
{
	VS,
	PS,
	GS,
	DS,
	HS
};
class CD3DRenderer;
class CD3D1XShader
{
public:
#ifndef DebuggingShaders
	CD3D1XShader(CD3DRenderer* pRenderer, RwD3D1XShaderType type, LPCSTR fileName, LPCSTR entryPoint);
#else
	CD3D1XShader(CD3DRenderer* pRenderer, RwD3D1XShaderType type, LPCWSTR fileName, LPCSTR entryPoint);
#endif

	~CD3D1XShader();
	void Set();
	void ReSet();
	ID3DBlob* getBlob() { return m_pBlob; }
private:
#ifndef DebuggingShaders
	HRESULT CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
#else
	HRESULT CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
#endif
private:
	CD3DRenderer*						m_pRenderer = nullptr;
	union 
	{
		ID3D11VertexShader* VS;
		ID3D11PixelShader* PS;
		ID3D11HullShader* HS;
		ID3D11DomainShader* DS;
		ID3D11GeometryShader* GS;
	}m_pShader;
	RwD3D1XShaderType m_type;
	ID3DBlob* m_pBlob;
};

#endif // D3D1XShader_h__
