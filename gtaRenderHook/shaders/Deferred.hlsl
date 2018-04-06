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

Texture2D txShadow 	: register(t3);
Texture2D txLighting : register(t4);
Texture2D txPrevFrame : register(t5);
Texture2D txReflections : register(t6);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
SamplerComparisonState samShadow          : register(s1);
struct Light
{
	float3 Position;
	int m_nLightType;
	float3 m_Color;
	float m_fRange;
};
StructuredBuffer<Light> lightInfo : register(t5);
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


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
// Direction light pass
float4 SunLightingPS(PSInput_Quad input) : SV_Target
{
	float4 OutLighting;
	if (vSunLightDir.w <= 0.0f)
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 AlbedoColor		= txGB0.Sample(samLinear, input.texCoordOut.xy);
	clip(length(AlbedoColor) <= 0);
	
	float4 NormalSpec		= txGB1.Sample(samLinear, input.texCoordOut.xy);
	float3 Normals			= normalize(DecodeNormals(NormalSpec.xy));
	float ViewZ				= DecodeFloatRG(NormalSpec.zw);
	float2 Parameters		= txGB2.Sample(samLinear, input.texCoordOut.xy).xy;
	float Roughness = 1 - Parameters.y;
	float Luminance = dot(AlbedoColor.rgb, float3(0.2126, 0.7152, 0.0722));
	const float3 ViewPos			= ViewInv[3].xyz;
	float3 WorldPos			= posFromDepth(ViewZ, input.texCoordOut.xy).xyz;
	
	float3 ViewDir = normalize(WorldPos.xyz - ViewPos); // View direction vector
	
	float Distance = length(WorldPos.xyz - ViewPos);
	
	float SpecIntensity = Parameters.x;
	float DiffuseTerm, SpecularTerm;
	CalculateDiffuseTerm_ViewDependent(Normals, vSunLightDir.xyz, ViewDir, DiffuseTerm, Roughness);
	CalculateSpecularTerm(Normals, vSunLightDir.xyz, -ViewDir, Roughness, SpecularTerm);
#if SAMPLE_SHADOWS==1
    float ShadowTerm = SampleShadowCascades(txShadow, samShadow, samLinear, WorldPos, Distance) * vSunLightDir.w;
#else
    float ShadowTerm = 1.0;
#endif
    float2 Lighting = float2(DiffuseTerm, SpecularTerm * SpecIntensity) * ShadowTerm;
	OutLighting.xyzw = Lighting.xxxy;
	
	return OutLighting;
}

// point and spot light pass
float4 PointLightingPS(PSInput_Quad input) : SV_Target
{
	float4 OutLighting;

	float4 AlbedoColor = txGB0.Sample(samLinear, input.texCoordOut.xy);
	clip(AlbedoColor.a <= 0);

	float4 NormalSpec = txGB1.Sample(samLinear, input.texCoordOut.xy);
	float3 Normals = normalize(DecodeNormals(NormalSpec.xy));
	float ViewZ = DecodeFloatRG(NormalSpec.zw);
	float2 Parameters = txGB2.Sample(samLinear, input.texCoordOut.xy).xy;

	const float3 ViewPos = ViewInv[3].xyz;
	float3 WorldPos = posFromDepth(ViewZ, input.texCoordOut.xy).xyz;
	
	float3 ViewDir = normalize(WorldPos.xyz - ViewPos); // View direction vector
	
	float Distance = length(WorldPos.xyz - ViewPos);
	float Luminance = dot(AlbedoColor.rgb, float3(0.2126, 0.7152, 0.0722));
	float Roughness = 1 - Parameters.y;
	float SpecIntensity = Parameters.x;
	float3 FinalDiffuseTerm = float3(0, 0, 0);
	float FinalSpecularTerm = 0;
	for (uint i = 0; i < uiLightCount; i++)
	{
		float3 LightDir = -normalize(WorldPos.xyz - lightInfo[i].Position.xyz);
		float LightDistance = length(WorldPos.xyz - lightInfo[i].Position.xyz);
        float DiffuseTerm, SpecularTerm;
		float3 LightColor =  float3(1, 1, 1);
		CalculateDiffuseTerm_ViewDependent(Normals.xyz, LightDir, ViewDir, DiffuseTerm, Roughness);
		CalculateSpecularTerm(Normals.xyz, LightDir, -ViewDir, Roughness, SpecularTerm);
	
		float d = max(LightDistance - lightInfo[i].m_fRange, 0);
		float denom = d / lightInfo[i].m_fRange + 1;
		
		float attenuation = 1.0f - saturate((LightDistance - 0.5f) / lightInfo[i].m_fRange);
		attenuation = pow(attenuation, 2);
		if (lightInfo[i].m_nLightType == 1)
		{
			float fSpot = pow(max(dot(-LightDir, lightInfo[i].m_Color), 0.0f), 2.0f);
			attenuation *= fSpot;
		}
		else
		{
			LightColor = lightInfo[i].m_Color;
		}
			
		FinalDiffuseTerm += DiffuseTerm * attenuation*LightColor;
		FinalSpecularTerm += SpecularTerm * attenuation*SpecIntensity;
	}
	float4 Lighting = float4(FinalDiffuseTerm, FinalSpecularTerm);
	OutLighting.xyzw = Lighting;
	
	return OutLighting;
}

float4 BlitPS(PSInput_Quad input) : SV_Target
{
	return txPrevFrame.Sample(samLinear, input.texCoordOut.xy);
}

float4 FinalPassPS(PSInput_Quad input) : SV_Target
{
	float4 OutLighting;

	float4 AlbedoColor = txGB0.Sample(samLinear, input.texCoordOut.xy);
	float2 Parameters = txGB2.Sample(samLinear, input.texCoordOut.xy).xy;

	const float3 ViewPos = ViewInv[3].xyz;
	
	float Metallness = Parameters.x;

	float4 Lighting = txLighting.Sample(samLinear, input.texCoordOut.xy);
			   
	float3 refColor=lerp(float3(1,1,1), AlbedoColor.rgb, Metallness);
	
	if (AlbedoColor.a <= 0)
	{
		OutLighting.xyz = float3(0, 0, 0);
		OutLighting.a = 0;
	}
	else
	{
		// Diffuse term consists of diffuse lighting, and sky ambient
		float3 diffuseTerm = (Lighting.xyz + vSkyLightCol.rgb * 0.3f);
		// Specular term consists of specular highlights
		float3 specularTerm = (Lighting.w * Lighting.xyz);
		// Reflection term is computed before deferred
		float3 reflectionTerm = txReflections.Sample(samLinear, input.texCoordOut.xy);
		// Add atmospheric scattering to result
        OutLighting.xyz = diffuseTerm * AlbedoColor.rgb + specularTerm + reflectionTerm * Metallness*2.5f;
		OutLighting.a = 1;
	}
	return OutLighting;
}
