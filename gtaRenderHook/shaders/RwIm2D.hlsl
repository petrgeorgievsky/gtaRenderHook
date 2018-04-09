#include "GameMath.hlsl"
//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D       txDiffuse : register( t0 );
SamplerState    samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct VS_IM2D_IN
{
    float4 vPosition   : POSITION;
    float4 cColor      : COLOR;
    float2 vTexCoord : TEXCOORD;
};

struct PS_IM2D_IN
{
    float4 vPosition : SV_POSITION;
    float4 cColor : COLOR;
    float2 vTexCoord : TEXCOORD;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_IM2D_IN VS(VS_IM2D_IN i)
{
    PS_IM2D_IN o;
    o.vTexCoord = i.vTexCoord;
    o.cColor    = i.cColor.zyxw;
    o.vPosition = float4((i.vPosition.x) * 2 / fScreenWidth - 1.0f, 
                      1.0f - (i.vPosition.y) * 2 / fScreenHeight, 
                              i.vPosition.z, 1.0); // transform to screen space
    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_IM2D_IN i) : SV_Target
{
	float4 OutColor;
	if(bHasTexture!=0)
        OutColor = float4(txDiffuse.Sample(samLinear, i.vTexCoord) * i.cColor);
	else
        OutColor = float4(i.cColor);
    DO_ALPHA_TEST(OutColor.a)
    return OutColor;
}