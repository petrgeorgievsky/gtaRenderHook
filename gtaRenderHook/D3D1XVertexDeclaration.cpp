#include "stdafx.h"
#include "D3D1XVertexDeclaration.h"
#include "D3D1XShader.h"
#include "D3DRenderer.h"
#include "RwD3D1XEngine.h"
#include "CDebug.h"
#include "D3DSpecificHelpers.h"

CD3D1XVertexDeclaration::CD3D1XVertexDeclaration(CD3D1XShader* pVS,UINT flags)
{
	m_pShader = pVS;
	m_stride = 0;
	if (flags&rpGEOMETRYPOSITIONS) {
		m_elements.push_back({ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, m_stride,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
		m_stride += 12;
	}
	if (flags&rpGEOMETRYNORMALS) {
		m_elements.push_back({ "NORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, m_stride,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
		m_stride += 12;
	}
	if (flags&rpGEOMETRYTEXTURED || flags&rpGEOMETRYTEXTURED2) {
		m_elements.push_back({ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, m_stride,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
		m_stride += 8;
	}
	if (flags&rpGEOMETRYPRELIT) {
		m_elements.push_back({ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, m_stride,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
		m_stride += 4;
	}
	m_inputInfo = flags;
	CALL_D3D_API(GET_D3D_DEVICE->CreateInputLayout(m_elements.data(), m_elements.size(), m_pShader->getBlob()->GetBufferPointer(), m_pShader->getBlob()->GetBufferSize(), &m_inputLayout),
		"Failed to create Input Layout");
}

CD3D1XVertexDeclaration::CD3D1XVertexDeclaration(std::vector<D3D11_INPUT_ELEMENT_DESC> elements, UINT stride, CD3D1XShader * pVS)
{
	m_pShader = pVS;
	m_stride = stride;
	m_elements = elements;
	auto blob = m_pShader->getBlob();
	CALL_D3D_API(GET_D3D_DEVICE->CreateInputLayout(m_elements.data(), m_elements.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayout),
		"Failed to create input layout for vertex shader");
}


CD3D1XVertexDeclaration::~CD3D1XVertexDeclaration()
{
	if (m_inputLayout) { 
		m_inputLayout->Release(); 
		m_inputLayout = nullptr;
	}
}
