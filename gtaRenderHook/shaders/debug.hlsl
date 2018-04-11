//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )
{
	row_major matrix World;
	row_major matrix View;
	row_major matrix Projection;
}

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
    outPos = mul( outPos, World );
	outPos = mul( outPos, View );
    outPos = mul( outPos, Projection );
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