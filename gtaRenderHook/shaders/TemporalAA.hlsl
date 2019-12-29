#include "GameMath.hlsl"
#include "GBuffer.hlsl"
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
Texture2D txScreen : register(t0);
Texture2D txAccumBuffer : register(t1);
Texture2D txGB1 : register(t2);
Texture2D txGB2 : register(t3);

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};

float3 RGB2YCbCr(float3 rgb)
{
    float3 RGB2Y = { 0.29900, 0.58700, 0.11400 };
    float3 RGB2Cb = { -0.16874, -0.33126, 0.50000 };
    float3 RGB2Cr = { 0.50000, -0.41869, -0.081 };

    return float3(dot(rgb, RGB2Y), dot(rgb, RGB2Cb), dot(rgb, RGB2Cr));
}

float3 YCbCr2RGB(float3 ycc)
{
    float3 YCbCr2R = { 1.0, 0.00000, 1.40200 };
    float3 YCbCr2G = { 1.0, -0.34414, -0.71414 };
    float3 YCbCr2B = { 1.0, 1.77200, 1.40200 };

    return float3(dot(ycc, YCbCr2R), dot(ycc, YCbCr2G), dot(ycc, YCbCr2B));
}

float4 TAA_PS(PS_QUAD_IN i) : SV_TARGET
{
    float2 Offset = float2(1.0f / fScreenWidth, 1.0f / fScreenHeight);
    float2 NeighborPattern[8] = {   float2(0, 1), float2(1, 0), float2(0, -1), float2(-1, 0),
                                    float2(1, 1), float2(-1, -1), float2(1, -1), float2(-1, 1) };
    float4 ScreenColor = txScreen.Sample(samLinear, i.vTexCoord.xy);
    float3 Normals;
    float ViewZ;

    GetNormalsAndDepth(txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals);
	
    float3 WorldPos = DepthToWorldPos(ViewZ, i.vTexCoord.xy).xyz;
    float4 ResultColor = ScreenColor;
    float4 ViewPos = mul(float4(WorldPos, 1), mPrevView);
    float4 PrevTC = mul(ViewPos, mProjection);
    float2 PrevTexCoords = float2(0.5f, 0.5f) + float2(0.5f, -0.5f) * (PrevTC.xy / max(PrevTC.w,0.001f));
    float2 MotionVec = PrevTexCoords - i.vTexCoord.xy;

    float coeff = (PrevTexCoords.x > 1.0 || PrevTexCoords.y > 1.0 
                || PrevTexCoords.x < 0.0 || PrevTexCoords.y < 0.0) ? 0.0 : TAABlendFactor;
    float4 AccamulatedColor = txAccumBuffer.Sample(samLinear, PrevTexCoords) ;
    float4 MatParams = txGB2.Sample(samLinear, i.vTexCoord.xy);
    if (coeff > 0.0)
    {
        float3 minRGB = ScreenColor.rgb;
        float3 maxRGB = ScreenColor.rgb;
        float3 NeighborSample; //= RGB2YCbCr(AccamulatedColor.rgb);
        for (int k = 0; k < 4; k++)
        {
            NeighborSample = txScreen.Sample(samLinear, i.vTexCoord.xy + NeighborPattern[k] * Offset).rgb;
            minRGB = min(minRGB, NeighborSample);
            maxRGB = max(maxRGB, NeighborSample);
        }
        
        //
        /*if ((AccamulatedColor.r < minRGB.r || AccamulatedColor.g < minRGB.g || AccamulatedColor.b < minRGB.b 
            || AccamulatedColor.r > maxRGB.r || AccamulatedColor.g > maxRGB.g || AccamulatedColor.b > maxRGB.b))
            coeff = 0.0f;*/
        //NeighborSample.r = clamp(NeighborSample.r, minY, maxY);
        AccamulatedColor.rgb = clamp(AccamulatedColor.rgb, minRGB, maxRGB);
       /* if (length(NeighborSample - ScreenColor.xyz) > 0.3)
            coeff = 0.5f;*/
    }
    float DistToScreenCenter = max(distance(float2(0.5, 0.5), i.vTexCoord.xy) * MBEdgeScale, MBCenterScale);
    float MinMBPixelCount = MBMaxSpeed;
    MotionVec = max(MotionVec, Offset * -MinMBPixelCount);
    MotionVec = min(MotionVec, Offset * MinMBPixelCount);
    MotionVec *= DistToScreenCenter;

    float Speed = length(MotionVec);
    float MotionBlurBlend = min(max(Speed - length(Offset), 0.0f) / 8.0f, 1.0f);
    ResultColor.rgb = (ScreenColor.rgb * (1 - coeff) + AccamulatedColor.rgb * coeff);
    if (Speed >= length(Offset) * MBMinPixelDistance)
    {
        float3 MotionBlur = float3(0, 0, 0);
        float Jitter = InterleavedGradientNoise(i.vPosition.xy);
        float MBCoeff = 0.0f;
        float2 MBTexCoord;
        const float ExpDistr[8] = { exp(-1), exp(-2), exp(-3), exp(-4), exp(-5), exp(-6), exp(-7), exp(-8) };
        for (int k = 1; k < 8; k++)
        {
            MBTexCoord = i.vTexCoord.xy + MotionVec * ((k + Jitter) / 8.0f);
            if (!(MBTexCoord.x > 1.0 || MBTexCoord.y > 1.0
                || MBTexCoord.x < 0.0 || MBTexCoord.y < 0.0))
            {
                MotionBlur += txScreen.SampleLevel(samLinear, MBTexCoord, 0).rgb * ExpDistr[k-1];
                MBCoeff += ExpDistr[k - 1];
            }
        }
        MotionBlur /= max(MBCoeff,0.01f);
        if (MBCoeff > 0.0f)
        {
            ResultColor.rgb += MotionBlur;
            ResultColor.rgb *= 0.5f;
        }
    }
    return ResultColor;
}