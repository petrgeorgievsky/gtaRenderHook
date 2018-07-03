/*
	This file contains functions used to achive atmospheric scattering or fog effect
	
*/
#include "LightingFunctions.hlsl"
#include "GameMath.hlsl"

static const float g_fAtmosphereHeight = 8000.0f;
static const float g_fThickAtmosphereHeight = 1200.0f;

/*!
	Rayleigh density at given height
*/
inline float GetRayleighDensity(float Height)
{
    return exp(-Height / g_fAtmosphereHeight);
}

/*!
	Mie density at given height
*/
inline float GetMieDensity(float Height)
{
    return exp(-Height / g_fThickAtmosphereHeight);
}

float3 CalculateRayleighScattering(float CosSqPlOne, float Height)
{
    // Rayleigh phase function approximation
    float phase = 0.75f * CosSqPlOne;
    // Rayleigh density approximation at current camera height
    float density = GetRayleighDensity(Height);
    // Rayleigh light scattering approximation(very rough)
    return (density * phase).xxx;
}

float3 CalculateMieScattering(float CosPhi, float CosSqPlOne, float Height)
{
    // Assymetry factor for mie phase function
    const float g = -0.85;
    // Henyey-Greenstein mie phase function approximation
    float MiePhase = 1.5f * ((1 - g * g) / (2 + g * g)) * (CosSqPlOne / pow(1 + g * g - 2 * g * CosPhi, 1.5f));
    float MieDensity = GetMieDensity(Height);
    // Mie light scattering approximation(very rough)
    return MieDensity * MiePhase;
}
/*!
    Computes atmospheric scattering color and blends it with screen color.
*/
float3 CalculateFogColor(float3 ScreenColor, float3 ViewDir, float3 LightDir, float ViewDepth, float Height, out float3 FullScattering)
{
    float CosPhi = max(dot(ViewDir, LightDir), 0.0);
    float CosSqPlOne = (1 + CosPhi * CosPhi);

    float MieCosPhi = max(-LightDir.z, 0.0);
    float MieCosSqPlOne = (1 + MieCosPhi * MieCosPhi);
    
    float3 SkyColor = lerp(vHorizonCol.rgb, vSkyLightCol.rgb, min(max(Height + 300.0f, 0.0f) / 600.0f, 1.0f));

    float3 MieScattering = CalculateMieScattering(MieCosPhi, MieCosSqPlOne, Height) * vSunColor.rgb * vSunLightDir.w;
    float3 RayleighScattering = CalculateRayleighScattering(CosSqPlOne, Height) * SkyColor;
    
    float3 SunContribution = min(pow(CosPhi, 8.0f), 1.0f) * vSunColor.rgb * vSunLightDir.w;
    FullScattering = ((RayleighScattering * 0.75f + MieScattering * 0.5f) + SunContribution * 0.5f);
        
    float FogFadeCoeff = saturate(max(ViewDepth - fFogStart, 0) / max(fFogRange,0.001f));
    
    return lerp(ScreenColor, FullScattering, FogFadeCoeff);
}
/*!
    Calculates sky color in specific direction.
*/
float3 GetSkyColor(float3 ViewDir, float3 LightDir, float3 FullScattering)
{
    float CosPhi = max(dot(ViewDir, LightDir), 0.0);

    // Compute sky color at skydome by blending scattering with sun disk
    float3 SunDiskColor = max(FullScattering, vSunColor.rgb * vSunColor.a);
    float SunDiskCoeff = CosPhi>0.9999f?1.0f:0.0f;
    
    return lerp(FullScattering, SunDiskColor, SunDiskCoeff * vSunLightDir.w);
}