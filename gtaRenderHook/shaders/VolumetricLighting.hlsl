#include "Shadows.hlsl"
#include "GBuffer.hlsl"
#include "GameMath.hlsl"
Texture2D txScreen : register(t0);
Texture2D txGB1 : register(t1);
Texture2D txShadow : register(t3);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
SamplerComparisonState samShadow : register(s1);

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};

float3 ComputeSunRays(float3 ViewPos, float3 WorldPos, float Length)
{
    const int SunRaySampleCount = 64;
    float3 Step = normalize(WorldPos - ViewPos) * Length / (float) SunRaySampleCount;
    float3 CurrentPos = ViewPos;
    float3 ResultColor = float3(0, 0, 0);
    for (int i = 0; i < SunRaySampleCount; i++)
    {
        ResultColor += SampleShadowCascadesUnfiltered(txShadow, samShadow, samLinear, CurrentPos, (Length / (float) SunRaySampleCount) * i);
        CurrentPos += Step;
    }
    return ResultColor / (float) SunRaySampleCount;
}

float4 VolumetricSunlightPS(PS_QUAD_IN i) : SV_TARGET
{
    float4 OutLighting;
    const float SunRaysMaxDistance = 40.0f;
    const float SunRaysColorIntensity = 0.25f;
    const float SunRaysBlendOffset = 0.2f;

    const float3 ViewPos = mViewInv[3].xyz;

    // Retrieve all needed buffer samples first
    float ViewZ;
    float3 Normals;
    GetNormalsAndDepth(txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals);

    float4 ScreenColor = txScreen.Sample(samLinear, i.vTexCoord.xy);
    
    float3 WorldPos = DepthToWorldPos(ViewZ, i.vTexCoord.xy).xyz;
    
    // Directions calculation maybe we should introduce macroses to encapsulate them
    float3 ViewDir = normalize(WorldPos - ViewPos);
    float3 LightDir = normalize(vSunLightDir.xyz);

    float SunRaysIntensity = min(max(dot(ViewDir, LightDir) + SunRaysBlendOffset, 0.0f), 1.0f) * SunRaysColorIntensity;

    float3 SunRays = ComputeSunRays(ViewPos, WorldPos, min(length(WorldPos - ViewPos), SunRaysMaxDistance)) * SunRaysIntensity * SunRaysIntensity;

    OutLighting.rgb = ScreenColor.rgb + SunRays * vSunColor.rgb * vSunColor.a;
    OutLighting.a = 1;

    return OutLighting;
}