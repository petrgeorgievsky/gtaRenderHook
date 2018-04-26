#include "stdafx.h"
#include "D3D1XShader.h"
#include "D3DRenderer.h"
#include "CDebug.h"
#include "D3D1XStateManager.h"
#include "RwD3D1XEngine.h"
#include "D3D1XShaderDefines.h"
#include "D3DSpecificHelpers.h"
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

void CD3D1XShader::Reload(CD3D1XShaderDefineList* localShaderDefineList)
{
}

HRESULT CD3D1XShader::CompileShaderFromFile(std::string szFileName, std::string szEntryPoint,
	std::string szShaderModel, ID3DBlob** ppBlobOut, CD3D1XShaderDefineList* localShaderDefineList) const
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
	std::vector<D3D_SHADER_MACRO> defines;
	auto defineList		 = g_pGlobalShaderDefines->GetDefineList();
	// concat local defines at the end if they are provided
	if (localShaderDefineList != nullptr) {
		auto localDefineList = localShaderDefineList->GetDefineList();
		defineList.insert(defineList.end(), localDefineList.begin(), localDefineList.end());
	}
	// convet list to required form
	for (const auto &define : defineList) {
		defines.push_back({define.m_sName.c_str(), define.m_sDefinition.c_str() });
	}
	defines.push_back({}); 
	ID3DBlob* pErrorBlob = nullptr;

	auto stemp = std::wstring(szFileName.begin(), szFileName.end());
	hr = D3DCompileFromFile(stemp.c_str(), defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint.c_str(), szShaderModel.c_str(), dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

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
		g_pDebug->printError("Failed to create vertex shader:" + fileName + "/" + entryPoint);
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

CD3D1XPixelShader::CD3D1XPixelShader(std::string fileName, std::string entryPoint, CD3D1XShaderDefineList* localShaderDefineList)
{
	// Compile shader
	m_sShaderModel = "ps_" + g_pStateMgr->GetShaderModel(GET_D3D_FEATURE_LVL);
	m_sFilePath = fileName;
	m_sEntryPoint = entryPoint;
	
	if (!CALL_D3D_API(CompileShaderFromFile(m_sFilePath, m_sEntryPoint, m_sShaderModel, &m_pBlob, localShaderDefineList),
		"Pixel shader compilation error:" + fileName + "/" + entryPoint))
		return;

	// Create shader resource
	if (!CALL_D3D_API(GET_D3D_DEVICE->CreatePixelShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr,
		reinterpret_cast<ID3D11PixelShader**>(&m_pShaderDC)),
		"Failed to create pixel shader:"+fileName+"/"+entryPoint))
	{
		m_pBlob->Release();
		m_pBlob = nullptr;
	}
}

void CD3D1XPixelShader::Set()
{
#ifdef USE_SHADER_CACHING
	if (this == m_pLastShader)
		return;
#endif // USE_SHADER_CACHING
	CD3D1XShader::Set();
	GET_D3D_CONTEXT->PSSetShader(static_cast<ID3D11PixelShader*>(m_pShaderDC), nullptr, 0);
#ifdef USE_SHADER_CACHING
	m_pLastShader = this;
#endif // USE_SHADER_CACHING
}
void CD3D1XPixelShader::ReSet()
{
	GET_D3D_CONTEXT->PSSetShader(nullptr, nullptr, 0);
	m_pLastShader = nullptr;
}

void CD3D1XPixelShader::Reload(CD3D1XShaderDefineList* localShaderDefineList)
{
	// try to create shaders, if succeed than release old resources and set new
	ID3D10Blob* pBlob;
	ID3D11PixelShader* pShader;
	// recompile shader
	if (!CALL_D3D_API_SILENT(CompileShaderFromFile(m_sFilePath, m_sEntryPoint, m_sShaderModel, &pBlob, localShaderDefineList),
		"Pixel shader compilation error:" + m_sFilePath + "/" + m_sEntryPoint))
		return;
	// create shader resource
	if (!CALL_D3D_API_SILENT(GET_D3D_DEVICE->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
		nullptr, &pShader),
		"Failed to create pixel shader:" + m_sFilePath + "/" + m_sEntryPoint))
	{
		pBlob->Release();
		pBlob = nullptr;
		return;
	}

	// TODO: check if they are used to avoid unexpected behavior
	if (m_pShaderDC) {
		m_pShaderDC->Release();
		m_pShaderDC = pShader;
	}
	if (m_pBlob) {
		m_pBlob->Release();
		m_pBlob = pBlob;
	}
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
		g_pDebug->printError("Failed to create compute shader:" + fileName + "/" + entryPoint);
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
		g_pDebug->printError("Failed to create geometry shader:" + fileName + "/" + entryPoint);
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
		g_pDebug->printError("Failed to create hull shader:" + fileName + "/" + entryPoint);
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
		g_pDebug->printError("Failed to create domain shader:" + fileName + "/" + entryPoint);
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
