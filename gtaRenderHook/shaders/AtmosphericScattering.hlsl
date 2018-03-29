#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "GameMath.hlsl"
#ifndef ATMOPHERIC_SCATTERING
#define ATMOPHERIC_SCATTERING

//****** Atmospheric light scattering. Approximate implementation of http://old.cescg.org/CESCG-2009/papers/PragueCUNI-Elek-Oskar09.pdf (just uses some assumptions)
float RayleighDensity(float height)
{
    return exp(-height / 8000.0f);
}

float IntegrateOverRayleighDensity(float distance)
{
    return -RayleighDensity(distance) / 8000.0f;
}

// Rayleigh transmitance function approximation (scattering(wavelength)*densityIntegral(distance))
float RayleighTransmitance(float dist, float wavelength)
{
    float RayleighColor;
    RayleighColor.r = 1.0 / pow(wavelength, 4);

    return RayleighColor * IntegrateOverRayleighDensity(dist) * 2 * pow(PI, 4);
}

float RayleighSingleScattering(float3 lightPos, float3 viewPos, float3 viewDirection, float wavelength, float scatterDistance)
{
    float3 pos = viewPos;
    float res = 0;
    for (int i = 0; i < 64; i++)
    {
        pos += (scatterDistance / 64) * viewDirection;
        float dist = abs(length(pos - lightPos));
        if (dist > 0.01)
            res += RayleighDensity(pos.z) * exp(-RayleighTransmitance(dist, wavelength));
    }
    return res / 8;
}

float MieDensity(float height)
{
    return exp(-height / 1200.0f);
}

float IntegrateOverMieDensity(float distance)
{
    return -MieDensity(distance) / 1200.0f;
}
// Mie transmitance function approximation (scattering()*densityIntegral(distance))
float MieTransmitance(float dist, float wavelength)
{
    return IntegrateOverMieDensity(dist) * 2 * pow(PI, 4);
}

float MieSingleScattering(float3 lightPos, float3 viewPos, float3 viewDirection, float wavelength, float scatterDistance)
{
    float3 pos = viewPos;
    float res = 0;
    for (int i = 0; i < 64; i++)
    {
        pos += (scatterDistance / 64) * viewDirection;
        res += MieDensity(pos.z) * exp(-MieTransmitance(abs(length(pos - lightPos)), wavelength));
    }
    return res / 64;
}

float3 CalculateRayleighScattering(float3 lightPos, float3 viewPos, float3 viewDirection, float Height, float scatterDistance)
{
    // phi is angle between light and scattering direction.
    float cosPhi = max(dot(viewDirection, normalize(vSunLightDir.xyz)), 0.0);
    float cosSqPlOne = (1 + cosPhi * cosPhi);
    // Rayleigh phase function approximation
    float RayleighPhase = 0.75f * cosSqPlOne;
    // Rayleigh density approximation at current camera height
    float RayleighDensity = exp(-Height / 8000.0f);
    // Rayleigh light scattering approximation(very rough)
    float3 RayleighColor;
    RayleighColor.r = /*RayleighSingleScattering(lightPos, viewPos, viewDirection, 6.115, scatterDistance)1.0f / pow(6.115, 4)*/1.0;
    RayleighColor.g = /*RayleighSingleScattering(lightPos, viewPos, viewDirection, 5.5, scatterDistance)  1.0f / pow(5.5, 4)  */1.0;
    RayleighColor.b = /*RayleighSingleScattering(lightPos, viewPos, viewDirection, 4.5, scatterDistance)  1.0f / pow(4.5, 4)  */1.0;
    RayleighColor = RayleighDensity * RayleighPhase /* 2 * pow(PI, 4)*/;
    return RayleighColor;
}

float3 CalculateMieScattering(float3 lightPos, float3 viewPos, float3 viewDirection, float Height, float scatterDistance)
{
    // Assymetry factor for mie phase function
    const float g = -0.85;
    // phi is angle between light and scattering direction.
    float cosPhi = max(dot(viewDirection, vSunLightDir.xyz), 0.0);
    float cosSqPlOne = (1 + cosPhi * cosPhi);
    // Henyey-Greenstein mie phase function approximation
    float MiePhase = 1.5f * ((1 - g * g) / (2 + g * g)) * (cosSqPlOne / pow(1 + g * g - 2 * g * cosPhi, 1.5f));
     // Mie density approximation at current camera height
    float MieDensity = exp(-Height / 1200.0f);
    // Rayleigh light scattering approximation(very rough)
    float3 MieColor;
    MieColor.r = /* MieSingleScattering(lightPos, viewPos, viewDirection, 6.115, scatterDistance) **/MieDensity * MiePhase;
    MieColor.b = /*MieSingleScattering(lightPos, viewPos, viewDirection, 4.5, scatterDistance) **/MieDensity * MiePhase;
    MieColor.g = /*MieSingleScattering(lightPos, viewPos, viewDirection, 5.5, scatterDistance) **/MieDensity * MiePhase;

    //MieColor *= 4 * PI;
    return MieColor;
}

/*void SunRays()
{

float SunDustShadowing = 0.0;
int SunDustSampleCount = 64;
float distance = length(WorldPos.xyz - ViewPos);
float curDist = 0;
float sdsStep = 50.0f / SunDustSampleCount;

int sdsSamples = 0;
    for (
int i = 0;i <
SunDustSampleCount; i++)
    {
        if (curDist > distance)
            break;
        SunDustShadowing += SampleShadowCascadesUnfiltered(txShadow, samShadow, samLinear, ViewPos + curDist * ViewDir, curDist);
        curDist +=
sdsStep;
        sdsSamples++;
    }
    SunDustShadowing /= (float)
sdsSamples;
}*/

struct PSInput_Quad
{
    float4 pos : SV_Position;
    float4 texCoordOut : TEXCOORD0;
};
Texture2D txScreen : register(t0);
Texture2D txGB1 : register(t1);
//Texture2D txShadow : register(t2);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif
SamplerComparisonState samShadow : register(s1);

// CLOUDS TEST ALGHORITHM, just simplex noise atm, TODO: make it generated automaticly and stuff
float4 permute(float4 x)
{
    return fmod((((x * 34.0) + 1.0) * x), 289.0);
}

float4 taylorInvSqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}
float snoise(float3 v)
{
    const float2 C = float2(1.0 / 6.0, 1.0 / 3.0);
    const float4 D = float4(0.0, 0.5, 1.0, 2.0);

// First corner
    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

// Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

  //  x0 = x0 - 0. + 0.0 * C 
    float3 x1 = x0 - i1 + 1.0 * C.xxx;
    float3 x2 = x0 - i2 + 2.0 * C.xxx;
    float3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
    i = fmod(i, 289.0);
    float4 p = permute(permute(permute(
             i.z + float4(0.0, i1.z, i2.z, 1.0))
           + i.y + float4(0.0, i1.y, i2.y, 1.0))
           + i.x + float4(0.0, i1.x, i2.x, 1.0));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0 / 7.0; // N=7
    float3 ns = n_ * D.wyz - D.xzx;

    float4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,N*N)

    float4 x_ = floor(j * ns.z);
    float4 y_ = floor(j - 7.0 * x_); // mod(j,N)

    float4 x = x_ * ns.x + ns.yyyy;
    float4 y = y_ * ns.x + ns.yyyy;
    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, float4(0.0,0.0,0.0,0.0));

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 p0 = float3(a0.xy, h.x);
    float3 p1 = float3(a0.zw, h.y);
    float3 p2 = float3(a1.xy, h.z);
    float3 p3 = float3(a1.zw, h.w);

//Normalise gradients
    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

// Mix final noise value
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, float4(dot(p0, x0), dot(p1, x1),
                                dot(p2, x2), dot(p3, x3)));
}
const float cloudPlaneStart = 70, cloudPlaneHeight = 10;
float GetCloudDensityAt(float3 pos)
{
    float heightDensity = min((pos.z - (cloudPlaneStart + cloudPlaneHeight / 2.0f)) / cloudPlaneHeight,1.0);
    return /*saturate(snoise(pos / 500.0))*/ heightDensity;
}

// returns distance to plane
bool RayPlaneIntersection(float3 rayOrig, float3 rayDir,float3 planeN,float3 planeP, out float3 hitP)
{
    float denom = dot(planeN, rayDir);
    hitP = float3(0, 0, 0);
    if (abs(denom) > 1e-6)
    {
        float3 p0l0 = planeP - rayOrig;
        float dist = dot(p0l0, planeN) / denom;
        hitP = rayOrig + dist * rayDir;
        return dist >= 0;
    }
    return false;
}

float3 GetCloudColor(float3 raymarchStartPos, float3 raymarchEndPos)
{
    const int maxraymarchSteps = 32;
    float3 currPos = raymarchStartPos;
    float3 step = (raymarchEndPos - raymarchStartPos) / (float)maxraymarchSteps;
    //float3 LightPos = normalize(vSunLightDir.xyz) * 8000 + viewPos;
    float3 cloudColor = float3(0,0,0);
    float T = 1,deltaT;
    for (int i = 0; i < maxraymarchSteps; i++)
    {
        deltaT = MieDensity(currPos.z * length(step));
        T *= deltaT;
        if (T < 1e-6)
            break;
        
        cloudColor += GetCloudDensityAt(currPos); //* (1 - deltaT) / (1.0f / 1200.0f) * T;
        currPos += step;
    }
    return cloudColor / (float)maxraymarchSteps;
}

//

float4 AtmosphericScatteringPS(PSInput_Quad input) : SV_Target
{
    float4 OutLighting;

    float4 AlbedoColor = txScreen.Sample(samLinear, input.texCoordOut.xy);
    
    float4 NormalSpec = txGB1.Sample(samLinear, input.texCoordOut.xy);
    float ViewZ = DecodeFloatRG(NormalSpec.zw);
    float skyMask = ViewZ <= 0;
    ViewZ = ViewZ <= 0 ? fFarClip : ViewZ;
	

    const float3 ViewPos = ViewInv[3].xyz;
    float3 WorldPos = posFromDepth(ViewZ, input.texCoordOut.xy).xyz;
    
    float3 ViewDir = normalize(WorldPos - ViewPos);
    
    float cosPhi = max(dot(ViewDir, normalize(vSunLightDir.xyz)), 0.0);
    
    float3 LightPos = normalize(vSunLightDir.xyz) * 8000 + ViewPos;
    float3 MieScattering = CalculateMieScattering(LightPos, ViewPos, float3(0, 0, -1), ViewPos.z, ViewZ) * vHorizonCol.rgb;
    float3 RayleighScattering = CalculateRayleighScattering(LightPos, ViewPos, ViewDir, ViewPos.z, ViewZ) * vSkyLightCol.rgb;
    
    float3 SunContribution = saturate(pow(max(dot(normalize(vSunLightDir.xyz), ViewDir), 0), 8)) * vSunColor.rgb * vSunLightDir.w;
    float3 FullScattering = ((RayleighScattering + MieScattering * 0.75f) + SunContribution);

    
    /*bool traceClouds=false; //= WorldPos.z >= cloudPlaneStart;
    float3 cloudPlaneStartP = float3(0, 0, cloudPlaneStart),cloudPlaneStartN=float3(0, 0, 1);
    float3 cloudPlaneEndP = float3(0, 0, cloudPlaneStart + cloudPlaneHeight), cloudPlaneEndN = float3(0, 0, 1);
    float3 bottomPoint= float3(WorldPos.xy, cloudPlaneStart);
    float3 topPoint= float3(WorldPos.xy + ViewDir.xy * cloudPlaneHeight, cloudPlaneStart + cloudPlaneHeight);*/
    //bool bottomPlaneIntersection = RayPlaneIntersection(ViewPos, -ViewDir, cloudPlaneStartN, cloudPlaneStartP, bottomPoint),
    //     upperPlaneIntersection = RayPlaneIntersection(ViewPos, -ViewDir, cloudPlaneEndN, cloudPlaneEndP, topPoint);
    
    /*float3 raymarchStartPoint, raymarchEndPoint;

    if (ViewPos.z > cloudPlaneStartP.z && ViewPos.z < cloudPlaneEndP.z)
    {
        raymarchStartPoint = ViewPos;
        raymarchEndPoint = ViewPos + ViewDir*10;
        traceClouds = true;

    }
    else
    {

        float toBottomDist = length(ViewPos - bottomPoint) * sign(dot(ViewPos - bottomPoint, ViewDir)),
              toTopDist = length(ViewPos - topPoint) * sign(dot(ViewPos - bottomPoint, ViewDir));
        if (toBottomDist<=0&&toTopDist<=0)
        {
            traceClouds = false;
        }
        else
        {
            traceClouds = true;
            if (toBottomDist <= 0)
            {
                raymarchStartPoint = ViewPos;
                raymarchEndPoint = topPoint;
            }
            else if (toTopDist <= 0)
            {
                raymarchStartPoint = ViewPos;
                raymarchEndPoint = bottomPoint;
            }
            else if (toBottomDist < toTopDist)
            {
                raymarchStartPoint = bottomPoint;
                raymarchEndPoint = topPoint;
            }
            else
            {
                raymarchStartPoint = topPoint;
                raymarchEndPoint = bottomPoint;
            }

        }

    }*/
    //if (traceClouds)
    //OutLighting.rgb = saturate(WorldPos.z/1000.0f); // GetCloudColor(ViewPos, (distance(ViewPos, topPoint) < distance(ViewPos, bottomPoint)) ? topPoint : bottomPoint);
    //}
    //
    

    OutLighting.a = 1;
    if (AlbedoColor.a <= 0)
    {
        OutLighting.xyz = lerp(FullScattering, max(FullScattering, vSunColor.rgb * vSunColor.a), 1 - saturate(pow(cosPhi - 0.999, 16)));
    }
    else
    {
        OutLighting.xyz = lerp(AlbedoColor.rgb, FullScattering, saturate(max(ViewZ - fFogStart, 0) / abs(fFogRange)));
    }
    return OutLighting;
}

#endif