#include "GameMath.hlsl"
#include "Shadows.hlsl"
#include "LightingFunctions.hlsl"
#include "ReflectionFunctions.hlsli"
#include "GBuffer.hlsl"
#include "VoxelGI.hlsl"
#include "Shadows.hlsl"
#include "AtmosphericScatteringFunctions.hlsli"
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
Texture2D txGB0 	: register(t0);
Texture2D txGB1 	: register(t1);
Texture2D txGB2 	: register(t2);
Texture2D txPrevFrame : register(t3);
TextureCube txCubeMap : register(t5);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
struct PSInput_Quad
{
    float4 pos : SV_Position;
    float4 texCoordOut : TEXCOORD0;
};

float3 GetUV(float3 position)
{
	float4 pV = mul(float4(position, 1.0f), mView);
	float4 pVP=mul(pV,mProjection);
	pVP.xy = float2(0.5f, 0.5f) + float2(0.5f, -0.5f) * pVP.xy/ pVP.w;
    return float3(pVP.xy, pV.z / pV.w);
}

float3 GetCameraSpacePos(float3 position)
{
    float4 pV = mul(float4(position, 1.0f), mView);
    return pV.xyz / pV.w;
}
float distanceSquared(float2 a, float2 b)
{
    a -= b;
    return dot(a, a);
}

bool ScreenSpaceRT(float3 startPoint, float3 rayDir, float step, out float2 uv)
{
    float3 currentPos = startPoint + rayDir * step * 0.01;
    float3 clipSpacePos;
    float3 increment = rayDir * step;
    float lastDist = 100000;
    for (int i = 0; i < 32; i++)
    {
        currentPos += increment;
        clipSpacePos = GetUV(currentPos);
        // bound check
        if (clipSpacePos.x > 1 || clipSpacePos.y > 1 || /*clipSpacePos.z >= 1 || */
            clipSpacePos.x < 0 || clipSpacePos.y < 0 /*|| clipSpacePos.z <= 0*/)
        {
            uv = float2(0, 0);
            return false;
        }
        // to check if ray hit something we recalculate position(probably bad way to do this)
        float4 NormalSpec = txGB1.Sample(samLinear, clipSpacePos.xy);
        float ViewZ = DecodeFloatRG(NormalSpec.zw);
        ViewZ = ViewZ <= 0 ? fFarClip : ViewZ;
        float3 restoredPosition = GetUV(DepthToWorldPos(ViewZ, clipSpacePos.xy).xyz);
        float dist = distance(restoredPosition.z, clipSpacePos.z);
        if (dist <= 0.1)
        {
            uv = clipSpacePos.xy;
            return true;
        }
        else
        {
            increment *= 1.05f;
        }
    }
    return false;
}

bool intersectsDepthBuffer(float z, float minZ, float maxZ)
{
    /*
     * Based on how far away from the camera the depth is,
     * adding a bit of extra thickness can help improve some
     * artifacts. Driving this value up too high can cause
     * artifacts of its own.
     */
    float depthScale = min(1.0f, z * 40);
    z += 0.5 + lerp(0.0f, 2.0f, depthScale);
    return (maxZ >= z) && (minZ - 0.5 <= z);
}
float linearDepthTexelFetch(float2 hitPixel)
{
    float4 NormalSpec = txGB1.Sample(samLinear, float2(hitPixel.x / fScreenWidth, 1 - hitPixel.y / fScreenHeight));
    return DecodeFloatRG(NormalSpec.zw);
}
void swap(inout float a, inout float b)
{
    float t = a;
    a = b;
    b = t;
}
float rand_1_05(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}
float rand_1_06(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(22.9898, 32.233) * 2.0)) * 64564.5453));
    return abs(noise.x + noise.y) * 0.5;
}
float rand_1_07(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(56.9898, 85.233) * 2.0)) * 12346.5453));
    return abs(noise.x + noise.y) * 0.5;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ReflectionPassPS(PSInput_Quad input) : SV_Target
{
    float4 NormalSpec = txGB1.Sample(samLinear, input.texCoordOut.xy);
    float ViewZ = DecodeFloatRG(NormalSpec.zw);
    ViewZ = ViewZ <= 0 ? fFarClip : ViewZ;
    float3 cameraSpacePos = DepthToViewPos(ViewZ, input.texCoordOut.xy).xyz;
    float3 worldSpacePos = DepthToWorldPos(ViewZ, input.texCoordOut.xy).xyz;
    float3 NormalsVS = DecodeNormals(NormalSpec.xy);
    NormalsVS = mul(float4(NormalsVS, 0.0f), mViewInv).rgb;
    float2 Parameters = txGB2.Sample(samLinear, input.texCoordOut.xy).xy;
    float Roughness = 1 - Parameters.y;

    const float3 ViewPos = mViewInv[3].xyz;

    float3 ViewDir = normalize(worldSpacePos - ViewPos);
    float3 ReflDir = normalize(reflect(ViewDir, NormalsVS));
    
    // simply return SSR for now, maybe will use DXR someday, but simple cubemap is way more possible 
    float Fallback=0;
    float FresnelCoeff = MicrofacetFresnel(NormalsVS, -ViewDir, Roughness);
    // Jitter rays with IGN to get glossy / diffuse reflection approximation
    // TODO: Add ability to cast more rays and some directionality to it, 
    // e.g. multiply by hemisphere distribution or something like that maybe?
    // Maybe even some different noise than IGN
    const float jitterScale = 5000;
    float3 ScaledWorldPos = worldSpacePos * jitterScale;
    float3 jitter = float3(rand_1_05(input.texCoordOut.xy) - 0.5f,
                           rand_1_06(input.texCoordOut.xy) - 0.5f, 
                           rand_1_07(input.texCoordOut.xy) - 0.5f) * Roughness * 0.5f;
    //ReflDir += jitter;
    float3 SSRColor = SSR(txPrevFrame, txGB1, samLinear, worldSpacePos, ReflDir, Roughness, Fallback);
    
    float3 LightDir = normalize(vSunLightDir.xyz);
    float3 FullScattering;
    float3 ObjectColor = CalculateFogColor(float3(0,0,0), ReflDir, LightDir, 1000, 0, FullScattering);
    
    float3 SkyColor = GetSkyColor(ReflDir, LightDir, FullScattering);
    ReflDir = normalize(reflect(ViewDir, NormalsVS));
    float3 ReflectionFallBack;
#if SAMPLE_CUBEMAP ==1
    ReflDir.x *= -1;
    float4 CubeMap = txCubeMap.SampleLevel(samLinear, ReflDir, Roughness*9.0f);
    ReflectionFallBack = lerp(CubeMap.rgb, SkyColor, 1 - CubeMap.a);
#else
    ReflectionFallBack = SkyColor;
#endif
    float3 FinalReflect;
#if USE_SSR==1
    FinalReflect = lerp(SSRColor, ReflectionFallBack, Fallback);
#else
    FinalReflect = ReflectionFallBack;
#endif
    return float4(isnan(FinalReflect) ? 0.0f.xxx: FinalReflect, 1);
}
