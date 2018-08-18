#pragma once
#include "D3D11Shader.h"
namespace RHEngine {
    /*!
        \class D3D11VertexShader
        \brief D3D11 vertex shader class.

        This class represents vertex shader.
    */
    class D3D11VertexShader: public D3D11Shader
    {
    public:
        /*!
            Initializes shader resource.
        */
        D3D11VertexShader(ID3D11Device* device, String fileName, String entryPoint);
        /*!
            Releases shader resource.
        */
        virtual ~D3D11VertexShader();
        /*!
            Sets shader to a D3D context.
        */
        void Set(ID3D11DeviceContext* context) override;
        /*!
            Removes shader from a D3D context.
        */
        void ReSet(ID3D11DeviceContext* context) override;
        /*!
            Releases resources, and reloads shader from file.
        */
        //virtual void Reload(CD3D1XShaderDefineList* localShaderDefineList = nullptr);
        /*!
		    Returns shader byte-code blob.
	    */
	    ID3DBlob* getBlob() { return m_pBlob; }
    protected:
        // Shader pointer.
        ID3D11DeviceChild*	m_pShaderDC = nullptr;
        // Blob pointer.
        ID3DBlob*			m_pBlob = nullptr;
        // Shader file path
        String			m_sFilePath;
        // Shader entry point
        String			m_sEntryPoint;
        // Shader model
        String			m_sShaderModel;
    };
};