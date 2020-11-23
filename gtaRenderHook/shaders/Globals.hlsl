#ifndef GLOBALS
#define GLOBALS
/*!
    Global render state buffer, currently game dependant 
*/
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
	float4  vCameraPos;     // wrong not to be used atm
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
    float3 __pad;           // padding to align structure to 16 bytes, unaligned structures are way slower
}
/*!
    Matrices updated each frame
*/
cbuffer PerFrameMatrixBuffer : register(b1)
{
    row_major matrix mView;
    row_major matrix mProjection;
    row_major matrix mViewProjection;
    row_major matrix mViewInv;
    row_major matrix mViewProjectionInv;
}
/*!
    Matrices updated for each object
*/
cbuffer PerObjectBuffer : register(b2)
{
    row_major matrix mWorld;
    row_major matrix mWorldInv;
}
/*!
    Matrices updated for each material
*/
cbuffer PerMaterialBuffer : register(b3)
{
    float4 cDiffuseColor : packoffset(c0);
    float fDiffuseIntensity : packoffset(c1.x);
    float fSpecularIntensity : packoffset(c1.y);
    float fGlossiness : packoffset(c1.z);
    float fMetallness : packoffset(c1.w);
    float2 ___padding : packoffset( c2 );
    int    bHasNormalTex : packoffset( c2.z );
    int    bHasSpecTex : packoffset( c2.w );
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
    float2 __padding;
}
cbuffer DeferredRendering : register(b6)
{
    uint  uiSSRMaxIterations;
    float fSSRStep;
    float fMaxShadowBlur;
    float fMinShadowBlur;
}
cbuffer VolumetricLighting : register(b7)
{
    float RaymarchingDistance;
    float SunlightBlendOffset;
    float SunlightIntensity;
    float padding___;
}
cbuffer AmbientOcclusion : register(b8)
{
    float AOIntensity;
    float AORadius;
    float AOCurve;
    float padding__;
}
cbuffer TemporalAA : register(b9)
{
    row_major matrix mPrevView;
    float3 TAAMovementVec;
    float TAABlendFactor;
    float MBMaxSpeed;
    float MBMinPixelDistance;
    float MBEdgeScale;
    float MBCenterScale;
}
cbuffer Clouds : register( b10 )
{
    float3 WindDir;
    float Time;
    float4 CloudsColor;

    float CloudStartHeight;
    float CloudEndHeight;
    float CloudCoverage;
    float CloudSpeed;
}
static const float voxelGridScale2 = 1.0f;
static const int	voxelGridSize = 32;
// Luminance vector
static const float3 g_vLuminance = float3(.299, .587, .114);
static const float  g_fHDRBrightTreshold = 0.7f;
#endif