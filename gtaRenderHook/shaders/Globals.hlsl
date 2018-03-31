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
    float4 vSunColor;
    float4 vWaterColor;
    float4 vGradingColor0;
    float4 vGradingColor1;
	uint uiLightCount;
    float fFogStart;
    float fFogRange;
    float fFarClip;
    float fTimeStep;
    float3 __pad;
}

cbuffer PerFrameMatrixBuffer : register(b1)
{
	//row_major matrix World;
    row_major matrix View;
    row_major matrix Projection;
    row_major matrix ViewProjection;
    row_major matrix ViewInv;
    row_major matrix ViewProjectionInv;
}

cbuffer PerObjectBuffer : register(b2)
{
    row_major matrix World;
}

cbuffer PerMaterialBuffer : register(b3)
{
	float4	DiffuseColor;
	float	DiffuseIntensity;
	float	SpecularIntensity;
	float	Glossiness;
	int	    HasSpecTex;
}

cbuffer VoxelViewMatrices : register(b4)
{
	row_major matrix VoxelView[6];
    float voxelGridScale;
    float _padding[3];
}

cbuffer PostProcessing : register(b5)
{
    float LumWhite;
    float MiddleGray;
    float __padding[2];
}
cbuffer DeferredRendering : register(b6)
{
    uint SSRMaxIterations;
    float   SSRStep;
    float MaxShadowBlur;
    float MinShadowBlur;
}
static const float voxelGridScale2 = 1.0f;
static const int	voxelGridSize = 32;
#endif