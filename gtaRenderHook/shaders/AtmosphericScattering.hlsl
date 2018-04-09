#include "GBuffer.hlsl"
#include "AtmosphericScatteringFunctions.hlsli"
#ifndef ATMOPHERIC_SCATTERING
#define ATMOPHERIC_SCATTERING

//****** Atmospheric light scattering. Not physically corrected just plausible to look at scattering

struct PSInput_Quad
{
    float4 pos : SV_Position;
    float4 texCoordOut : TEXCOORD0;
};

Texture2D txScreen : register(t0);
Texture2D txGB1 : register(t1);
//Texture2D txShadow : register(t2);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
SamplerComparisonState samShadow : register(s1);

float4 AtmosphericScatteringPS(PSInput_Quad i) : SV_Target
{
    float4 OutLighting;

    const float3 ViewPos = mViewInv[3].xyz;

    float ViewZ;
    float3 Normals;
    GetNormalsAndDepth(txGB1, samLinear, i.texCoordOut.xy, ViewZ, Normals);

    float4 ScreenColor = txScreen.Sample(samLinear, i.texCoordOut.xy);
    
    float3 WorldPos     = DepthToWorldPos(ViewZ, i.texCoordOut.xy).xyz;
    
    float3 ViewDir  = normalize(WorldPos - ViewPos);
    float3 LightDir = normalize(vSunLightDir.xyz);

    float Height    = ViewPos.z;
    float CosPhi    = max(dot(ViewDir, LightDir), 0.0);
    
    float3 FullScattering;
    float3 ObjectColor = CalculateFogColor(ScreenColor.rgb, ViewDir, LightDir, ViewZ, Height, FullScattering);

    // compute sky color at skydome by blending scattering with sun disk
    float3 SunDiskColor = max(FullScattering, vSunColor.rgb * vSunColor.a);
    float  SunDiskCoeff = 1.0f - min(pow(CosPhi - 0.999f, 16.0f), 1.0f);
    
    float3 SkyColor = lerp(FullScattering, SunDiskColor, SunDiskCoeff);
    
    float SkyMask = ScreenColor.a <= 0;
    if (ScreenColor.a <= 0)
        OutLighting.xyz = SkyColor;
    else
        OutLighting.xyz = ObjectColor;
    OutLighting.a = 1;

    return OutLighting;
}

#endif