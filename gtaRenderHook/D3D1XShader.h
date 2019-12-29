#ifndef D3D1XShader_h__
#define D3D1XShader_h__
class CD3DRenderer;
class CD3D1XShaderDefineList;
// TODO: Divide this into separate files perhaps
/*!
    \class CD3D1XShader
    \brief Base D3D shader class.

    This class represents shader of some sort.
*/
class CD3D1XShader
{
public:
    /*!
        Initializes shader resource.
    */
    CD3D1XShader();
    /*!
        Releases shader resource.
    */
    virtual ~CD3D1XShader();
    /*!
        Sets shader to a D3D context.
    */
    virtual void Set();
    /*!
        Removes shader from a D3D context.
    */
    virtual void ReSet();
    /*!
        Releases resources, and reloads shader from file.
    */
    virtual void Reload( CD3D1XShaderDefineList* localShaderDefineList = nullptr );
    /*!
        Returns shader byte-code blob.
    */
    ID3DBlob* getBlob() { return m_pBlob; }
public:
    /*!
        Compiles shader with custom entry point and shader model
    */
    HRESULT CompileShaderFromFile( std::string szFileName, std::string szEntryPoint,
                                   std::string szShaderModel, ID3DBlob** ppBlobOut, CD3D1XShaderDefineList* localShaderDefineList = nullptr ) const;
protected:
    // Shader pointer.
    ID3D11DeviceChild*	m_pShaderDC = nullptr;
    // Blob pointer.
    ID3DBlob*			m_pBlob = nullptr;
    // Shader file path
    std::string			m_sFilePath;
    // Shader entry point
    std::string			m_sEntryPoint;
    // Shader model
    std::string			m_sShaderModel;
#ifdef DEBUG
    // Shader counter(for debug info)
    static int			 m_ShaderCount;
#endif // DEBUG
};

// Vertex Shader
class CD3D1XVertexShader : public CD3D1XShader
{
public:
    CD3D1XVertexShader( std::string fileName, std::string entryPoint );
    void Set();
    void ReSet();
    // Last shader pointer.
    static CD3D1XVertexShader* m_pLastShader;
};

// Pixel Shader
class CD3D1XPixelShader : public CD3D1XShader
{
public:
    CD3D1XPixelShader( std::string fileName, std::string entryPoint, CD3D1XShaderDefineList* localShaderDefineList = nullptr );
    void Set();
    void ReSet();
    void Reload( CD3D1XShaderDefineList* localShaderDefineList = nullptr );
    // Last shader pointer.
    static CD3D1XPixelShader* m_pLastShader;
};

// Compute Shader
class CD3D1XComputeShader : public CD3D1XShader
{
public:
    CD3D1XComputeShader( std::string fileName, std::string entryPoint );
    void Set();
    void ReSet();
    // Last shader pointer.
    static CD3D1XComputeShader* m_pLastShader;
};

// Geometry Shader
class CD3D1XGeometryShader : public CD3D1XShader
{
public:
    CD3D1XGeometryShader( std::string fileName, std::string entryPoint );
    void Set();
    void ReSet();
    // Last shader pointer.
    static CD3D1XGeometryShader* m_pLastShader;
};

// Hull Shader
class CD3D1XHullShader : public CD3D1XShader
{
public:
    CD3D1XHullShader( std::string fileName, std::string entryPoint );
    void Set();
    void ReSet();
    // Last shader pointer.
    static CD3D1XHullShader* m_pLastShader;
};

// Domain Shader
class CD3D1XDomainShader : public CD3D1XShader
{
public:
    CD3D1XDomainShader( std::string fileName, std::string entryPoint );
    void Set();
    void ReSet();
    // Last shader pointer.
    static CD3D1XDomainShader* m_pLastShader;
};
#endif // D3D1XShader_h__
