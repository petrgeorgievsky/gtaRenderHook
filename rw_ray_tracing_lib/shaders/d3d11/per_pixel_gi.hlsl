struct PayLoad
{
    float3 normal;
    float hitDist;
    float4 hitPosition;
    float4 color;
};


#include "raytracing/rt_structs.hlsli"

static const float3 skyColor = float3(0.3f, 0.5f, 1.0f);
static const float3 botColor = float3(0.0f, 0.0f, 0.0f);

float3 getSkyColor(Ray r){
    float dir = max(dot(r.direction, float3(0,0,1)),0.0f);
    
    float3 light = float3(-1000.0f,1000.0f,1000.0f);

    return lerp( botColor, skyColor, dir ) + pow(max(dot(r.direction,normalize(light)),0.0f),16);
}
inline void INIT_PAYLOAD_MISS(inout PayLoad payLoad, in Ray r, in float maxDist)
{
    payLoad.hitDist = maxDist;
    payLoad.normal = -r.direction.xyz;
    payLoad.hitPosition = r.origin + r.direction*maxDist;
    payLoad.color.rgb = getSkyColor(r);
    payLoad.color.a = 1;
}

inline void INIT_PAYLOAD_AABB_HIT(inout PayLoad payLoad)
{
    payLoad.color = float4(0,0,0,0);
}

StructuredBuffer<Triangle> triangleBuffer : register(t0);
StructuredBuffer<Vertex> bufVertexList : register(t1);
StructuredBuffer<BLASLeaf> bottomLevelAS : register(t2);
StructuredBuffer<TLASLeaf> topLevelAS : register(t3);
Texture2D<float4> gbPosition : register(t4);
Texture2D<float4> gbNormals : register(t5);
Texture2D<float4> randomTexture : register(t6);

inline float3 GET_TRIANGLE_POS(uint id)
{
    return bufVertexList[id].pos.xyz;
}

inline float4 GET_VERTEX_COLOR(uint id)
{
    uint color = bufVertexList[id].color;
    float d = 1.0f/255.0f;
    return float4( float((color) & 0xFF)*d, float((color >> 8) & 0xFF)*d, float((color >> 16) & 0xFF)*d, float((color >> 24) & 0xFF)*d );
}

inline void INIT_PAYLOAD_HIT_TRIANGLE(inout PayLoad payLoad, float u, float v, uint4 triangle_ids)
{
    payLoad.color = GET_VERTEX_COLOR(triangle_ids.x) * (1-u-v) + GET_VERTEX_COLOR(triangle_ids.y) * u + GET_VERTEX_COLOR(triangle_ids.z) * v;
}

#include "raytracing/triangle_intersection.hlsl"
#include "raytracing/aabb_intersection.hlsl"
#include "raytracing/bvh_traverse.hlsl"

RWTexture2D<float4> tResultSH_Y : register(u0);
RWTexture2D<float2> tResultSH_CoCg : register(u1);

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

cbuffer GIParams : register( b1 )
{
    float fRenderingScale;
    float fRaytracingDistance;
    float fNoiseScale;
    float gi_padd_;
};

SamplerState s0 : register(s0);

#define THREAD_GROUP_SIZE 8

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
    CalculateRayIntrinsics( ray );
    return ray;
}
struct SH
{
    float4 shY;
    float2 CoCg;
};

SH irradiance_to_SH(float3 color, float3 dir)
{
    SH result;

    float   Co      = color.r - color.b;
    float   t       = color.b + Co * 0.5;
    float   Cg      = color.g - t;
    float   Y       = max(t + Cg * 0.5, 0.0);

    result.CoCg = float2(Co, Cg);

    float   L00     = 0.282095;
    float   L1_1    = 0.488603 * dir.y;
    float   L10     = 0.488603 * dir.z;
    float   L11     = 0.488603 * dir.x;

    result.shY = float4 (L11, L1_1, L10, L00) * Y;

    return result;
}

SH init_SH()
{
    SH result;
    result.shY = float4(0,0,0,0);
    result.CoCg = float2(0,0);
    return result;
}

SH load_SH(int2 p)
{
    SH result;
    result.shY = tResultSH_Y[p];
    result.CoCg = tResultSH_CoCg[p];
    return result;
}
void accumulate_SH(inout SH accum, SH b, float scale)
{
    accum.shY += b.shY * scale;
    accum.CoCg += b.CoCg * scale;
}

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void TraceRays(uint3 DTid : SV_DispatchThreadID, uint groupid : SV_GroupIndex)
{
    float3 light = float3(-1000.0f,1000.0f,1000.0f);
    float lightSize = 25.0f;
    float eps = 0.01;
    PayLoad payLoad;

    float4 resultColor = float4( 0, 0, 0, 1 );
    float4 position = gbPosition[DTid.xy/fRenderingScale];
    float3 normals = normalize( gbNormals[DTid.xy/fRenderingScale].xyz );
    if( position.a <= 0 )
    {
        tResultSH_Y[DTid.xy] =  float4( 0, 0, 0, 0 );
        tResultSH_CoCg[DTid.xy] =  float2( 0, 0 );
        return;
    }
    float4 ray_cast_point = float4( position.xyz + normals * eps, 1 );
    uint nscale = uint(fNoiseScale);
    int rndOffsetX = (frac(deltas.x)*nscale);
    int rndOffsetY = (frac(deltas.y)*nscale);
    int2 rng_seed = int2((DTid.x+rndOffsetX)%nscale,(DTid.y+rndOffsetY)%nscale);
    float3 randomness = normalize( randomTexture[rng_seed].xyz * 2.0 - 1.0.xxx + normals + normalize(light) );

    Ray gi_ray;
    gi_ray.origin = ray_cast_point;
    gi_ray.direction = float4( randomness/*normalize( -reflect( normalize( vViewPos.xyz - gbPosition[DTid.xy].xyz ), 
                                                    normalize( gbNormals[DTid.xy].xyz ) ) )*/, 0 );
    CalculateRayIntrinsics( gi_ray );
    
    PayLoad gi_load;

    bool hit_not_sky = TraverseSceneTLAS( gi_ray, gi_load, fRaytracingDistance, false );

    Ray gi_sray;
    gi_sray.origin = float4( gi_ray.origin.xyz + gi_load.hitDist * gi_ray.direction.xyz + gi_load.normal*0.01f, 1.0);
    gi_sray.direction = float4( normalize(light), 0 );
    CalculateRayIntrinsics( gi_sray );

    PayLoad gis_load;
    bool in_shadow = TraverseSceneTLAS( gi_sray, gis_load, 1000, true );

    SH s_harm = irradiance_to_SH( gi_load.color * max(dot(gi_load.normal, gi_sray.direction.xyz),0.0) * !in_shadow * hit_not_sky + !hit_not_sky * gi_load.color, gi_ray.direction.xyz );
    SH old_sh = load_SH(DTid.xy);
    SH result_sh = init_SH();
    float accum_fps = 6.0f;
    accumulate_SH(result_sh,s_harm,1.0/accum_fps);
    accumulate_SH(result_sh,old_sh,1.0-(1.0/accum_fps));
    if( any( isnan( result_sh.shY ) ) || any(isnan( result_sh.CoCg )) ) 
        result_sh = init_SH();

    tResultSH_Y[DTid.xy] = result_sh.shY;
    tResultSH_CoCg[DTid.xy] = result_sh.CoCg;
}