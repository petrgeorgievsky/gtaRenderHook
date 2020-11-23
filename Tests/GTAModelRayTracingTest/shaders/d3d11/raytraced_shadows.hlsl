struct PayLoad
{
    float3 normal;
    float hitDist;
    float4 hitPosition;
};

#include "raytracing/rt_structs.hlsli"

inline void INIT_PAYLOAD_MISS(inout PayLoad payLoad, in Ray r, in float maxDist)
{
    payLoad.hitDist = maxDist;
    payLoad.normal = float3(0,0,0);
    payLoad.hitPosition = r.origin + r.direction*maxDist;
}

inline void INIT_PAYLOAD_HIT_TRIANGLE(inout PayLoad payLoad, float u, float v)
{

}

StructuredBuffer<Triangle> triangleBuffer : register(t0);
StructuredBuffer<float4> bufVertexList : register(t1);
StructuredBuffer<BLASLeaf> bottomLevelAS : register(t2);
StructuredBuffer<TLASLeaf> topLevelAS : register(t3);
Texture2D<float4> gbPosition : register(t4);
Texture2D<float4> gbNormals : register(t5);

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

SamplerState s0 : register(s0);

#define THREAD_GROUP_SIZE 16

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void TraceRays(uint3 DTid : SV_DispatchThreadID, uint groupid : SV_GroupIndex)
{
    float3 light = float3(-1000.0f,1000.0f,1000.0f);
    float lightSize = 25.0f;
    float eps = 0.005;
    PayLoad payLoad;

    float4 resultColor = float4( 0, 0, 0, 1 );
    float4 ray_cast_point = float4( gbPosition[DTid.xy].xyz + gbNormals[DTid.xy].xyz * eps, 1 );

    Ray shadow_ray;
    shadow_ray.origin = ray_cast_point;
    shadow_ray.direction = float4( normalize( light ), 0 );
    CalculateRayIntrinsics( shadow_ray );
    
    PayLoad shadow_pay_load;

    bool has_shadows = TraverseSceneTLAS( shadow_ray, shadow_pay_load, 1000, true );
    
    if( !has_shadows )
        resultColor = float4( 1, 1, 1, 1 );

    tResult[DTid.xy] = resultColor;
}