#include "GameMath.hlsl"
#include "Shadows.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "VoxelGI.hlsl"
#include "Shadows.hlsl"
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
Texture2D txGB0 	: register(t0);
Texture2D txGB1 	: register(t1);
Texture2D txGB2 	: register(t2);
Texture2D txGB3     : register(t3);

Texture2D txShadow 	: register(t4);
Texture2D txLighting : register(t4);
Texture2D txPrevFrame : register(t5);
Texture2D txReflections : register(t6);
Texture2D txAO : register(t7);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
SamplerComparisonState samShadow          : register(s1);
struct Light
{
	float3  vPosition;
	int   nLightType;
	float3  cColor;
	float   fRange;
    float3  vDir;
    float   fPadding;
};
/*!
    Matrices updated each frame
*/
cbuffer LightBuffer : register(b10)
{
    Light aDynamicLights[16];
};
struct VS_QUAD_IN
{
    float4 vPosition : POSITION;
	float2 vTexCoord : TEXCOORD0;
};
struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_QUAD_IN VS(VS_QUAD_IN i)
{
    PS_QUAD_IN o;
    o.vPosition = i.vPosition;
    o.vTexCoord = float4(i.vTexCoord, 0, 1);
	return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
// Direction light pass
float4 SunLightingPS(PS_QUAD_IN i) : SV_Target
{
	float4 OutLighting;

    const float3 ViewPos = mViewInv[3].xyz;

    clip(vSunLightDir.w <= 0.0f);

    float4 AlbedoColor = txGB0.Sample(samLinear, i.vTexCoord.xy);

	clip(length(AlbedoColor) <= 0);

	float3 Normals;
	float ViewZ;
    GetNormalsAndDepth(txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals);
    Normals = normalize(Normals);
    float4 Parameters = txGB2.Sample(samLinear, i.vTexCoord.xy);
    float4 Radiance = txGB3.Sample(samLinear, i.vTexCoord.xy);

	float Roughness = 1 - Parameters.y;
    float SpecIntensity = Parameters.x;

	float Luminance = dot(AlbedoColor.rgb, float3(0.2126, 0.7152, 0.0722));

	
	float3 WorldPos	= DepthToWorldPos(ViewZ, i.vTexCoord.xy).xyz;
	
	float3 ViewDir = normalize(WorldPos.xyz - ViewPos); // View direction vector
	
	float Distance = length(WorldPos.xyz - ViewPos);
	
	
	float DiffuseTerm, SpecularTerm;
	CalculateDiffuseTerm_ViewDependent(Normals, vSunLightDir.xyz, ViewDir, DiffuseTerm, Roughness);
	CalculateSpecularTerm(Normals, vSunLightDir.xyz, -ViewDir, Roughness, SpecularTerm);

#if SAMPLE_SHADOWS==1
    float ShadowTerm = SampleShadowCascades(txShadow, samShadow, samLinear, WorldPos, Distance) * vSunLightDir.w;
#else
    float ShadowTerm = 1.0;
#endif

    float2 Lighting = float2(DiffuseTerm, SpecularTerm * SpecIntensity) * ShadowTerm;
    OutLighting.xyzw = float4(Lighting.x * vSunColor.rgb + Radiance.rgb * saturate(1.0f - vSunLightDir.w + 0.2f), Lighting.y);
	
	return OutLighting;
}

float SpotLightIntensity(float cosA, float umbraAngle, float penumbraAngle)
{
    float t = saturate((cosA - cos(umbraAngle)) / (cos(penumbraAngle) - cos(umbraAngle)));
    return smoothstep(0, 1, t);
}

// point and spot light pass
float4 PointLightingPS(PS_QUAD_IN i) : SV_Target
{
	float4 OutLighting;
    float3 Normals;
    float ViewZ;

    const float3 ViewPos = mViewInv[3].xyz;

    float4 AlbedoColor = txGB0.Sample(samLinear, i.vTexCoord.xy);
	clip(AlbedoColor.a <= 0);
    
    float4 Parameters = txGB2.Sample(samLinear, i.vTexCoord.xy);

    GetNormalsAndDepth(txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals);
    Normals = normalize(Normals);

    float3 WorldPos = DepthToWorldPos(ViewZ, i.vTexCoord.xy).xyz;
	
	float3 ViewDir = normalize(WorldPos.xyz - ViewPos); // View direction vector
	
	float Distance = length(WorldPos.xyz - ViewPos);

	float Luminance = dot(AlbedoColor.rgb, float3(0.2126, 0.7152, 0.0722));
	float Roughness = 1 - Parameters.y;
	float SpecIntensity = Parameters.x;

	float3 FinalDiffuseTerm = float3(0, 0, 0);
	float FinalSpecularTerm = 0;
    //[unroll(128)]
    for (uint i = 0; i < uiLightCount; i++)
	{
        float3 LightDir = -normalize(WorldPos.xyz - aDynamicLights[i].vPosition.xyz);
        float LightDistance = length(WorldPos.xyz - aDynamicLights[i].vPosition.xyz);

        float DiffuseTerm, SpecularTerm;
        float3 LightColor = aDynamicLights[i].cColor;
		CalculateDiffuseTerm_ViewDependent(Normals.xyz, LightDir, ViewDir, DiffuseTerm, Roughness);
		CalculateSpecularTerm(Normals.xyz, LightDir, -ViewDir, Roughness, SpecularTerm);
		
        float Attenuation = 1.0f - pow(saturate(LightDistance / aDynamicLights[i].fRange), 2);
        Attenuation *= Attenuation;
        if (aDynamicLights[i].nLightType == 1)
		{
            float fSpot = SpotLightIntensity(max(dot(-LightDir, aDynamicLights[i].vDir), 0.0f), PI / 3.0f, PI / 6.0f); //pow(, 4.0f);
            Attenuation *= fSpot;
        }
			
        FinalDiffuseTerm += DiffuseTerm * Attenuation * LightColor;
        FinalSpecularTerm += SpecularTerm * Attenuation * SpecIntensity;
    }
	float4 Lighting = float4(FinalDiffuseTerm, FinalSpecularTerm);
	OutLighting.xyzw = Lighting;
	
	return OutLighting;
}

float4 BlitPS(PS_QUAD_IN i) : SV_Target
{
	return txPrevFrame.Sample(samLinear, i.vTexCoord.xy);
}

float4 FinalPassPS(PS_QUAD_IN i) : SV_Target
{
	float4 OutLighting;
    const float3 ViewPos = mViewInv[3].xyz;

    float4 AlbedoColor = txGB0.Sample(samLinear, i.vTexCoord.xy);
    float4 Parameters = txGB2.Sample(samLinear, i.vTexCoord.xy);
    float3 Normals;
    float ViewZ;
    GetNormalsAndDepth(txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals);
    Normals = normalize(Normals);

    float3 WorldPos = DepthToWorldPos(ViewZ, i.vTexCoord.xy).xyz;

    float3 ViewDir = normalize(WorldPos.xyz - ViewPos);
    uint MaterialType = ConvertToMatType(Parameters.w);
	
	float Metallness = Parameters.x;

    float4 Lighting = txLighting.Sample(samLinear, i.vTexCoord.xy);
	
	if (AlbedoColor.a <= 0)
	{
		OutLighting.xyz = float3(0, 0, 0);
		OutLighting.a = 0;
	}
	else
	{
        float AmbientOcclusion = txAO.Sample(samLinear, i.vTexCoord.xy).r;
		// Diffuse term consists of diffuse lighting, and sky ambient
        float3 DiffuseTerm = (Lighting.xyz + vSkyLightCol.rgb * 0.3f * AmbientOcclusion);
		// Specular term consists of specular highlights
        float3 SpecularTerm = (Lighting.w * Lighting.xyz) * AmbientOcclusion;
		// Reflection term is computed before deferred
		float3 ReflectionTerm = txReflections.Sample(samLinear, i.vTexCoord.xy).rgb;
        // Fresnel coeff
        float FresnelCoeff = MicrofacetFresnel(Normals, -ViewDir, Parameters.y);
        // Increase reflection for cars
        if (MaterialType == 1)
        {
            //FresnelCoeff = lerp(1.0f,FresnelCoeff, Parameters.y);
            ReflectionTerm *= 1.5f;
        }
		// Add atmospheric scattering to result
        OutLighting.xyz = DiffuseTerm * AlbedoColor.rgb + SpecularTerm * Parameters.x + ReflectionTerm * FresnelCoeff * Parameters.x;
		OutLighting.a = 1;
	}
	return OutLighting;
}
