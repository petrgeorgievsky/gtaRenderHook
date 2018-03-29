#include "GameMath.hlsl"
//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D       txDiffuse : register( t0 );
SamplerState    samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Position   : POSITION;
    float4 Color      : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT_PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT_PS_INPUT VS(VS_INPUT input)
{
    VS_OUTPUT_PS_INPUT output;
    output.TexCoord = input.TexCoord;
    output.Color    = input.Color.zyxw;
    output.Position = float4((input.Position.x) * 2 / fScreenWidth - 1.0f, 
                      1.0f - (input.Position.y) * 2 / fScreenHeight, 
                              input.Position.z, 1.0); // transform to screen space
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(float4 Pos : SV_POSITION, VS_OUTPUT_PS_INPUT input ) : SV_Target
{
	float4 outColor;
	if(bHasTexture!=0)
        outColor = float4(txDiffuse.Sample(samLinear, input.TexCoord) * input.Color);
	else
        outColor = float4(input.Color);
    DO_ALPHA_TEST(outColor.a)
	return outColor;
}