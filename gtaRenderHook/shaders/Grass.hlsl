/*
    Instanced grass (CGrassRendererRH), drawn into the G-buffer.
    One instance per tuft, placed like CGrassRenderer::DrawTriPlants: plant matrix right = (sxy,0,0),
    up = (0,sxy,0), at = (bend,bend,sz), position = the tuft's spot in the plant triangle.
*/
#include "GameMath.hlsl"
#include "GBuffer.hlsl"

Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

// grass colour multiplier (plants.dat colour x intensity is the base)
#define GRASS_BRIGHTNESS 1.0f
// how far grass normals lean from the tuft's own normal toward straight up; 1 = lit like flat ground
#define GRASS_NORMAL_UP 0.7f
// texture alpha below this is cut (as the building pipeline does)
#define GRASS_ALPHA_CUTOFF 0.3f

struct VS_GRASS_IN
{
    float3 vPosition    : POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float3 vNormal      : NORMAL;
    float4 vInstPos     : TEXCOORD5;    // xyz position, w scale xy
    float3 vInstParams  : TEXCOORD6;    // x scale z, y wind bend, z distance fade
    float4 cInstColor   : COLOR;
};

struct PS_GRASS_IN
{
    float4 vPosition    : SV_POSITION;
    float4 vColor       : COLOR;
    float4 vNormalDepth : NORMAL;
    float4 vTexCoord    : TEXCOORD0;    // xy uv, z distance fade
};

PS_GRASS_IN VS( VS_GRASS_IN i )
{
    PS_GRASS_IN o;
    const float ScaleXY = i.vInstPos.w;
    const float ScaleZ  = i.vInstParams.x;
    const float Bend    = i.vInstParams.y;

    float3 WorldPos = float3( i.vPosition.x * ScaleXY + i.vPosition.z * Bend,
                              i.vPosition.y * ScaleXY + i.vPosition.z * Bend,
                              i.vPosition.z * ScaleZ ) + i.vInstPos.xyz;
    float4 ViewPos = mul( float4( WorldPos, 1.0f ), mView );
    o.vPosition    = mul( ViewPos, mProjection );

    float3 Normal   = normalize( lerp( i.vNormal, float3( 0.0f, 0.0f, 1.0f ), GRASS_NORMAL_UP ) );
    o.vNormalDepth  = float4( Normal, ViewPos.z );
    o.vTexCoord     = float4( i.vTexCoord, i.vInstParams.z, 0.0f );
    o.vColor        = i.cInstColor;
    return o;
}

PS_DEFERRED_OUT PS( PS_GRASS_IN i )
{
    float4 Albedo = txDiffuse.Sample( samLinear, i.vTexCoord.xy );
    if ( Albedo.a < GRASS_ALPHA_CUTOFF )
        discard;
    // dithered fade over the last 20 m, TAA smooths it
    if ( i.vTexCoord.z < Bayer4x4( i.vPosition.xy ) - 0.03f )
        discard;

    PS_DEFERRED_OUT Out;
    FillGBuffer( Out, float4( Albedo.rgb * i.vColor.rgb * GRASS_BRIGHTNESS, 1.0f ), i.vNormalDepth.xyz,
                 i.vNormalDepth.w, float4( 0.0f, 0.1f, 0.0f, 0.0f ) );
    return Out;
}
