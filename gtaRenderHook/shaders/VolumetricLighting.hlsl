#include "AtmosphericScatteringFunctions.hlsli"
#include "GBuffer.hlsl"
#include "GameMath.hlsl"
#include "Shadows.hlsl"
Texture2D txScreen : register( t0 );
Texture2D txGB1 : register( t1 );
Texture2D txVolumetric : register( t2 );
Texture2D txShadow : register( t3 );
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register( s0 );
#endif
SamplerComparisonState samShadow : register( s1 );
#ifndef SUNLIGHT_RM_STEPS
#define SUNLIGHT_RM_STEPS 16
#endif

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};
/*!
    Calculates sun-light volumetric scattering
    TODO: improve this alghorithm quality and performance, by introducing
   jittering or maybe something more
*/
float3 ComputeSunRays( float3 ViewPos, float3 WorldPos, float Length )
{
    const int SunRaySampleCount = SUNLIGHT_RM_STEPS;
    float3    Step =
        normalize( WorldPos - ViewPos ) * Length / (float)SunRaySampleCount;
    float3 CurrentPos  = ViewPos;
    float3 ResultColor = float3( 0, 0, 0 );
    for ( int i = 0; i < SunRaySampleCount; i++ )
    {
        ResultColor += SampleShadowCascadesUnfiltered(
            txShadow, samShadow, samLinear, CurrentPos,
            ( Length / (float)SunRaySampleCount ) * i );
        CurrentPos += Step;
    }
    return ResultColor / (float)SunRaySampleCount;
}

/*!
    Calculates sun-light volumetric scattering.
*/
float3 ComputeSunRaysWithLighting( PS_QUAD_IN i, float3 LightDir,
                                   float3 ViewPos, float3 WorldPos,
                                   float Length )
{
    const int SunRaySampleCount = SUNLIGHT_RM_STEPS;
    float3    ViewDir           = normalize( WorldPos - ViewPos );
    float     StepLength        = Length / (float)SunRaySampleCount;

    float3 Step   = ViewDir * StepLength;
    float  Jitter = Bayer4x4( i.vPosition.xy );

    float3 CurrentPos  = ViewPos + Step * Jitter;
    float3 ResultColor = 0.0;

    float SunBlend = saturate( dot( ViewDir, LightDir ) + SunlightBlendOffset );
    SunBlend       = SunBlend * SunBlend * 2.0;

    float3       SunRayContrib = SunBlend * vSunColor.rgb * vSunColor.a * 0.5f;
    const float3 RayleighContrib = vSkyLightCol.rgb * 0.25f;
    const float3 MieContrib      = vHorizonCol.rgb * 0.25f;

    for ( int index = 0; index < SunRaySampleCount; index++ )
    {
        float3 CurrentRay = SampleShadowCascadesUnfiltered(
            txShadow, samShadow, samLinear, CurrentPos,
            StepLength * ( index + Jitter ) );

        ResultColor += CurrentRay *
                       ( SunRayContrib +
                         RayleighContrib * GetRayleighDensity( CurrentPos.z ) +
                         GetMieDensity( CurrentPos.z ) * MieContrib );

        CurrentPos += Step;
    }
    return ResultColor / (float)SunRaySampleCount;
}

float4 VolumetricSunlightPS( PS_QUAD_IN i ) : SV_TARGET
{
    float4 OutLighting;

    const float3 ViewPos = mViewInv[3].xyz;

    // Retrieve all needed buffer samples first
    float  ViewZ;
    float3 Normals;
    GetNormalsAndDepth( txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals );

    float3 WorldPos = DepthToWorldPos( ViewZ, i.vTexCoord.xy ).xyz;

    // Directions calculation maybe we should introduce macroses to encapsulate
    // them
    float3 ViewDir  = normalize( WorldPos - ViewPos );
    float3 LightDir = normalize( vSunLightDir.xyz );

    float SunRaysIntensity =
        min( max( dot( ViewDir, LightDir ) + SunlightBlendOffset, 0.0f ),
             1.0f ) *
        SunlightIntensity;

    float3 SunRays = ComputeSunRaysWithLighting(
        i, LightDir, ViewPos, WorldPos,
        min( length( WorldPos - ViewPos ), RaymarchingDistance ) );

    OutLighting.rgb = SunRays * SunlightIntensity;
    OutLighting.a = 1;

    return OutLighting;
}

float4 VolumetricCombinePS( PS_QUAD_IN i ) : SV_TARGET
{
    float4 OutLighting;

    float4 ScreenColor = txScreen.Sample( samLinear, i.vTexCoord.xy );
    float  intesity    = saturate(
        length( txVolumetric.Sample( samLinear, i.vTexCoord.xy ).rgb ) );
    OutLighting.rgb =
        lerp( ScreenColor.rgb,
              txVolumetric.Sample( samLinear, i.vTexCoord.xy ).rgb, intesity );
    OutLighting.a = 1;

    return OutLighting;
}