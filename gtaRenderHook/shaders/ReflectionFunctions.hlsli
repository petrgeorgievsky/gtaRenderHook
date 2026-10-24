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

// The G-buffer is rendered with the TAA jitter. Everything SSR reads from it has to change smoothly with the
// sub-pixel offset: on flat surfaces (car paint) the ray march sits right at its hit threshold, and any read that
// jumps by a whole texel when the jitter changes flips the whole surface between SSR and the fallback.

// view depth at uv; depth is packed into two 8-bit channels, so decode the four texels first and blend after
float SSRSampleDepth(Texture2D NormalDepth, float2 uv)
{
    float2 Size;
    NormalDepth.GetDimensions(Size.x, Size.y);
    float2 Pos = uv * Size - 0.5f;
    int2 Texel = int2(floor(Pos));
    float2 Blend = Pos - Texel;
    float Depth[4];
    [unroll]
    for (int k = 0; k < 4; k++)
    {
        int2 t = clamp(Texel + int2(k & 1, k >> 1), int2(0, 0), int2(Size) - 1);
        float d = DecodeFloatRG(NormalDepth.Load(int3(t, 0)).zw);
        Depth[k] = d <= 0 ? fFarClip : d;
    }
    return lerp(lerp(Depth[0], Depth[1], Blend.x), lerp(Depth[2], Depth[3], Blend.x), Blend.y);
}

// DepthToWorldPos, but with this frame's projection offset (the jitter) taken into account
float3 SSRDepthToWorldPos(float depth, float2 uv)
{
    float2 Ndc = float2(uv.x * 2 - 1, (1 - uv.y) * 2 - 1);
    float2 Ray = float2((Ndc.x - mProjection[2].x) / mProjection[0].x, (Ndc.y - mProjection[2].y) / mProjection[1].y);
    return mul(float4(Ray * depth, depth, 1.0f), mViewInv).xyz;
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
        float3 RestoredWorldPos = SSRDepthToWorldPos(SSRSampleDepth(NormalDepth, ClipSpacePos.xy), ClipSpacePos.xy);

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
    float3 refsample = ScreenTexture.SampleLevel(Sampler, ClipSpacePos.xy + vSSRPrevFrameOffset, 0).rgb;
    return refsample; //* error0 * error1;
}