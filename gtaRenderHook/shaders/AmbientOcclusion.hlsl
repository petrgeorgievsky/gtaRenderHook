// Ground-truth ambient occlusion (GTAO): for each pixel, a few screen-space directions ("slices") are searched for the
// highest horizon on both sides, and the cosine-weighted visible arc between the two horizons is integrated against the
// surface normal (Jimenez et al., "Practical Real-Time Strategies for Accurate Indirect Occlusion", 2016). The result is
// then smoothed by an edge-aware blur (AODenoisePS, run horizontally and vertically).
// Output: r = visibility, 1 = unoccluded.
#include "GameMath.hlsl"
#include "GBuffer.hlsl"

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};
// normals+depth
Texture2D txGB1 : register(t0);
// ambient occlusion to denoise (AODenoisePS)
Texture2D txAOInput : register(t1);

#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif

// steps on each side of a slice
#ifndef AO_SAMPLE_COUNT
#define AO_SAMPLE_COUNT 8
#endif
// directions per pixel
#ifndef AO_SLICE_COUNT
#define AO_SLICE_COUNT 2
#endif

static const float AO_PI = 3.14159265f;
static const float AO_HALF_PI = 1.57079633f;

// acos approximation, max error about 1e-4
float AOFastAcos(float x)
{
    float r = (-0.156583f * abs(x) + AO_HALF_PI) * sqrt(1.0f - abs(x));
    return x >= 0 ? r : AO_PI - r;
}

float AOInterleavedGradientNoise(float2 p)
{
    return frac(52.9829189f * frac(dot(p, float2(0.06711056f, 0.00583715f))));
}

// G-buffer texel without filtering: interpolating the encoded depth across edges gives depths that don't exist
void AOLoadNormalsDepth(float2 uv, out float viewZ, out float3 viewNormal)
{
    uint w, h;
    txGB1.GetDimensions(w, h);
    const uint2 texel = min(uint2(uv * float2(w, h)), uint2(w - 1, h - 1));
    const float4 nd = txGB1.Load(int3(texel, 0));
    viewZ = DecodeFloatRG(nd.zw);
    viewZ = viewZ <= 0 ? fFarClip : viewZ;
    viewNormal = DecodeNormals(nd.xy);
}

float3 AOViewPos(float2 uv)
{
    float viewZ;
    float3 n;
    AOLoadNormalsDepth(uv, viewZ, n);
    return DepthToViewPos(viewZ, uv).xyz;
}

float4 AmbientOcclusionPS(PS_QUAD_IN i) : SV_Target
{
    const float2 uv = i.vTexCoord.xy;
    float viewZ;
    float3 N;
    AOLoadNormalsDepth(uv, viewZ, N);
    if (viewZ >= fFarClip * 0.99f || viewZ < 0.05f)
        return float4(1, 1, 1, 1);

    const float3 P = DepthToViewPos(viewZ, uv).xyz;
    const float3 V = normalize(-P);

    // effect radius on screen, in texture coordinates, kept below a quarter of the screen
    float radius = AORadius;
    float2 radiusUV = radius * float2(mProjection[0].x, mProjection[1].y) * 0.5f / viewZ;
    const float shrink = min(1.0f, 0.25f / max(radiusUV.x, radiusUV.y));
    radiusUV *= shrink;
    radius *= shrink;
    if (max(radiusUV.x / AOTexelSize.x, radiusUV.y / AOTexelSize.y) < 1.5f)
        return float4(1, 1, 1, 1); // smaller than a texel: nothing to find

    // samples further than the radius fade out instead of cutting off
    const float falloffRange = 0.615f * radius;
    const float falloffMul = -1.0f / falloffRange;
    const float falloffAdd = (radius - falloffRange) / falloffRange + 1.0f;

    const float2 noisePos = i.vPosition.xy + AOFrame * float2(5.588238f, 7.123456f);
    const float sliceNoise = AOInterleavedGradientNoise(noisePos);
    const float stepNoise = AOInterleavedGradientNoise(noisePos.yx * 1.31f + 17.0f);

    float visibility = 0;
    [unroll]
    for (int slice = 0; slice < AO_SLICE_COUNT; slice++)
    {
        const float phi = (slice + sliceNoise) * AO_PI / AO_SLICE_COUNT;
        const float2 omega = float2(cos(phi), sin(phi));    // screen direction (texture coordinates)

        // view-space direction of omega at this pixel's depth, and the slice plane through it and the view vector
        const float3 dirVec = normalize(DepthToViewPos(viewZ, uv + omega * AOTexelSize).xyz - P);
        const float3 orthoDir = dirVec - dot(dirVec, V) * V;
        const float3 axis = normalize(cross(orthoDir, V));
        const float3 projN = N - axis * dot(N, axis);
        const float projNLen = length(projN);
        const float cosN = saturate(dot(projN, V) / max(projNLen, 1e-4f));
        const float n = sign(dot(orthoDir, projN)) * AOFastAcos(cosN);

        // horizons start at the tangent plane
        const float lowCos0 = cos(n + AO_HALF_PI);
        const float lowCos1 = cos(n - AO_HALF_PI);
        float horizonCos0 = lowCos0, horizonCos1 = lowCos1;

        [loop]
        for (int s = 0; s < AO_SAMPLE_COUNT; s++)
        {
            float t = (s + stepNoise) / AO_SAMPLE_COUNT;
            t *= t; // more samples near the pixel
            const float2 offset = omega * (radiusUV * t + AOTexelSize);
            const float2 uv0 = uv + offset, uv1 = uv - offset;

            if (all(uv0 > 0) && all(uv0 < 1))
            {
                const float3 d0 = AOViewPos(uv0) - P;
                const float l0 = length(d0);
                const float c0 = lerp(lowCos0, dot(d0 / l0, V), saturate(l0 * falloffMul + falloffAdd));
                horizonCos0 = max(horizonCos0, c0);
            }
            if (all(uv1 > 0) && all(uv1 < 1))
            {
                const float3 d1 = AOViewPos(uv1) - P;
                const float l1 = length(d1);
                const float c1 = lerp(lowCos1, dot(d1 / l1, V), saturate(l1 * falloffMul + falloffAdd));
                horizonCos1 = max(horizonCos1, c1);
            }
        }

        float h0 = -AOFastAcos(horizonCos1);
        float h1 = AOFastAcos(horizonCos0);
        h0 = n + clamp(h0 - n, -AO_HALF_PI, AO_HALF_PI);
        h1 = n + clamp(h1 - n, -AO_HALF_PI, AO_HALF_PI);
        const float arc0 = (cosN + 2 * h0 * sin(n) - cos(2 * h0 - n)) * 0.25f;
        const float arc1 = (cosN + 2 * h1 * sin(n) - cos(2 * h1 - n)) * 0.25f;
        visibility += projNLen * (arc0 + arc1);
    }
    visibility = saturate(visibility / AO_SLICE_COUNT);
    const float ao = lerp(1.0f, pow(visibility, AOCurve), saturate(AOIntensity));
    return float4(ao, ao, ao, 1);
}

// Edge-aware blur along AODenoiseDir: neighbours count less the more their depth or normal differs.
float4 AODenoisePS(PS_QUAD_IN i) : SV_Target
{
    const int2 center = int2(i.vPosition.xy);
    uint w, h;
    txAOInput.GetDimensions(w, h);

    float viewZ;
    float3 N;
    AOLoadNormalsDepth(i.vTexCoord.xy, viewZ, N);
    if (viewZ >= fFarClip * 0.99f)
        return float4(1, 1, 1, 1);

    float sum = 0, weightSum = 0;
    [unroll]
    for (int k = -4; k <= 4; k++)
    {
        const int2 p = clamp(center + int2(AODenoiseDir * k), int2(0, 0), int2(w - 1, h - 1));
        float sampleZ;
        float3 sampleN;
        AOLoadNormalsDepth((p + 0.5f) * AOTexelSize, sampleZ, sampleN);
        float weight = exp(-k * k / 12.5f);                                            // gaussian, sigma 2.5 texels
        weight *= saturate(1.0f - abs(sampleZ - viewZ) / (viewZ * 0.03f + 0.05f));      // depth edges
        weight *= pow(saturate(dot(sampleN, N)), 8);                                     // normal edges
        sum += txAOInput.Load(int3(p, 0)).r * weight;
        weightSum += weight;
    }
    const float ao = weightSum > 1e-4f ? sum / weightSum : txAOInput.Load(int3(center, 0)).r;
    return float4(ao, ao, ao, 1);
}
