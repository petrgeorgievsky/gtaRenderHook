/**
 *   This file contains structs used by ReSTIR shaders
 */

// TODO: Move elsewhere
struct AnalyticLight
{
    vec4 posAndRadius;
    vec4 dirAndAttenuation;
    vec4 color;
};

struct Reservoir
{
    float totalWeight;
    int visitedSampleCount;
    float selectedLightWeight;
    int selectedLightId;
};

// TODO: Move elsewhere
struct SurfacePoint
{
    vec3 worldPos;
    vec3 normal;
};