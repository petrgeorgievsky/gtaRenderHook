//--------------------------------------------------------------------------------------
// Bindings
//--------------------------------------------------------------------------------------

Texture2D       t0  : register(t0);
SamplerState    s0  : register(s0);

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct VS_IM2D_IN
{
    float4 vPosition    : POSITION;
    float4 cColor       : COLOR;
    float2 vTexCoord    : TEXCOORD;
};

struct PS_IM2D_IN
{
    float4 vPosition    : SV_POSITION;
    float4 cColor       : COLOR;
    float2 vTexCoord    : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shaders
//--------------------------------------------------------------------------------------

PS_IM2D_IN BaseVS(VS_IM2D_IN i)
{
    PS_IM2D_IN o;
    
    float fScreenWidth = 1280;
    float fScreenHeight = 720;

    o.vTexCoord = i.vTexCoord;
    o.cColor = i.cColor.rgba;

    o.vPosition = float4((i.vPosition.x) * 2 / fScreenWidth - 1.0f,
                      1.0f - (i.vPosition.y) * 2 / fScreenHeight,
                              i.vPosition.z, 1.0); // transform to screen space
    //o.vPosition = float4(i.vPosition.xy /* float2(2.0f / fScreenWidth, -2.0f / fScreenHeight) - float2(1.0f, -1.0f)*/, 0.0f, 1.0f); // transform to screen space
    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 NoTexPS( PS_IM2D_IN i ) : SV_Target
{
    float4 OutColor;
    OutColor = i.cColor;
    return OutColor;
}

float4 TexPS( PS_IM2D_IN i ) : SV_Target
{
    float4 OutColor;
    OutColor = t0.Sample( s0, i.vTexCoord );
    return OutColor;
}