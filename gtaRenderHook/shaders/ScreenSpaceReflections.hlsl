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
Texture2D txPrevFrame : register(t3);
#ifndef SSR_SAMPLE_COUNT
#define SSR_SAMPLE_COUNT 8
#endif
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
float3 SSR(float3 texelPosition, float3 reflectDir,float roughness, out float fallback)
{
    float3 currentRay = 0;

    float3 nuv = 0;
    float L = fSSRStep;
	float fStepScaling = 1.0f;
	fallback=0.0;
    float3 csRayStart = GetCameraSpacePos(texelPosition);
    float3 csRayDir = reflectDir;
   // return csRayStart;
    float2 uv;
    float3 hp;
   // return float3(0, 0, 0);
    //int maxIterations = min(SSRMaxIterations, 128);
    for (int i = 0; i < SSR_SAMPLE_COUNT; i++)
    {
        currentRay = texelPosition + reflectDir * L;

        nuv = GetUV(currentRay);
        if (nuv.x < 0 || nuv.x > 1 || nuv.y < 0 || nuv.y > 1){
			fallback=1.0;
            return float3(0, 0, 0);
		}
        float4 NormalSpec = txGB1.Sample(samLinear, nuv.xy);
        float ViewZ = DecodeFloatRG(NormalSpec.zw);
        float3 newPosition = DepthToWorldPos(ViewZ, nuv.xy).xyz;
        L = length(texelPosition - newPosition);
        if (abs(ViewZ - nuv.z) < 0.000001f)
        {
			fallback=1.0;
            return float3(0, 0, 0);
		}
       /*L+=fStep;
        fStep*=fStepScaling;*/
    }
    float error0 = saturate(max(L - 0.011,0) / 0.044);
    float maxOutScreenRayDist = 0.03;
    float error1 = saturate(max(nuv.x - maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25) *
                   saturate(max(nuv.y - maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25) *
                   saturate(max(1 - nuv.x + maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25) *
                   saturate(max(1 - nuv.y + maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25);
	fallback = 1 - error0 * error1;
    float3 refsample = txPrevFrame.Sample(samLinear, nuv.xy).rgb;
    return refsample; //* error0 * error1;
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
    float2 Parameters = txGB2.Sample(samLinear, input.texCoordOut.xy).xy;
    float Roughness = 1 - Parameters.y;

    const float3 ViewPos = mViewInv[3].xyz;

    float3 ViewDir = normalize(worldSpacePos - ViewPos);
    float3 ReflDir = normalize(reflect(ViewDir, NormalsVS));
    
    // simply return SSR for now, maybe will use DXR someday, but simple cubemap is way more possible 
    float Fallback;
    float FresnelCoeff = MicrofacetFresnel(NormalsVS, -ViewDir, Roughness);
   // float3 raytracedColor = ScreenSpaceRT(cameraSpacePos,)
    return float4(SSR(worldSpacePos, ReflDir, Roughness, Fallback) * (1 - Fallback) * FresnelCoeff, 1);
}
