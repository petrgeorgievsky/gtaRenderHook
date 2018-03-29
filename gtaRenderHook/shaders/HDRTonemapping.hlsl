#include "Globals.hlsl"
// alghorithm from old DirectXSDK, should rework someday soon
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
Texture2D txInputRaster 	: register(t0);
Texture2D txAvgLogLum 	: register(t1);
SamplerState samLinear : register(s0);

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
// Pixel Shader
//--------------------------------------------------------------------------------------
float LogLuminance(PSInput_Quad input) : SV_Target
{
	float4 OutColor;
	float3 lumVec = float3(0.2126, 0.7152, 0.0722);
    float logLum		= log(dot(txInputRaster.Sample(samLinear, input.texCoordOut.xy).rgb,lumVec));
	return logLum;
}
float3 HUEtoRGB(in float H)
{
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	return saturate(float3(R, G, B));
}
float3 HSLtoRGB(in float3 HSL)
{
	float3 RGB = HUEtoRGB(HSL.x);
	float C = (1 - abs(2 * HSL.z - 1)) * HSL.y;
	return (RGB - 0.5) * C + HSL.z;
}
float3 RGBtoHCV(in float3 RGB)
{
	float Epsilon = 1e-10;
	// Based on work by Sam Hocevar and Emil Persson
	float4 P = (RGB.g < RGB.b) ? float4(RGB.bg, -1.0, 2.0 / 3.0) : float4(RGB.gb, 0.0, -1.0 / 3.0);
	float4 Q = (RGB.r < P.x) ? float4(P.xyw, RGB.r) : float4(RGB.r, P.yzx);
	float C = Q.x - min(Q.w, Q.y);
	float H = abs((Q.w - Q.y) / (6 * C + Epsilon) + Q.z);
	return float3(H, C, Q.x);
}
float3 RGBtoHSL(in float3 RGB)
{
	float Epsilon = 1e-10;
	float3 HCV = RGBtoHCV(RGB);
	float L = HCV.z - HCV.y * 0.5;
	float S = HCV.y / (1 - abs(L * 2 - 1) + Epsilon);
	return float3(HCV.x, S, L);
}
static const float4 LUM_VECTOR = float4(.299, .587, .114, 0);

static const float BRIGHT_THRESHOLD = 0.5f;
float4 Tonemap(PSInput_Quad input) : SV_Target
{
	float3 lumVec = float3(0.2126, 0.7152, 0.0722);
	float middleGray = 0.5;
	float avgLogLum = txAvgLogLum.Load(int3(0, 0, 0));
	float3 rgbColor = txInputRaster.Sample(samLinear, input.texCoordOut.xy).rgb;
    float luminance = dot(LUM_VECTOR.rgb, rgbColor);
    float3 white = float3(1, 1, 1);
    // here i use standard gta color grading colors to do some nice(or bad depends on what you think) color grading
    float3 GradingColor = lerp(lerp(vGradingColor0.rgb, white, 1 - vGradingColor0.a), lerp(vGradingColor1.rgb, white, 1 - vGradingColor1.a), saturate(1-luminance)) + 0.5f;
    rgbColor *= GradingColor;
    rgbColor *= MiddleGray / (avgLogLum + 0.001f);
    rgbColor *= (1.0f + rgbColor / LumWhite);
    rgbColor /= (1.0f + rgbColor);
    
	//float3 scaledLum = (middleGray / avgLum)*rgbColor;
	//float3 hslColor = RGBtoHSL(rgbColor);
	//float3 outColor = HSLtoRGB(float3(hslColor.xy, scaledLum / (1 + scaledLum)));

    return float4(rgbColor * GradingColor, 1);
}


float4 DownScale2x2_Lum(PSInput_Quad Input) : SV_TARGET
{
    float4 vColor = 0.0f;
    float fAvg = 0.0f;
    
    for (int y = -1; y < 1; y++)
    {
        for (int x = -1; x < 1; x++)
        {
            // Compute the sum of color values
            vColor = txInputRaster.Sample(samLinear, Input.texCoordOut.xy, int2(x, y));
                
            fAvg += dot(vColor, LUM_VECTOR);
        }
    }
    
    fAvg /= 4;
    
    return float4(fAvg, fAvg, fAvg, 1.0f);
}
float4 DownScale3x3(PSInput_Quad Input) : SV_TARGET
{
    float fAvg = 0.0f;
    float4 vColor;
    
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            // Compute the sum of color values
            vColor = txInputRaster.Sample(samLinear, Input.texCoordOut.xy, int2(x, y));
                        
            fAvg += vColor.r;
        }
    }
    
    // Divide the sum to complete the average
    fAvg /= 9;
    
    return float4(fAvg, fAvg, fAvg, 1.0f);
}
float4 DownScale3x3_BrightPass(PSInput_Quad Input) : SV_TARGET
{
    float3 vColor = 0.0f;
    float4 vLum = txAvgLogLum.Sample(samLinear, float2(0, 0));
    float fLum = vLum.r;

    vColor = txInputRaster.Sample(samLinear, Input.texCoordOut.xy).rgb;
 
    // Bright pass and tone mapping
    vColor = max(0.0f, vColor - BRIGHT_THRESHOLD);
    vColor *= MiddleGray / (fLum + 0.001f);
    vColor *= (1.0f + vColor / LumWhite);
    vColor /= (1.0f + vColor);
    
    return float4(vColor, 1.0f);
}

float4 AdaptationPass
    (PSInput_Quad Input) : SV_TARGET
{
    float fAdaptedLum = txInputRaster.Sample(samLinear, Input.texCoordOut.xy).rgb;
    float fCurrentLum = txAvgLogLum.Sample(samLinear, float2(0, 0));
    
    // The user's adapted luminance level is simulated by closing the gap between
    // adapted luminance and current luminance by 5% every frame, based on a
    // 60 fps rate. This is not an accurate model of human adaptation, which can
    // take longer than half an hour.
    float fNewAdaptation = fAdaptedLum + (fCurrentLum - fAdaptedLum) * (1 - pow(0.95f, 60 * 0.01f));
    return float4(fNewAdaptation, fNewAdaptation, fNewAdaptation, 1.0f);
}