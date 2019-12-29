//--------------------------------------------------------------------------------------
// Bindings
//--------------------------------------------------------------------------------------

Texture2D tPrev : register(t0);
Texture2D tCurr : register(t1);
SamplerState s0 : register(s0);

cbuffer SceneVariables : register(b1)
{
    float random_a;
    float random_b;
    uint frame_count;
    uint random_b_ui;
};

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct VS_IM2D_IN
{
    float4 vPosition : POSITION;
    float4 cColor : COLOR;
    float2 vTexCoord : TEXCOORD;
};

struct PS_IM2D_IN
{
    float4 vPosition : SV_POSITION;
    float4 cColor : COLOR;
    float2 vTexCoord : TEXCOORD;
};


float4 Accumulate(PS_IM2D_IN i) : SV_Target
{
    float4 OutColor;
    OutColor = (tCurr.Sample(s0, i.vTexCoord) * ((float) frame_count - 1.0f) + tPrev.Sample(s0, i.vTexCoord)) / ((float) frame_count);
    return OutColor;
}