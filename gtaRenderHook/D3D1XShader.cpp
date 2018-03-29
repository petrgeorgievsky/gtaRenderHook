#include "stdafx.h"
#include "D3D1XShader.h"
#include "D3DRenderer.h"
#include "CDebug.h"
#include "D3D1XStateManager.h"
#include "RwD3D1XEngine.h"
#include <d3dcompiler.h>

#define USE_SHADER_CACHING

#ifdef DEBUG
int CD3D1XShader::m_ShaderCount = 0;
#endif // DEBUG

CD3D1XShader::CD3D1XShader()
{
#ifdef DEBUG
	m_ShaderCount++;
#endif // DEBUG
}

CD3D1XShader::~CD3D1XShader()
{
	if (m_pShaderDC) {
		m_pShaderDC->Release();
		m_pShaderDC = nullptr;
	}
	if (m_pBlob) {
		m_pBlob->Release();
		m_pBlob = nullptr;
	}
}

void CD3D1XShader::Set()
{

}

void CD3D1XShader::ReSet()
{
}

HRESULT CD3D1XShader::CompileShaderFromFile(std::string szFileName, std::string szEntryPoint, std::string szShaderModel, ID3DBlob** ppBlobOut) const
{
	auto hr = S_OK;

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
	const auto featureLevel = GET_D3D_FEATURE_LVL;
	auto featureLvl = to_string(featureLevel);
	const D3D10_SHADER_MACRO defines[] = { {"FEATURE_LEVEL",featureLvl.c_str()}, {} };
	ID3DBlob* pErrorBlob = nullptr;

	auto stemp = std::wstring(szFileName.begin(), szFileName.end());
	hr = D3DCompileFromFile(stemp.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint.c_str(), szShaderModel.c_str(), dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			g_pDebug->printMsg(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()),0);
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

CD3D1XVertexShader* CD3D1XVertexShader::m_pLastShader = nullptr;
CD3D1XPixelShader* CD3D1XPixelShader::m_pLastShader = nullptr;
CD3D1XComputeShader* CD3D1XComputeShader::m_pLastShader = nullptr;
CD3D1XGeometryShader* CD3D1XGeometryShader::m_pLastShader = nullptr;
CD3D1XHullShader* CD3D1XHullShader::m_pLastShader = nullptr;
CD3D1XDomainShader* CD3D1XDomainShader::m_pLastShader = nullptr;

#pragma region Vertex Shader

CD3D1XVertexShader::CD3D1XVertexShader(std::string fileName, std::string entryPoint)
{
	// Compile shader
	std::string shaderModel = "vs_" + g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

	if (FAILED(CompileShaderFromFile(fileName, entryPoint, shaderModel, &m_pBlob)))
		g_pDebug->printError("The FX file cannot be compiled.");

	auto pd3DDevice = GET_D3D_DEVICE;
	// Create shader
	if (FAILED(pd3DDevice->CreateVertexShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, (ID3D11VertexShader**)&m_pShaderDC)))
	{
		m_pBlob->Release();
		m_pBlob = nullptr;
		g_pDebug->printError("Failed to create vertex shader");
	}
}

void CD3D1XVertexShader::Set()
{
#ifdef USE_SHADER_CACHING
	if (this == m_pLastShader)
		return;
#endif // USE_SHADER_CACHING

	CD3D1XShader::Set();
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->VSSetShader(static_cast<ID3D11VertexShader*>(m_pShaderDC), nullptr, 0);

#ifdef USE_SHADER_CACHING
	m_pLastShader = this;
#endif // USE_SHADER_CACHING
	
}

void CD3D1XVertexShader::ReSet()
{
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->VSSetShader(nullptr, nullptr, 0);
	m_pLastShader = nullptr;
}

#pragma endregion

#pragma region Pixel Shader

CD3D1XPixelShader::CD3D1XPixelShader(std::string fileName, std::string entryPoint)
{// Compile shader
	auto shaderModel = "ps_" + g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

	if (FAILED(CompileShaderFromFile(fileName, entryPoint, shaderModel, &m_pBlob)))
		g_pDebug->printError("The FX file cannot be compiled.");

	auto pd3dDevice = GET_D3D_DEVICE;
	// Create shader
	if (FAILED(pd3dDevice->CreatePixelShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11PixelShader**>(&m_pShaderDC))))
	{
		m_pBlob->Release();
		m_pBlob = nullptr;
		g_pDebug->printError("Failed to create vertex shader");
	}
}

void CD3D1XPixelShader::Set()
{
#ifdef USE_SHADER_CACHING
	if (this == m_pLastShader)
		return;
#endif // USE_SHADER_CACHING
	CD3D1XShader::Set();
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->PSSetShader(static_cast<ID3D11PixelShader*>(m_pShaderDC), nullptr, 0); 
#ifdef USE_SHADER_CACHING
	m_pLastShader = this;
#endif // USE_SHADER_CACHING
}
void CD3D1XPixelShader::ReSet()
{
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->PSSetShader(nullptr, nullptr, 0);
	m_pLastShader = nullptr;
}

#pragma endregion

#pragma region Compute Shader

CD3D1XComputeShader::CD3D1XComputeShader(std::string fileName, std::string entryPoint)
{// Compile shader
	std::string shaderModel = "cs_" + g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

	if (FAILED(CompileShaderFromFile(fileName, entryPoint, shaderModel, &m_pBlob)))
		g_pDebug->printError("The FX file cannot be compiled.");

	auto pd3dDevice = GET_D3D_DEVICE;
	// Create shader
	if (FAILED(pd3dDevice->CreateComputeShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11ComputeShader**>(&m_pShaderDC))))
	{
		m_pBlob->Release();
		m_pBlob = nullptr;
		g_pDebug->printError("Failed to create vertex shader");
	}
}

void CD3D1XComputeShader::Set()
{
#ifdef USE_SHADER_CACHING
	if (this == m_pLastShader)
		return;
#endif // USE_SHADER_CACHING
	CD3D1XShader::Set();
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->CSSetShader(static_cast<ID3D11ComputeShader*>(m_pShaderDC), nullptr, 0); 
#ifdef USE_SHADER_CACHING
		m_pLastShader = this;
#endif // USE_SHADER_CACHING
}
void CD3D1XComputeShader::ReSet()
{
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->CSSetShader(nullptr, nullptr, 0);
	m_pLastShader = nullptr;
}

#pragma endregion

#pragma region Geometry Shader

CD3D1XGeometryShader::CD3D1XGeometryShader(std::string fileName, std::string entryPoint)
{// Compile shader
	std::string shaderModel = "gs_" + g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

	if (FAILED(CompileShaderFromFile(fileName, entryPoint, shaderModel, &m_pBlob)))
		g_pDebug->printError("The FX file cannot be compiled.");

	auto pd3dDevice = GET_D3D_DEVICE;
	// Create shader
	if (FAILED(pd3dDevice->CreateGeometryShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11GeometryShader**>(&m_pShaderDC))))
	{
		m_pBlob->Release();
		m_pBlob = nullptr;
		g_pDebug->printError("Failed to create vertex shader");
	}
}

void CD3D1XGeometryShader::Set()
{
#ifdef USE_SHADER_CACHING
	if (this == m_pLastShader)
		return;
#endif // USE_SHADER_CACHING
	CD3D1XShader::Set();
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->GSSetShader(static_cast<ID3D11GeometryShader*>(m_pShaderDC), nullptr, 0); 
#ifdef USE_SHADER_CACHING
	m_pLastShader = this;
#endif // USE_SHADER_CACHING
}
void CD3D1XGeometryShader::ReSet()
{
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->GSSetShader(nullptr, nullptr, 0);
	m_pLastShader = nullptr;
}

#pragma endregion

#pragma region Hull Shader

CD3D1XHullShader::CD3D1XHullShader(std::string fileName, std::string entryPoint)
{// Compile shader
	std::string shaderModel = "hs_" + g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

	if (FAILED(CompileShaderFromFile(fileName, entryPoint, shaderModel, &m_pBlob)))
		g_pDebug->printError("The FX file cannot be compiled.");

	auto pd3dDevice = GET_D3D_DEVICE;
	// Create shader
	if (FAILED(pd3dDevice->CreateHullShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11HullShader**>(&m_pShaderDC))))
	{
		m_pBlob->Release();
		m_pBlob = nullptr;
		g_pDebug->printError("Failed to create vertex shader");
	}
}

void CD3D1XHullShader::Set()
{
#ifdef USE_SHADER_CACHING
	if (this == m_pLastShader)
		return;
#endif // USE_SHADER_CACHING

		CD3D1XShader::Set();
		auto pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->HSSetShader(static_cast<ID3D11HullShader*>(m_pShaderDC), nullptr, 0);
#ifdef USE_SHADER_CACHING
		m_pLastShader = this;
#endif // USE_SHADER_CACHING
}

void CD3D1XHullShader::ReSet()
{
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->HSSetShader(nullptr, nullptr, 0); 
	m_pLastShader = nullptr;
}

#pragma endregion

#pragma region Domain Shader

CD3D1XDomainShader::CD3D1XDomainShader(std::string fileName, std::string entryPoint)
{// Compile shader
	std::string shaderModel = "ds_" + g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);

	if (FAILED(CompileShaderFromFile(fileName, entryPoint, shaderModel, &m_pBlob)))
		g_pDebug->printError("The FX file cannot be compiled.");

	auto pd3dDevice = GET_D3D_DEVICE;
	// Create shader
	if (FAILED(pd3dDevice->CreateDomainShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11DomainShader**>(&m_pShaderDC))))
	{
		m_pBlob->Release();
		m_pBlob = nullptr;
		g_pDebug->printError("Failed to create vertex shader");
	}
}

void CD3D1XDomainShader::Set()
{
#ifdef USE_SHADER_CACHING
	if (this == m_pLastShader)
		return;
#endif // USE_SHADER_CACHING

		CD3D1XShader::Set();
		auto pImmediateContext = GET_D3D_CONTEXT;
		pImmediateContext->DSSetShader(static_cast<ID3D11DomainShader*>(m_pShaderDC), nullptr, 0);
#ifdef USE_SHADER_CACHING
		m_pLastShader = this;
#endif // USE_SHADER_CACHING
}

void CD3D1XDomainShader::ReSet()
{
	auto pImmediateContext = GET_D3D_CONTEXT;
	pImmediateContext->DSSetShader(nullptr, nullptr, 0);
	m_pLastShader = nullptr;
}
#pragma endregion
