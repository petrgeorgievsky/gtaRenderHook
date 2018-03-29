// Fullscreen quad. Used for postprocessing
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
struct VSInput_Quad
{
    float4 pos : POSITION;
    float2 texcoord : TEXCOORD0;
};
struct PSInput_Quad
{
    float4 pos : SV_Position;
    float4 texCoordOut : TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PSInput_Quad VS(VSInput_Quad input)
{
    PSInput_Quad output;
    output.pos = input.pos;
    float2 tc0 = input.texcoord;
    output.texCoordOut = float4(tc0, 0, 1);
    
    return output;
}