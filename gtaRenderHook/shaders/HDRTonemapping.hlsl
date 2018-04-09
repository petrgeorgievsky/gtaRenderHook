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
float LogLuminance(PS_QUAD_IN i) : SV_Target
{
	float4 OutColor;
	float3 LumVec = float3(0.2126, 0.7152, 0.0722);
    float LogLum = log(dot(txInputRaster.Sample(samLinear, i.vTexCoord.xy).rgb, LumVec));
    return LogLum;
}

float4 Tonemap(PS_QUAD_IN i) : SV_Target
{
	float AvgLogLum = txAvgLogLum.Load(int3(0, 0, 0));
    float3 ScreenColor = txInputRaster.Sample(samLinear, i.vTexCoord.xy).rgb;
    float Luminance = GetLuminance(ScreenColor);
    float3 White = float3(1, 1, 1);
    // here i use standard gta color grading colors to do some nice(or bad depends on what you think) color grading
    float3 GradingColor = lerp(lerp(vGradingColor0.rgb, White, 1 - vGradingColor0.a), lerp(vGradingColor1.rgb, White, 1 - vGradingColor1.a), saturate(1-Luminance)) + 0.5f;
    
    //ScreenColor *= GradingColor;
    ScreenColor *= fMiddleGray / (AvgLogLum + 0.001f);
    ScreenColor *= (1.0f + ScreenColor / fLumWhite);
    ScreenColor /= (1.0f + ScreenColor);

    return float4(ScreenColor * GradingColor, 1);
}

/*!
    Downscales image averaging 4 pixels and returns average luminance 
*/
float4 DownScale2x2_Lum(PS_QUAD_IN i) : SV_TARGET
{
    float Avg = 0.0f;
    float4 Color = 0.0f;
    
    for (int y = -1; y < 1; y++)
    {
        for (int x = -1; x < 1; x++)
        {
            // Compute the sum of color values
            Color = txInputRaster.Sample(samLinear, i.vTexCoord.xy, int2(x, y));
                
            Avg += GetLuminance(Color.rgb);
        }
    }
    
    Avg /= 4;
    
    return float4(Avg, Avg, Avg, 1.0f);
}

float4 DownScale3x3(PS_QUAD_IN i) : SV_TARGET
{
    float Avg = 0.0f;
    float4 vColor = 0.0f;
    
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            // Compute the sum of color values
            vColor = txInputRaster.Sample(samLinear, i.vTexCoord.xy, int2(x, y));
                        
            Avg += vColor.r;
        }
    }
    
    // Divide the sum to complete the average
    Avg /= 9;
    
    return float4(Avg, Avg, Avg, 1.0f);
}

float4 DownScale3x3_BrightPass(PS_QUAD_IN i) : SV_TARGET
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

float4 AdaptationPass(PS_QUAD_IN i) : SV_TARGET
{
    float AdaptedLum = txInputRaster.Sample(samLinear, i.vTexCoord.xy).rgb;
    float CurrentLum = txAvgLogLum.Sample(samLinear, float2(0, 0));
    
    // The user's adapted luminance level is simulated by closing the gap between
    // adapted luminance and current luminance by 5% every frame, based on a
    // 60 fps rate. This is not an accurate model of human adaptation, which can
    // take longer than half an hour.
    float NewAdaptation = AdaptedLum + (CurrentLum - AdaptedLum) * (1 - pow(0.95f, 60 * 0.01f));
    return float4(NewAdaptation, NewAdaptation, NewAdaptation, 1.0f);
}