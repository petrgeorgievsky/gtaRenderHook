#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#ifndef SSR_SAMPLE_COUNT
#define SSR_SAMPLE_COUNT 8
#endif

float3 WorldToClipPos(float3 position)
{
    float4 pV = mul(float4(position, 1.0f), mView);
    float4 pVP = mul(pV, mProjection);
    pVP.xy = float2(0.5f, 0.5f) + float2(0.5f, -0.5f) * pVP.xy / pVP.w;
    return float3(pVP.xy, pV.z / pV.w);
}

float3 SSR(Texture2D ScreenTexture, Texture2D NormalDepth,
            SamplerState Sampler, float3 WorldPos, float3 ReflectDir, float Roughness, out float Fallback)
{
    float3 HitPointWorldPos = 0;
    float3 ClipSpacePos = 0;
    float L = fSSRStep;
    float StepScaling = 1.0f;
    Fallback = 0.0;
    float found = 1.0f;
    for (int i = 0; i < SSR_SAMPLE_COUNT; i++)
    {
        HitPointWorldPos = WorldPos + ReflectDir * L;

        ClipSpacePos = WorldToClipPos(HitPointWorldPos);
        if (ClipSpacePos.x < 0 || ClipSpacePos.x > 1 || ClipSpacePos.y < 0 || ClipSpacePos.y > 1)
        {
            Fallback = 1.0;
            return float3(0, 0, 0);
        }
        float ViewZ;
        float3 Normal;
        GetNormalsAndDepth(NormalDepth, Sampler, ClipSpacePos.xy, ViewZ, Normal);

        float3 RestoredWorldPos = DepthToWorldPos(ViewZ, ClipSpacePos.xy).xyz;

        L = length(WorldPos - RestoredWorldPos);
        if (L < fSSRStep * 1.5f)
        {
            found = 0.0f;
            break;
        }
       /* if (abs(ViewZ - ClipSpacePos.z) < 0.000001f)
        {
            Fallback = 1.0;
            return float3(0, 0, 0);
        }*/
        /*L+=fStep;
        fStep*=fStepScaling;*/
    }
    float Error0 = saturate(max(L - 0.011, 0) / 0.044);
    float ScreenEdgeBlend = 0.03;
    float Error1 = saturate(max(ClipSpacePos.x - ScreenEdgeBlend, 0) / ScreenEdgeBlend * 0.25) *
                   saturate(max(ClipSpacePos.y - ScreenEdgeBlend, 0) / ScreenEdgeBlend * 0.25) *
                   saturate(max(1 - ClipSpacePos.x + ScreenEdgeBlend, 0) / ScreenEdgeBlend * 0.25) *
                   saturate(max(1 - ClipSpacePos.y + ScreenEdgeBlend, 0) / ScreenEdgeBlend * 0.25);
    Fallback = 1 - Error0 * Error1 * found;
    float3 refsample = ScreenTexture.SampleLevel(Sampler, ClipSpacePos.xy, 0).rgb;
    return refsample; //* error0 * error1;
}