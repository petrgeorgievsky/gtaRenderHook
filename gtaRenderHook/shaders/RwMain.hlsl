#include "GameMath.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

struct VS_MAIN_IN
{
    float3 vPosition   	: POSITION;
    float2 vTexCoord     	: TEXCOORD;
    float3 vInNormal    	: NORMAL;
    float4 vInColor : COLOR;
    float3 vInTangents : TEXCOORD1;
    float3 vInBiTangents : TEXCOORD2;
};

struct PS_MAIN_IN
{
	float4 vPosition	: SV_POSITION;
	float4 vColor		: COLOR;
	float3 vNormal		: NORMAL;
	float2 vTexCoord		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_MAIN_IN VS(VS_MAIN_IN i)
{
    PS_MAIN_IN o;
	float4 	outPos 	= float4( i.vPosition,1.0);// transform to screen space
			outPos 	= mul( outPos, mWorld );
			outPos	= mul( outPos, mView );
    o.vPosition 	= mul( outPos, mProjection );
	o.vNormal   	= mul( i.vInNormal, (float3x3)mWorld);
    o.vTexCoord     = i.vTexCoord;
	o.vColor		= i.vInColor;
	
    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_MAIN_IN i) : SV_Target
{
    float4 OutColor = txDiffuse.Sample(samLinear, i.vTexCoord) * cDiffuseColor;
    OutColor.xyz    *= (max(dot(i.vNormal, vSunLightDir.xyz), 0.0f) * 0.1f + i.vColor.xyz * 0.9f);
    OutColor.a      *= i.vColor.w;
	
    DO_ALPHA_TEST(OutColor.a)
    return OutColor;
}