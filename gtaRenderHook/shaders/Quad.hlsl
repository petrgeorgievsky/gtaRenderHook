// Fullscreen quad. Used for postprocessing

//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
Texture2D txPrevFrame : register(t0);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
struct VS_QUAD_IN
{
    float4 vPosition : POSITION;
    float2 vTexCoord : TEXCOORD0;
};
struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_QUAD_IN VS(VS_QUAD_IN i)
{
    PS_QUAD_IN o;
    o.vPosition = i.vPosition;
    o.vTexCoord = float4(i.vTexCoord, 0, 1);
    return o;
}

float4 BlitPS(PS_QUAD_IN i) : SV_Target
{
    return txPrevFrame.Sample(samLinear, i.vTexCoord.xy);
}