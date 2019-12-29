// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "ShadowPipeline.h"
#include "D3D1XShader.h"
#include "D3D1XShaderDefines.h"

#ifndef DebuggingShaders
CShadowPipeline::CShadowPipeline( const std::string &pipeName ) : CD3D1XPipeline{ pipeName }
#else
CShadowPipeline::CShadowPipeline( const std::wstring &pipeName ) : m_sPipeName{ pipeName }
#endif // !DebuggingShaders
{
#ifndef DebuggingShaders
    std::string shaderPath = "shaders/" + m_sPipeName + ".hlsl";
#else
    std::wstring shaderPath = L"shaders/" + m_sPipeName + L".hlsl";
#endif // !DebuggingShaders
    CD3D1XShaderDefineList defineList;
    defineList.AddDefine( "HAS_TEXTURE", "1" );
    m_pShadowPS = new CD3D1XPixelShader( shaderPath, "ShadowPS", &defineList );
    m_pShadowPS_NoTex = new CD3D1XPixelShader( shaderPath, "ShadowPS" );
}


CShadowPipeline::~CShadowPipeline()
{
    delete m_pShadowPS;
    delete m_pShadowPS_NoTex;
}

void CShadowPipeline::SetShadowPipeShader( bool textured )
{
    if ( textured )
        m_pShadowPS->Set();
    else
        m_pShadowPS_NoTex->Set();
}
