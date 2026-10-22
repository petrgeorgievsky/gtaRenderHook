#include "Globals.hlsl"
#ifndef SHADOWS_HLSL
#define SHADOWS_HLSL

#ifndef SHADOW_BLUR_KERNEL
#define SHADOW_BLUR_KERNEL 2
#endif
cbuffer ShadowBuffer : register(b4)
{
    row_major matrix DirLightViewProj[4];
    float4 FadeDistances;
    float FadeDistanceMax;
    int ShadowSize;
    int CascadeCount;
    float _pad;
    float4 ShadowBias;
}

// ---- Soft shadows ----------------------------------------------------------------------------------------------------
// Percentage-closer soft shadows (PCSS): a blocker search finds how far the casters are above the receiver, and the
// penumbra grows with that distance, so shadows are sharp where objects touch the ground and soften further away. The
// result is filtered with hardware-compared taps over a Vogel disk. Sizes are worked out in metres from each cascade's
// light matrix, so the softness matches across cascades; the filter never gets narrower than 1.5 shadow-map texels,
// which keeps sub-texel casters (fence wire, foliage) from aliasing. The disk rotation is fixed per screen pixel, so the
// result is stable from frame to frame (no temporal AA needed to hide it).
#ifndef SHADOW_FILTER_TAPS
#define SHADOW_FILTER_TAPS 16
#endif
#ifndef SHADOW_BLOCKER_TAPS
#define SHADOW_BLOCKER_TAPS 12
#endif

// Metres covered by one shadow-map texel of cascade i (from its orthographic light matrix).
float ShadowTexelWorldSize(int i)
{
    const float ndcPerMetre = length(float3(DirLightViewProj[i]._11, DirLightViewProj[i]._21, DirLightViewProj[i]._31));
    return 2.0f / (ndcPerMetre * ShadowSize);
}

// Metres per unit of cascade i's shadow depth.
float ShadowDepthWorldScale(int i)
{
    return 1.0f / length(float3(DirLightViewProj[i]._13, DirLightViewProj[i]._23, DirLightViewProj[i]._33));
}

// Point index of count on a Vogel (golden-angle) disk of radius 1, rotated by phi.
float2 ShadowVogelDisk(int index, int count, float phi)
{
    const float r = sqrt((index + 0.5f) / count);
    const float theta = index * 2.39996323f + phi;
    return r * float2(cos(theta), sin(theta));
}

// Disk rotation: interleaved gradient noise of the receiver's screen pixel.
float ShadowRotationNoise(float3 worldPos)
{
    const float4 clip = mul(float4(worldPos, 1), mViewProjection);
    const float2 pixel = floor((clip.xy / clip.w * float2(0.5f, -0.5f) + 0.5f) * float2(fScreenWidth, fScreenHeight));
    return 6.2831853f * frac(52.9829189f * frac(dot(pixel, float2(0.06711056f, 0.00583715f))));
}

// Visibility (1 = lit) of light-space position lp in cascade i. softness scales the penumbra.
float SampleShadowCascadeSoft(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp,
                              float3 lp, int i, float phi, float softness)
{
    const float2 atlasTexel = float2(1.0f / (ShadowSize * CascadeCount), 1.0f / ShadowSize);
    const float2 uv = float2((0.5f * lp.x + 0.5f + i) / CascadeCount, 0.5f - 0.5f * lp.y);
    const float receiverZ = lp.z - ShadowBias[i];
#if BLUR_SHADOWS == 1
    const float texelWorld = ShadowTexelWorldSize(i);
    const float minTexels = max(1.5f, fMinShadowBlur / texelWorld);
#if USE_PCS_SHADOWS == 1
    // blocker search over about 0.6 m: average depth of the casters in front of the receiver
    const float searchTexels = clamp(0.6f / texelWorld, 2.0f, 16.0f);
    float blockerSum = 0, blockerCount = 0;
    [unroll]
    for (int b = 0; b < SHADOW_BLOCKER_TAPS; b++)
    {
        const float d = txShadow.SampleLevel(samShadowNonComp, uv + ShadowVogelDisk(b, SHADOW_BLOCKER_TAPS, phi) * searchTexels * atlasTexel, 0).r;
        if (d < receiverZ)
        {
            blockerSum += d;
            blockerCount += 1;
        }
    }
    if (blockerCount == 0)
        return 1.0f;
    // penumbra width per metre of distance to the caster: MaxShadowBlur * 0.1 (0.2 ~ twice the sun's apparent size)
    const float blockerDistance = (receiverZ - blockerSum / blockerCount) * ShadowDepthWorldScale(i);
    const float radiusTexels = clamp(blockerDistance * fMaxShadowBlur * 0.1f * softness / texelWorld, minTexels, 16.0f);
#else
    const float radiusTexels = clamp(fMaxShadowBlur * 0.1f * softness / texelWorld, minTexels, 16.0f);
#endif
    float lit = 0;
    [unroll]
    for (int k = 0; k < SHADOW_FILTER_TAPS; k++)
        lit += txShadow.SampleCmpLevelZero(samShadow, uv + ShadowVogelDisk(k, SHADOW_FILTER_TAPS, phi) * radiusTexels * atlasTexel, receiverZ);
    return lit / SHADOW_FILTER_TAPS;
#else
    return txShadow.SampleCmpLevelZero(samShadow, uv, receiverZ);
#endif
}

// Visibility of a world position: the smallest cascade that contains it, blended into the next cascade over the last
// 10% of its range (no visible seams), and faded out at the end of the last cascade.
float SampleShadowCascadesSoft(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp,
                               float3 worldPos, float viewZ, float softness)
{
    const float4 wp = float4(worldPos, 1);
    const float phi = ShadowRotationNoise(worldPos);
    const float margin = 36.0f / ShadowSize; // filter and blocker-search reach, in light-space units
    const float cascadeStart[4] = { FadeDistances.x, FadeDistances.y, FadeDistances.z, FadeDistances.w };
    const float cascadeEnd[4] = { FadeDistances.y, FadeDistances.z, FadeDistances.w, FadeDistanceMax };

    [loop]
    for (int i = 0; i < CascadeCount; i++)
    {
        const float3 lp = mul(wp, DirLightViewProj[i]).xyz;
        if (any(abs(lp.xy) > 1.0f - margin) || lp.z > 1.0f)
            continue;
        float shadow = SampleShadowCascadeSoft(txShadow, samShadow, samShadowNonComp, lp, i, phi, softness);

        const float blendStart = lerp(cascadeStart[i], cascadeEnd[i], 0.9f);
        const float blend = saturate((viewZ - blendStart) / max(cascadeEnd[i] - blendStart, 1e-3f));
        if (i + 1 < CascadeCount)
        {
            if (blend > 0)
            {
                const float3 lpNext = mul(wp, DirLightViewProj[i + 1]).xyz;
                if (all(abs(lpNext.xy) <= 1.0f - margin))
                    shadow = lerp(shadow, SampleShadowCascadeSoft(txShadow, samShadow, samShadowNonComp, lpNext, i + 1, phi, softness), blend);
            }
        }
        else
            shadow = lerp(shadow, 1.0f, blend);
        return shadow;
    }
    return 1.0f;
}

// Samples shadow texture without any blurring.
float SampleShadowMapUnfiltered(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp, float3 LightViewPos, float2 offset, int i)
{
    float2 ShadowTexCoord = 0.5 * LightViewPos.xy + float2(0.5, 0.5);
    ShadowTexCoord.y = 1.0f - ShadowTexCoord.y;
	// early fail
    if (ShadowTexCoord.x < 0 || ShadowTexCoord.x > 1 || ShadowTexCoord.y < 0 || ShadowTexCoord.y > 1)
        return 1;
	// calculate shadow coordinate(because we use 4 cascades, we need to get 4 texcoord spaces each is half of original space plus offset)
    ShadowTexCoord.x *= 1.0f / CascadeCount;
    ShadowTexCoord += offset;
	
    return txShadow.SampleCmpLevelZero(samShadow, ShadowTexCoord, LightViewPos.z - ShadowBias[i]);
}

// Samples shadow texture for compute shaders.
float SampleShadowMapCompute(Texture2D txShadow, float3 LightViewPos)
{
    LightViewPos = mul(float4(LightViewPos, 1), DirLightViewProj[0]).xyz;
    float3 projectTexCoord;
    projectTexCoord = LightViewPos;
    float2 ShadowTexC = 0.5 * projectTexCoord.xy + float2(0.5, 0.5);
    ShadowTexC.y = 1.0f - ShadowTexC.y;
    float sSample = txShadow.Load(int3(ShadowTexC.x * ShadowSize, ShadowTexC.y * ShadowSize, 0)).r;
    return (LightViewPos.z - sSample < ShadowBias[0] * 8);
}

bool CheckIsOutOfBounds( float3 LightViewPos )
{
    float2 ShadowTexCoord = 0.5 * LightViewPos.xy + float2( 0.5, 0.5 );
    ShadowTexCoord.y      = 1.0f - ShadowTexCoord.y;

    float blur_err =
        ( fMaxShadowBlur * max( SHADOW_BLUR_KERNEL, 3 ) / ShadowSize );
    // early fail
    if ( ShadowTexCoord.x < blur_err ||
         ShadowTexCoord.x > ( 1 - blur_err ) ||
         ShadowTexCoord.y < blur_err ||
         ShadowTexCoord.y > ( 1 - blur_err ) )
        return true;
    return false;
}

// Samples cascade shadow maps, blended
float SampleShadowCascades(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp, float3 worldPos, float viewZ)
{
	// early fail
    if (vSunLightDir.w <= 0)
        return 0.0f;
	// Shift view depth a bit, to avoid some blurring problems
    viewZ = max( viewZ - 0.15f, 0.0 );
    const float Shadow = SampleShadowCascadesSoft(txShadow, samShadow, samShadowNonComp, worldPos, viewZ, 1.0f);
    return max(Shadow, 1 - vSunLightDir.w);
}
// Samples cascade shadow maps, blended 
float SampleShadowCascadesUnfiltered(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp, float3 worldPos, float viewZ)
{
	// early fail
    if (vSunLightDir.w <= 0)
        return 0;
	// TODO: make blend distance customizible, perhaps different for each cascade.
    const float blendDistance = 6;
	// Shift view depth by blend distance, optimization for static blend distance
    viewZ += blendDistance;
    const float LFadeDistances[] =
    {
        FadeDistances.x,
        FadeDistances.y,
        FadeDistances.z,
        FadeDistances.w,
        FadeDistanceMax
    };
    float Shadow = 1;
    float blendFactor;
    float far, near;
    float4 WorldPos = float4(worldPos.xyz, 1);
    int i = dot(float4(CascadeCount > 0, CascadeCount > 1, CascadeCount > 2, CascadeCount > 3),
			  float4(viewZ > FadeDistances.y, viewZ > FadeDistances.z, viewZ > FadeDistances.w, viewZ > FadeDistanceMax));
    float3 LightViewPos = mul(WorldPos, DirLightViewProj[i]).xyz;
    far = FadeDistanceMax;
    float2 shadowTCOffset = float2((float) i / CascadeCount, 0.0f);
    Shadow = SampleShadowMapUnfiltered(txShadow, samShadow, samShadowNonComp, LightViewPos, shadowTCOffset, i);
	/*for (i = 0; i < CascadeCount; i++)
    {
        float3 LightViewPos = mul(WorldPos, DirLightViewProj[i]).xyz;
        far = LFadeDistances[i + 1];
        near = LFadeDistances[i];
		// Cascade blending factor
        blendFactor = min(max(viewZ - near, 0) / blendDistance, 1.0);
        float2 shadowTCOffset = float2((float) i / CascadeCount, 0.0f);
        Shadow = lerp(Shadow, SampleShadowMapUnfiltered(txShadow, samShadow, samShadowNonComp, LightViewPos, shadowTCOffset, i), blendFactor);
    }*/
    blendFactor = min(max(viewZ - far, 0) / blendDistance, 1.0);

    return max(lerp(Shadow, 1.0, blendFactor), 1 - vSunLightDir.w);
}
#endif