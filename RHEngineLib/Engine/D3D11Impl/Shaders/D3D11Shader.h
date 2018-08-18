#pragma once
#include "../D3D11Common.h"
namespace RHEngine {
    /*!
        \class CD3D1XShader
        \brief Base D3D shader class.

        This class represents shader of some sort.
    */
    class D3D11Shader
    {
    public:
        /*!
            Initializes shader resource.
        */
        D3D11Shader();
        /*!
            Releases shader resource.
        */
        virtual ~D3D11Shader();
        /*!
            Sets shader to a D3D context.
        */
        virtual void Set(ID3D11DeviceContext* context)=0;
        /*!
            Removes shader from a D3D context.
        */
        virtual void ReSet(ID3D11DeviceContext* context)=0;
        /*!
            Releases resources, and reloads shader from file.
        */
        //virtual void Reload(CD3D1XShaderDefineList* localShaderDefineList = nullptr);
    public:
        /*!
            Compiles shader with custom entry point and shader model
        */
        HRESULT CompileShaderFromFile(String fileName, String entryPoint, String shaderModel, ID3DBlob** blobOut) const;
    protected:
        // Shader pointer.
        ID3D11DeviceChild*	m_pShaderDC = nullptr;
        // Shader file path
        String			m_sFilePath;
        // Shader entry point
        String			m_sEntryPoint;
        // Shader model
        String			m_sShaderModel;
    };
};