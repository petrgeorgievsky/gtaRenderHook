#include "GBuffer.hlsl"
#include "Globals.hlsl"
#include "GameMath.hlsl"

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};
// normals+depth
Texture2D txGB1 : register(t0);
// Base lighting info, could be used later for some GI approximation, unused right now
//Texture2D txLighting : register(t1);

#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif

#ifndef AO_SAMPLE_COUNT
#define AO_SAMPLE_COUNT 16
#endif

float4 AmbientOcclusionPS(PS_QUAD_IN i) : SV_Target
{
    float4 OutLighting;

    const float3 ViewPos = mViewInv[3].xyz;

    // Some GBuffer info
    float ViewZ;
    float3 Normals;
    GetNormalsAndDepth(txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals);
        
    float3 WorldPos = DepthToWorldPos(ViewZ, i.vTexCoord.xy).xyz;
    
    // Some directions
    float3 ViewDir = normalize(WorldPos - ViewPos);
    float3 LightDir = normalize(vSunLightDir.xyz);
    
    
    // TODO: Remove if operation to improve performance, or maybe not
    OutLighting.xyz = float3(0.5f,1,1);
    OutLighting.a = 1;

    return OutLighting;
}