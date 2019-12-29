#include "GameMath.hlsl"

// alghorithm from old DirectXSDK, should rework someday soon
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
Texture2D txInputRaster 	: register(t0);
Texture2D txAvgLogLum 	: register(t1);
SamplerState samLinear : register(s0);

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
// Pixel Shader
//--------------------------------------------------------------------------------------
/*
    Calculates natural logarithm of luminance for each pixel
*/
float LogLuminancePS(PS_QUAD_IN i) : SV_Target
{
    float3 ScrenColor = txInputRaster.Sample(samLinear, i.vTexCoord.xy).rgb;
    float LogLum = log(GetLuminance(ScrenColor));
    return LogLum;
}

float GaussianDistribution(float x, float y, float rho)
{
    float g = 1.0f / sqrt(2.0f * 3.14f * rho * rho);
    g *= exp(-(x * x + y * y) / (2 * rho * rho));

    return g;
}

float4 TonemapPS(PS_QUAD_IN i) : SV_Target
{
    float2 g_avSampleOffsets[15];
    float4 g_avSampleWeights[15];
    float tu = 1.0f / (float) fScreenWidth;
    float fDeviation = 3.0f,
          fMultiplier = 1.25f;

    // Fill the center texel
    float weight = 1.0f * GaussianDistribution(0, 0, fDeviation);
    g_avSampleWeights[7] = float4(weight, weight, weight, 1.0f);

    g_avSampleOffsets[7] = 0.0f;

    // Fill one side
    for (int k = 1; k < 8; k++)
    {
        weight = fMultiplier * GaussianDistribution((float) k, 0, fDeviation);
        g_avSampleOffsets[7 - k] = -k * tu;

        g_avSampleWeights[7 - k] = float4(weight, weight, weight, 1.0f);
    }

    // Copy to the other side
    for (int j = 8; j < 15; j++)
    {
        g_avSampleWeights[j] = g_avSampleWeights[14 - j];
        g_avSampleOffsets[j] = -g_avSampleOffsets[14 - j];
    }

	float AvgLogLum = txAvgLogLum.Load(int3(0, 0, 0)).r;
    float3 ScreenColor = txInputRaster.Sample(samLinear, i.vTexCoord.xy).rgb;
    float Luminance = GetLuminance(ScreenColor);
    float3 White = float3(1, 1, 1);    
    ScreenColor *= fMiddleGray / (AvgLogLum + 0.001f);
    ScreenColor *= (1.0f + ScreenColor / fLumWhite);
    ScreenColor /= (1.0f + ScreenColor);

    float4 vSample = 0.0f;
    float4 vColor = 0.0f;
    float2 vSamplePosition;
    
    for (int iSample = 0; iSample < 15; iSample++)
    {
        // Sample from adjacent points
        vSamplePosition = i.vTexCoord.xy + g_avSampleOffsets[iSample];
        vColor = txInputRaster.Sample(samLinear, vSamplePosition);
        vColor *= fMiddleGray / (AvgLogLum + 0.001f);
        vColor *= (1.0f + vColor / fLumWhite);
        vColor /= (1.0f + vColor);

        vSample += g_avSampleWeights[iSample] * vColor;
    }

    //ScreenColor += vSample;

#if USE_GTA_CC==1
    // here i use standard gta color grading colors to do some nice(or bad depends on what you think) color grading
    // TODO: separate from tonemapping perhaps to make more controll
    float3 GradingColor = lerp( lerp(vGradingColor0.rgb, White, 1 - vGradingColor0.a), 
                                lerp(vGradingColor1.rgb, White, 1 - vGradingColor1.a), saturate(1-Luminance)) + 0.5f;
    ScreenColor *= GradingColor;
#endif
    return float4(ScreenColor, 1);
}

/*!
    Downscales image averaging 4 pixels(center pixel and 3 pixels on top and left) and returns average luminance.
    or in kernel terms something like this:
    1 1 0
    1 1 0
    0 0 0
*/
float4 DownScale2x2_LumPS(PS_QUAD_IN i) : SV_TARGET
{
    float Avg = 0.0f;
    float4 Color = 0.0f;

    [unroll]
    for (int y = -1; y < 1; y++)
    {
        [unroll]
        for (int x = -1; x < 1; x++)
        {
            // Compute the sum of color values
            Color = txInputRaster.SampleLevel(samLinear, i.vTexCoord.xy, 0, int2(x, y));
                
            Avg += GetLuminance(Color.rgb);
        }
    }
    
    Avg /= 4;
    
    return float4(Avg, Avg, Avg, 1.0f);
}
/*!
    Downscales image averaging 9 pixels(center pixel and 8 pixels around it) and returns average luminance, simple 3x3 box kernel
*/
float4 DownScale3x3PS(PS_QUAD_IN i) : SV_TARGET
{
    float Avg = 0.0f;
    float4 vColor = 0.0f;

    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            // Compute sum of color values
            vColor = txInputRaster.Sample(samLinear, i.vTexCoord.xy, int2(x, y));
                        
            Avg += vColor.r;
        }
    }
    
    // Divide the sum to complete the average
    Avg /= 9;
    
    return float4(Avg, Avg, Avg, 1.0f);
}
/*!
    Adapts luminance between current and previous frame
*/
float4 AdaptationPassPS(PS_QUAD_IN i) : SV_TARGET
{
    float AdaptedLum = txInputRaster.Sample(samLinear, i.vTexCoord.xy).r;
    float CurrentLum = txAvgLogLum.Sample(samLinear, float2(0, 0)).r;
    
    // The user's adapted luminance level is simulated by closing the gap between
    // adapted luminance and current luminance by 5% every frame, based on a
    // 60 fps rate. This is not an accurate model of human adaptation, which can
    // take longer than half an hour.
    // Going to make this more adjustable, e.g. add rate param
    float NewAdaptation = AdaptedLum + (CurrentLum - AdaptedLum) * (1 - pow(0.95f, 60 * 0.01f));
    return float4(NewAdaptation, NewAdaptation, NewAdaptation, 1.0f);
}

// UNUSED(right now)
float4 DownScale3x3_BrightPassPS(PS_QUAD_IN i) : SV_TARGET
{
    float3 vColor = 0.0f;
    float Luminance = txAvgLogLum.Sample(samLinear, float2(0, 0)).r;

    vColor = txInputRaster.Sample(samLinear, i.vTexCoord.xy).rgb;
 
    // Bright pass and tone mapping
    vColor = max(0.0f, vColor - g_fHDRBrightTreshold);
    vColor *= fMiddleGray / (Luminance + 0.001f);
    vColor *= (1.0f + vColor / fLumWhite);
    vColor /= (1.0f + vColor);
    
    return float4(vColor, 1.0f);
}