#include "Globals.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
float4 VS( in float4 Pos : POSITION ) : SV_POSITION
{
	float4 outPos=float4(Pos.xyz,1.0);// transform to screen space
    outPos = mul( outPos, mWorld );
	outPos = mul( outPos, mView );
    outPos = mul( outPos, mProjection );
    return outPos;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( float4 Pos : SV_POSITION ) : SV_Target
{
	float4 outColor=float4(1,1,1,1);
	return outColor;
}

float4 RenderRasterPS(PS_QUAD_IN i) : SV_Target
{
    float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
    return outColor;
}