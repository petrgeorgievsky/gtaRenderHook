#include "GameMath.hlsl"
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

struct VS_IM3D_IN
{
    float4 vPosition    : POSITION;
    float4 cColor       : COLOR;
    float2 vTexCoord    : TEXCOORD;
};

struct PS_IM3D_IN
{
    float4 vPosition    : SV_POSITION;
    float4 cColor       : COLOR;
    float2 vTexCoord    : TEXCOORD;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_IM3D_IN VS(VS_IM3D_IN i)
{
    PS_IM3D_IN o;
	o.vTexCoord = i.vTexCoord;
	o.cColor    = i.cColor.zyxw;
	float4 outPos = float4(i.vPosition.xyz,1.0);// transform to world space
    outPos = mul( outPos, mWorld );
	outPos = mul( outPos, mView );
    outPos = mul( outPos, mProjection );
    o.vPosition = outPos;
    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_IM3D_IN i) : SV_Target
{
	float4 OutColor;
	if(bHasTexture!=0)
        OutColor = float4(txDiffuse.Sample(samLinear, i.vTexCoord) * i.cColor);
	else
        OutColor = float4(i.cColor);
	
    DO_ALPHA_TEST(OutColor.w)
    return OutColor;
}