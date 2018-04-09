#ifndef GLOBALS
#define GLOBALS

cbuffer RenderStateBuffer : register( b0 )
{
	uint	bHasTexture;
	float	fScreenWidth;
	float	fScreenHeight;
	uint	uiAlphaTestType;
	float	fAlphaTestRef;
	uint	bFogEnable;
	uint	uiFogType;
	uint	cFogColor;
	float4  vCameraPos;
	float4  vSunLightDir;
	float4  vSkyLightCol;
    float4  vHorizonCol;
    float4  vSunColor;
    float4  vWaterColor;
    float4  vGradingColor0;
    float4  vGradingColor1;
	uint    uiLightCount;
    float   fFogStart;
    float   fFogRange;
    float   fFarClip;
    float   fTimeStep;
    float3 __pad;
}

cbuffer PerFrameMatrixBuffer : register(b1)
{
	//row_major matrix World;
    row_major matrix mView;
    row_major matrix mProjection;
    row_major matrix mViewProjection;
    row_major matrix mViewInv;
    row_major matrix mViewProjectionInv;
}

cbuffer PerObjectBuffer : register(b2)
{
    row_major matrix mWorld;
}

cbuffer PerMaterialBuffer : register(b3)
{
	float4	cDiffuseColor;
	float	fDiffuseIntensity;
	float	fSpecularIntensity;
	float	fGlossiness;
    int    bHasSpecTex;
}

cbuffer VoxelViewMatrices : register(b4)
{
	row_major matrix VoxelView[6];
    float voxelGridScale;
    float _padding[3];
}

cbuffer PostProcessing : register(b5)
{
    float fLumWhite;
    float fMiddleGray;
    float __padding[2];
}
cbuffer DeferredRendering : register(b6)
{
    uint  SSRMaxIterations;
    float fSSRStep;
    float fMaxShadowBlur;
    float fMinShadowBlur;
}
static const float voxelGridScale2 = 1.0f;
static const int	voxelGridSize = 32;
static const float3 g_vLuminance = float3(.299, .587, .114);
static const float g_fHDRBrightTreshold = 0.5f;
#endif