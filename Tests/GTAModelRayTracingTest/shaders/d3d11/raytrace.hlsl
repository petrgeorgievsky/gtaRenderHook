struct PayLoad
{
    float3 normal;
    float hitDist;
    float4 color;
    float4 hitPosition;
};

#include "raytracing/rt_structs.hlsli"


static const float3 skyColor = float3(0.7f, 0.9f, 1.0f);
static const float3 botColor = float3(0.5f, 0.7f, 1.0f);


float3 getSkyColor(Ray r){
    float dir = max(dot(r.direction, float3(0,0,1)),0.0f);
    return lerp(botColor, skyColor, dir );
}

inline void INIT_PAYLOAD_MISS(inout PayLoad payLoad, in Ray r, in float maxDist)
{
    payLoad.hitDist = maxDist;
    payLoad.color.rgb = getSkyColor(r);
    payLoad.color.a = 1;
    payLoad.normal = float3(0,0,0);
    payLoad.hitPosition = r.origin + r.direction*maxDist;
}

inline void INIT_PAYLOAD_HIT_TRIANGLE(inout PayLoad payLoad, float u, float v)
{
    payLoad.color = float4(1,1,1,1);
}

StructuredBuffer<Triangle> triangleBuffer : register(t0);
StructuredBuffer<float4> bufVertexList : register(t1);
StructuredBuffer<BLASLeaf> bottomLevelAS : register(t2);
StructuredBuffer<TLASLeaf> topLevelAS : register(t3);

inline float3 GET_TRIANGLE_POS(uint id)
{
    return bufVertexList[id].xyz;
}

#include "raytracing/triangle_intersection.hlsl"
#include "raytracing/aabb_intersection.hlsl"
#include "raytracing/bvh_traverse.hlsl"

RWTexture2D<float4> tResult : register(u0);

cbuffer SceneConstants : register( b0 )
{
    float4x4 mView;
    float4x4 mProjection;
    float4x4 mViewProjection;
    float4x4 mInvViewProjection;
    float4 vViewDir;
    float4 vViewPos;
    float4 deltas;
    float4 padd_[2];
};

//Texture2D gb0 : register(t5);
//Texture2D gb1 : register(t6);
SamplerState s0 : register(s0);


float rand_1_05(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}

float rand_1_06(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(22.9898, 32.233) * 2.0)) * 64564.5453));
    return abs(noise.x + noise.y) * 0.5;
}

float rand_1_07(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(56.9898, 85.233) * 2.0)) * 12346.5453));
    return abs(noise.x + noise.y) * 0.5;
}

float3 randomPointOnUnitSphere(float2 seed)
{
    float3 randomPointOnSphere;
    float theta = 2 * 3.14f * rand_1_05(seed.xy);
    float phi = acos(1 - 2 * rand_1_06(seed.xy));
    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);
    randomPointOnSphere = normalize(float3(x, y, z));
    return randomPointOnSphere;
}

inline Ray GenerateCameraRay(uint2 index, in float3 cameraPosition, in float4x4 cameraMatrix)
{
    
    float screen_width = 1280.0f;
    float screen_height = 720.0f;
    float aspect_ratio = screen_width/screen_height;

    float2 xy = index; // center in the middle of the pixel.
    float2 screenPos = float2(xy.x/screen_width,xy.y/screen_height) * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    float4 camWorldDir = mul(cameraMatrix, float4( screenPos.x * aspect_ratio, screenPos.y, 1, 0) );

    Ray ray;
    ray.origin = float4( cameraPosition, 1 );
    ray.direction =float4( normalize(camWorldDir.xyz), 0);
    ray.invdirection = float4( 1.0f / ray.direction.x, 
                               1.0f / ray.direction.y, 
                               1.0f / ray.direction.z, 0);

    return ray;
}

[numthreads(8, 8, 1)]
void TraceRays(uint3 DTid : SV_DispatchThreadID, uint groupid : SV_GroupIndex)
{
    float3 light = float3(-1000.0f,1000.0f,1000.0f);
    float lightSize = 25.0f;
	
    Ray r = GenerateCameraRay(DTid.xy,vViewPos.xyz,mInvViewProjection);

    float dist;
    float eps = 0.0001;

    PayLoad payLoad;

    float4 resultColor = float4(getSkyColor(r), 1);

    if(TraverseSceneTLAS(r,payLoad,8000,false))
    {
        float4 hitPoint = r.origin + r.direction * payLoad.hitDist;
        Ray shadow_ray;
        shadow_ray.origin = hitPoint + 
                            float4( payLoad.normal * eps, 0 );
        shadow_ray.origin.a = 1;
        shadow_ray.direction = float4( normalize(light + randomPointOnUnitSphere(DTid.xy+ float2(deltas.xx))*lightSize),0);
        CalculateRayIntrinsics(shadow_ray);
        
        
        PayLoad shadow_pay_load;
        float ndl = saturate( max(dot(normalize(light),normalize(payLoad.normal)),0.0) );

        bool has_shadows = TraverseSceneTLAS(shadow_ray,shadow_pay_load,1000, true);
        
        if( has_shadows )
            resultColor = float4(0.0,0.0,0.0,1);
        else
            resultColor = float4(ndl,ndl,ndl,1);
    }

    tResult[DTid.xy] = resultColor;
}