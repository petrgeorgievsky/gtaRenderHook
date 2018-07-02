#include "GBuffer.hlsl"
#include "Shadows.hlsl"
#include "AtmosphericScatteringFunctions.hlsli"
#ifndef ATMOPHERIC_SCATTERING
#define ATMOPHERIC_SCATTERING

//****** Atmospheric light scattering. Not physically corrected just plausible to look at scattering

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};

Texture2D txScreen : register(t0);
Texture2D txGB1 : register(t1);
Texture2D txShadow : register(t3);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
SamplerComparisonState samShadow : register(s1);
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

float4 AtmosphericScatteringPS(PS_QUAD_IN i) : SV_Target
{
    float4 OutLighting;

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

    // Some coefficients used in fog computation
    float Height = WorldPos.z;
    
    float3 FullScattering;
    float3 ObjectColor = CalculateFogColor(ScreenColor.rgb, ViewDir, LightDir, ViewZ, Height, FullScattering);
    
    float3 SkyColor = GetSkyColor(ViewDir, LightDir, FullScattering);
    
    // TODO: Remove if operation to improve performance, or maybe not
    if (ScreenColor.a <= 0)
        OutLighting.xyz = SkyColor;
    else
        OutLighting.xyz = ObjectColor;
    OutLighting.a = 1;

    return OutLighting;
}

#endif