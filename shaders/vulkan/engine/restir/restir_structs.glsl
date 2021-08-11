/**
 *   This file contains structs used by ReSTIR shaders
 */

// Packed light
struct PackedLight
{
    vec4 p0;
    vec4 p1;
    vec4 p2;
};

// TODO: Move elsewhere
struct AnalyticLight
{
    vec4 posAndRadius;
    vec4 dirAndAttenuation;
    vec4 color;
};

struct TriangleLight
{
    vec3 v0;
    vec3 v1;
    vec3 v2;
    int instanceId;
    float intensity;
};

AnalyticLight UnpackLight(PackedLight l)
{
    AnalyticLight result;

    result.posAndRadius = l.p0;
    result.dirAndAttenuation = l.p1;
    result.color = l.p2;

    return result;
}

TriangleLight UnpackTriLight(PackedLight l)
{
    TriangleLight result;

    result.v0 = l.p0.xyz;
    result.v1 = l.p1.xyz;
    result.v2 = l.p2.xyz;
    result.instanceId = floatBitsToInt(l.p0.w);
    result.intensity = l.p1.w;

    return result;
}

struct Reservoir
{
    float totalWeight;
    float selectedLightWeight;
    int visitedSampleCount;
    int selectedLightId;
};

// TODO: Move elsewhere
struct SurfacePoint
{
    vec3 worldPos;
    vec3 normal;
};