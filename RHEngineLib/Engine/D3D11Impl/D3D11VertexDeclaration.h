#pragma once
#include "D3D11Common.h"
namespace RHEngine {
    /*!
        \class CD3D1XVertexDeclaration
        \brief Base D3D vertex declaration class.

        This class represents input vertex layout for specific vertex shader.
    */
    class D3D11VertexDeclaration
    {
    public:
        //D3D11VertexDeclaration(CD3D1XShader* pVS, UINT flags);
        D3D11VertexDeclaration(ID3D11Device* device, const std::vector<D3D11_INPUT_ELEMENT_DESC> &elements, UINT stride);
        ~D3D11VertexDeclaration();
        const std::vector<D3D11_INPUT_ELEMENT_DESC>	&getElementInfo() const { return m_elements; }
        const UINT								&getStride() const { return m_stride; }
        const UINT								&getInputInfo() const { return m_inputInfo; }
        ID3D11InputLayout*						getInputLayout() const { return m_inputLayout; }
    private:
        ID3D11InputLayout*						m_inputLayout = nullptr;
        std::vector<D3D11_INPUT_ELEMENT_DESC>	m_elements;
        UINT									m_inputInfo, m_stride;
    };
};