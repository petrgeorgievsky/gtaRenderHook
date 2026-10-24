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

// Sample pattern repeating every 4x4 pixels (a Bayer matrix), so the denoiser, whose kernel spans exactly one period,
// averages every pattern value once and leaves no grain (the approach of Intel's XeGTAO).
static const float AOBayer4x4[16] = { 0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5 };
float AOTiledNoise(uint2 pixel, uint2 shift)
{
    const uint2 p = (pixel + shift) & 3u;
    return (AOBayer4x4[p.y * 4 + p.x] + 0.5f) / 16.0f;
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

// ---- Wide-range occlusion ----------------------------------------------------------------------------------------------
// GTAO's horizon search only reaches AORadius and fades out occluders across large depth gaps, so the road under and in
// front of a car gets almost nothing. This term samples like the Serendipity preset's AO, which handles that well:
// - the spiral is centred on the point 0.25 m above the surface (along its normal), which, seen from a low camera,
//   projects onto whatever stands over the receiver (a car's bumper and body);
// - its reach is in screen pixels (up to 224 at close range, shrinking past 20 m), so a nearby car fills the spiral;
// - each tap counts by how far it rises above the receiver's surface, weighted by 1/distance^2 (full within ~3 m).
// Changes from the original: a fixed 4x4 pattern instead of random noise (the denoiser removes it), rises under
// AO_WIDE_BIAS don't count (the bumps of a normal-mapped road turned into grain), and only upward-facing receivers
// (ground, floors) are darkened, so car body panels keep GTAO's detail.
#ifndef AO_WIDE_TAPS
#define AO_WIDE_TAPS 16
#endif
#ifndef AO_WIDE_STRENGTH
#define AO_WIDE_STRENGTH 1.3f
#endif
#ifndef AO_WIDE_BIAS
#define AO_WIDE_BIAS 0.2f
#endif

float AOWideOcclusion(float2 uv, float3 P, float3 N, float viewZ, uint2 pixel)
{
    // world up in view space; full effect on faces within ~45 degrees of up, none on walls
    const float upness = dot(N, normalize(mView[2].xyz));
    const float receiver = saturate((upness - 0.35f) / 0.35f);
    const float fade = smoothstep(0.5f, 2.5f, viewZ) * smoothstep(75.0f, 60.0f, viewZ);
    if (receiver * fade <= 0)
        return 0;

    const float3 C = P + N * 0.25f;
    if (C.z <= 0.05f)
        return 0;
    const float2 centerUV = float2(C.x * mProjection[0].x / C.z * 0.5f + 0.5f, 0.5f - C.y * mProjection[1].y / C.z * 0.5f);
    // 7 px per tap over 32 taps in the original; the same reach with AO_WIDE_TAPS
    const float tapPixels = 7.0f / max(1.0f, viewZ * 0.05f) * (32.0f / AO_WIDE_TAPS);
    const float rotation = AOTiledNoise(pixel, uint2(1, 3)) * 6.2831853f;

    float sum = 0, weight = 0;
    [loop]
    for (int s = 1; s <= AO_WIDE_TAPS; s++)
    {
        const float angle = 10.166406827f * s + rotation;
        const float2 suv = centerUV + float2(cos(angle) / fScreenWidth, sin(angle) / fScreenHeight) * tapPixels * s;
        if (any(suv <= 0) || any(suv >= 1))
        {
            weight += 0.5f;
            continue;
        }
        float sZ;
        float3 sN;
        AOLoadNormalsDepth(suv, sZ, sN);
        const float3 d = DepthToViewPos(sZ, suv).xyz - P;
        const float dist = max(length(d), 1e-3f);
        const float3 u = d / dist;
        const float w = saturate(10.0f / (dist * dist));
        const float elevation = saturate((dot(u, N) - AO_WIDE_BIAS) * 5.0f);
        sum += (0.5f * abs(dot(u, sN)) + 0.5f) * elevation * w;
        weight += w;
    }
    return weight > 0 ? saturate(AO_WIDE_STRENGTH * sum / weight) * receiver * fade : 0;
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

    const uint2 pixel = uint2(i.vPosition.xy);
    const float frameShift = frac(AOFrame * 0.61803399f); // TemporalNoise: shift the pattern every frame
    const float sliceNoise = frac(AOTiledNoise(pixel, uint2(0, 0)) + frameShift);
    const float stepNoise = frac(AOTiledNoise(pixel, uint2(2, 1)) + frameShift);

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
    visibility *= 1.0f - AOWideOcclusion(uv, P, N, viewZ, pixel);
    const float ao = lerp(1.0f, pow(visibility, AOCurve), saturate(AOIntensity));
    return float4(ao, ao, ao, 1);
}

// Edge-aware blur along AODenoiseDir over one noise period: neighbours count less the more their depth or normal differs.
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
    for (int k = -2; k <= 2; k++)
    {
        const int2 p = clamp(center + int2(AODenoiseDir * k), int2(0, 0), int2(w - 1, h - 1));
        float sampleZ;
        float3 sampleN;
        AOLoadNormalsDepth((p + 0.5f) * AOTexelSize, sampleZ, sampleN);
        float weight = abs(k) == 2 ? 0.5f : 1.0f;    // spans exactly one 4-texel noise period
        weight *= saturate(1.0f - abs(sampleZ - viewZ) / (viewZ * 0.03f + 0.05f));      // depth edges
        weight *= pow(saturate(dot(sampleN, N)), 8);                                     // normal edges
        sum += txAOInput.Load(int3(p, 0)).r * weight;
        weightSum += weight;
    }
    const float ao = weightSum > 1e-4f ? sum / weightSum : txAOInput.Load(int3(center, 0)).r;
    return float4(ao, ao, ao, 1);
}
