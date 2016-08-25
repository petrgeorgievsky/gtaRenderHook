#include "stdafx.h"
#include "D3D1XShader.h"
#include "D3DRenderer.h"
#include "CDebug.h"

#ifndef DebuggingShaders
CD3D1XShader::CD3D1XShader(CD3DRenderer* pRenderer, RwD3D1XShaderType type, LPCSTR fileName, LPCSTR entryPoint) : m_pRenderer{ pRenderer }
#else
CD3D1XShader::CD3D1XShader(CD3DRenderer* pRenderer, RwD3D1XShaderType type, LPCWSTR fileName, LPCSTR entryPoint) : m_pRenderer{ pRenderer }
#endif
{
	m_type = type;
	// Compile the shader
	std::string shaderModel;
	std::string shaderVersion = "4_0_level_9_1";
	auto featureLevel = m_pRenderer->getFeatureLevel();
	if (featureLevel < D3D_FEATURE_LEVEL_10_0 && (m_type == RwD3D1XShaderType::GS))
		return;
	if (featureLevel < D3D_FEATURE_LEVEL_11_0 && (m_type == RwD3D1XShaderType::DS || m_type == RwD3D1XShaderType::HS))
		return;
	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1:
		shaderVersion = "4_0_level_9_1";
		break;
	case D3D_FEATURE_LEVEL_9_2:
		shaderVersion = "4_0_level_9_2";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		shaderVersion = "4_0_level_9_3";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		shaderVersion = "4_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		shaderVersion = "4_0";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		shaderVersion = "5_0";
		break;
	case D3D_FEATURE_LEVEL_11_1:
		shaderVersion = "5_0";
		break;
	default:
		break;
	}
	switch (type)
	{
	case RwD3D1XShaderType::VS:
		shaderModel = ("vs_" + shaderVersion);
		break;
	case RwD3D1XShaderType::PS:
		shaderModel = ("ps_" + shaderVersion);
		break;
	case RwD3D1XShaderType::GS:
		shaderModel = ("gs_" + shaderVersion);
		break;
	case RwD3D1XShaderType::HS:
		shaderModel = ("hs_" + shaderVersion);
		break;
	case RwD3D1XShaderType::DS:
		shaderModel = ("ds_" + shaderVersion);
		break;
	}
	
	if (FAILED(CompileShaderFromFile(fileName, entryPoint, shaderModel.c_str(), &m_pBlob)))
		g_pDebug->printError("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.");
	
	auto pd3dDevice= m_pRenderer->getDevice();
	// Create the shader
	HRESULT hr=S_OK;
	switch (type)
	{
	case RwD3D1XShaderType::VS:
		hr = pd3dDevice->CreateVertexShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &m_pShader.VS);
		break;
	case RwD3D1XShaderType::PS:
		hr = pd3dDevice->CreatePixelShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &m_pShader.PS);
		break;
	case RwD3D1XShaderType::GS:
		hr = pd3dDevice->CreateGeometryShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &m_pShader.GS);
		break;
	case RwD3D1XShaderType::HS:
		hr = pd3dDevice->CreateHullShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &m_pShader.HS);
		break;
	case RwD3D1XShaderType::DS:
		hr = pd3dDevice->CreateDomainShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &m_pShader.DS);
		break;
	}
	if (FAILED(hr))
	{
		m_pBlob->Release();
		g_pDebug->printError("failed to create shader");
	}
}


CD3D1XShader::~CD3D1XShader()
{
	auto featureLevel = m_pRenderer->getFeatureLevel();
	if (featureLevel < D3D_FEATURE_LEVEL_10_0 && (m_type == RwD3D1XShaderType::GS))
		return;
	if (featureLevel < D3D_FEATURE_LEVEL_11_0 && (m_type == RwD3D1XShaderType::DS || m_type == RwD3D1XShaderType::HS))
		return;
	switch (m_type)
	{
	case RwD3D1XShaderType::VS:
		if (m_pShader.VS)
			m_pShader.VS->Release();
		break;
	case RwD3D1XShaderType::PS:
		if (m_pShader.PS)
			m_pShader.PS->Release();
		break;
	case RwD3D1XShaderType::HS:
		if (m_pShader.HS)
			m_pShader.HS->Release();
		break;
	case RwD3D1XShaderType::GS:
		if (m_pShader.GS)
			m_pShader.GS->Release();
		break;
	case RwD3D1XShaderType::DS:
		if (m_pShader.DS)
			m_pShader.DS->Release();
		break;
	}
	if (m_pBlob)
		m_pBlob->Release();
}

void CD3D1XShader::Set()
{
	auto featureLevel = m_pRenderer->getFeatureLevel();
	if (featureLevel < D3D_FEATURE_LEVEL_10_0 && (m_type == RwD3D1XShaderType::GS))
		return;
	if (featureLevel < D3D_FEATURE_LEVEL_11_0 && (m_type == RwD3D1XShaderType::DS || m_type == RwD3D1XShaderType::HS))
		return;
	auto pImmediateContext = m_pRenderer->getContext();
	switch (m_type)
	{
	case RwD3D1XShaderType::VS:
		pImmediateContext->VSSetShader(m_pShader.VS, nullptr, 0);
		break;
	case RwD3D1XShaderType::PS:
		pImmediateContext->PSSetShader(m_pShader.PS, nullptr, 0);
		break;
	case RwD3D1XShaderType::DS:
		pImmediateContext->DSSetShader(m_pShader.DS, nullptr, 0);
		break;
	case RwD3D1XShaderType::HS:
		pImmediateContext->HSSetShader(m_pShader.HS, nullptr, 0);
		break;
	case RwD3D1XShaderType::GS:
		pImmediateContext->GSSetShader(m_pShader.GS, nullptr, 0);
		break;
	}
}
void CD3D1XShader::ReSet()
{
	auto featureLevel = m_pRenderer->getFeatureLevel();
	if (featureLevel < D3D_FEATURE_LEVEL_10_0 && (m_type == RwD3D1XShaderType::GS))
		return;
	if (featureLevel < D3D_FEATURE_LEVEL_11_0 && (m_type == RwD3D1XShaderType::DS || m_type == RwD3D1XShaderType::HS))
		return;
	auto pImmediateContext = m_pRenderer->getContext();
	switch (m_type)
	{
	case RwD3D1XShaderType::VS:
		pImmediateContext->VSSetShader(nullptr, nullptr, 0);
		break;
	case RwD3D1XShaderType::PS:
		pImmediateContext->PSSetShader(nullptr, nullptr, 0);
		break;
	case RwD3D1XShaderType::DS:
		pImmediateContext->DSSetShader(nullptr, nullptr, 0);
		break;
	case RwD3D1XShaderType::HS:
		pImmediateContext->HSSetShader(nullptr, nullptr, 0);
		break;
	case RwD3D1XShaderType::GS:
		pImmediateContext->GSSetShader(nullptr, nullptr, 0);
		break;
}
}
#ifndef DebuggingShaders
HRESULT CD3D1XShader::CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
#else
HRESULT CD3D1XShader::CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
#endif
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = (1 << 11);
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= (1 << 0);

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= (1 << 2);
#endif
	auto featureLevel = m_pRenderer->getFeatureLevel();
	std::string featureLvl = to_string(featureLevel);
	const D3D10_SHADER_MACRO defines[] = { {"FEATURE_LEVEL",featureLvl.c_str()}, {} };
	ID3DBlob* pErrorBlob = nullptr;
#ifndef DebuggingShaders
	hr = D3DX11CompileFromFile(szFileName, defines,nullptr,szEntryPoint,szShaderModel, dwShaderFlags,0,nullptr,ppBlobOut,&pErrorBlob, &hr);
#else
	hr = D3DCompileFromFile(szFileName, defines, nullptr, szEntryPoint, szShaderModel,	dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
#endif // !1
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			g_pDebug->printMsg(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}