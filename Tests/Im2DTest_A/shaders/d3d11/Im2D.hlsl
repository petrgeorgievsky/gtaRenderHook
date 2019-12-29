//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D       txDiffuse : register( t0 );
SamplerState    sLinear : register( s0 );

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
PS_IM2D_IN BaseVS(VS_IM2D_IN i)
{
    PS_IM2D_IN o;
    o.vTexCoord = i.vTexCoord;
    o.cColor    = i.cColor.zyxw;
	float fScreenWidth = 640;
	float fScreenHeight = 480;
    o.vPosition = float4((i.vPosition.x) * 2 / fScreenWidth - 1.0f, 
                      1.0f - (i.vPosition.y) * 2 / fScreenHeight, 
                              i.vPosition.z, 1.0); // transform to screen space
    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 NoTexPS(PS_IM2D_IN i) : SV_Target
{
	float4 OutColor;
	float fScreenWidth = 640;
	float fScreenHeight = 480;
	OutColor = float4(i.cColor.rgb,1);
    return OutColor;
}